1. Future 的简单定义

   ```rust
   trait SimpleFuture {
       type Output;
       fn poll(&mut self, wake: fn()) -> Poll<Self::Output>;
   }
   ```

   其中的`wake`参数是一个回调函数，是执行器在调用`poll()`的时候注册到`Future`中
   去的，当`Future`可以取得一些进展的时候，它就会调用被注册的`wake`回调函数来通
   知执行器自己可以取得一些进展了，可以尝试调用`poll()`试试看了。

   这个`wake`回调函数是执行器注册的，并且是在`poll()`函数的实现内进行注册的，当
   执行器第一次调用`poll()`时**却发现此`Future`并没有取得进展**，那么就会给此`Future`
   注册上`wake()`回调函数。

   一个读`socket`拿取数据的伪代码如下：

   ```rust
   /// This is a Future.
   pub struct SocketRead<'a> {
       socket: &'a Socket,
   }
   
   impl SimpleFuture for SocketRead<'_> {
       type Output = Vec<u8>;
   
       fn poll(&mut self, wake: fn()) -> Poll<Self::Output> {
           if self.socket.has_data_to_read() {
               // socket有数据，写入buffer中并返回
               Poll::Ready(self.socket.read_buf())
           } else {
               // socket中还没数据，无法取得进展，注册wake，返回Pending
               //
               // 注册一个`wake`函数，当数据可用时，该函数会被调用，
               // 然后当前Future的执行器会再次调用`poll`方法，此时就可以读取到数据
               self.socket.set_readable_callback(wake);
               Poll::Pending
           }
       }
   }
   ```

2. 使用真实的`Future trait`定义来构造一个定时器Future

   这个定时器的`Future`逻辑很简单，普通的`Future`是在不确定的将来返回真正的数据，
   但定时器`Future`在定时结束后就可以返回数据。

   定时的时间我们专门 spawn 一个线程，然后调用 `sleep(duration)` 来实现定时的功
   能，此线程我们名为定时线程。因为定时线程需要和 `Future` 进行沟通，比如说在定
   时结束后需要更新`Future`的状态为`已完成`，所以我们需要用到`Arc<Mutex<State>>`.

   
   ```rust
   struct TimerFuture {
       state: Arc<Mutex<State>>, 
   }

   struct State {
       completed: bool,
       /// 可以简单理解为wake回调函数
       waker: Option<std::task::Waker>,
   }
   ```

   ```rust
   impl TimerFuture {
       fn new() -> Self {
           let state = Arc::new(Mutex::new(State{
               completed: false,
               waker: None,
           }));

           let state_clone = Arc::clone(&state);

           // 定时开始
           thread::spawn(move||{
               thread::sleep(Duration::from_secs(2)); 
               let guard = state_clone().lock().unwrap();

               // 定时结束，接下来
               // 1. 更新状态
               guard.completed = true;
               
               // 2. 调用`wake`通知执行器，自己已完成
               if let Some(waker) = guard.waker.take() {
                   waker.wake(); 
               }
           });
           
           Self {
               state, 
           }
       } 
   }
   ```

   ```rust
   impl Future for TimerFuture {
       // 定时器Future并不会返回什么真正有用的值
       type Output = ();      
        
       fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
           let guard = self.state.lock().unwrap();

           if guard.completed {
               return Poll::Ready(());
           } else {
               // future没有进展，注册函数，返回pending
               guard.waker = Some(cx.waker().clone());

               return Poll::Pending;
           }
       }
   } 
   ```

   测试我们的定时器`Future`:

   ```rust
   async fn ready_future() {
       println!("FUTURE: ready_future");
   }
   
   async fn timer_future() {
       let _ = TimerFuture::new(Duration::from_secs(2)).await;
       println!("FUTURE: timer_future");
   }
   
   #[tokio::main]
   async fn main() {
       let timer_future = timer_future();
       let ready = ready_future();
   
       tokio::join!(ready, timer_future);
   }
   ```

   创建了一个立刻就准备好的`Future` ready_future, 以及我们的2秒后才可以准备好的
   定时器`Future`，使用`join!()`来并发地执行这2个`Future`:


   ```shell
   $ cargo r -q
   FUTURE: ready_future
   FUTURE: timer_future  # 2s 后才打印出来
   ```

   可以发现我们的定时器`Future`是模拟成功的！
