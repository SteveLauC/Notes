#### 1.1
1. diff between `async` and `muplithreading`

   举个例子，后厨需要做一个鸡蛋，烤一份土豆 
   * sync: 做鸡蛋，烤土豆(顺序执行)
   * async, single threaded: 做鸡蛋，设置一个定时器，然后烤土豆，再设置一个定时器
   * async, multithreaded: 一个厨师做鸡蛋，另一个厨师烤土豆

   > async is about tasks while mutlithreading is about workers


   > [link](https://stackoverflow.com/q/34680985/14092446)

   有好几种concurrent models，os threads 和 async programming是其中的两种

2. OS threads比较适合数量较小的工作，因为它会用CPU和memory，线程切换是很费资源的。
   而async则支持多得多的任务数量，而且有少得多的cpu和memory的负载。在rust中，使用
   async会导致编译binary的大小增大，因为bundle了runtime

#### 1.3
1. async函数返回的是一个`Future<ReturnTypeOfThatFn>`，每一个`Future`都需要在`executor`
   上进行执行

   ```rust
   // 使用`futures`crate中的`block_on`这个`executor`
   // `block_on` will block the current thread until the Future is complete
   use futures::executor::block_on;

   async hello() {
       println!("hello");
   }

   fn main() {
       let f = hello();
       block_on(f);
   }
   ```

   ```shell
   $ cargo run
   hello
   ```

2. 使用`.await`来执行一个`Future`

   It will not block the caller, but instead **asynchronously waits** for the future 
   to complete, allowing other tasks to run if the future is currently unable 
   to make progress.

   它不会阻塞当前线程，如果一个`Future`在当前完不成，此`Future`就会交出其对当前
   线程的控制权，让其他的`Future`来占用当前线程

   > 仅可在`async`的函数中使用

    ```rust
    use futures::executor::block_on;

    fn main() {
        block_on(hello());
    }

    async fn hello() {
        // use `.await` to run the `Future`
        real_hello().await;
    }

    async fn real_hello() {
        println!("hello");
    }
    ```

    > 需要注意的是，在一个 async block 中，有多个`Future`，然后多次使用`.await`来
    > 等待，那么这几个在同一个 async block 里的`Future`s 的执行顺序仍然是从上往下
    > 的，并不会说第一个无法取得进展，而先去执行第二个
    >
    > ```rust
    > async fn first() {
    >     // tokio::time::sleep(Duration::from_secs(2)).await;
    >     println!("first");
    > }
    >
    > async fn second() {
    >     println!("second");
    > }
    >
    > #[tokio::main]
    > async fn main() {
    >     first().await; 
    >     second().await; 
    > }
    > ```
    >
    > 上面的代码永远都是先打印`first`，再打印`second`.即使说`first()`里面`tokio::time::sleep()`
    > 了，也会是先打印`first`。
    >
    >
    > 那种一个`Future`无法取得进展，换用另一个`Future`的情况得用`join!()`:
    >
    > ```rust
    > #[tokio::main]
    > async fn main() {
    >     let first_fut = first();
    >     let second_fut = second();
    >
    >     tokio::join!(first, second);
    > }
    > ```
    >
    > 比如取消`first()`里`sleep()`的注释，来运行上面的代码：
    > 
    > ```shell
    > $ cargo r
    > second
    > first    # 2s 后才打印first
    > ```
