1. 有2种使用`async`的方法，`async fn`和`async block`，均会返回实现了`Future`
   trait的值

   > async closures are unstable
   ```rust
   use futures::executor::block_on;
   fn main() {
       let clo = async || {
           println!("hello from async closure");
       };

       block_on(clo());
   }
   ```

    ```rust
    use std::future::Future;

    fn main() {}

    async fn int() -> i32 {
        0
    }

    fn foo() -> impl Future<Output = i32> {
        // async fn
        // int()

        // async block
        async { 0 }
    }
    ```

2. `Future`是惰性的(inert)，只有当被执行的时候才会做一些事情。最常用的执行一个
   `future`的方法是`.await`它。当一个`future`被`.await`时，会尝试去执行，如果执
   时发现无法取得进展，此`future`会交出对当前线程的控制权，从而去运行其他可以取
   得进展的线程。等到刚才的`future`发现可以取得进展了，又会将线程的控制权取回，
   然后继续运行

   > 与其说其是惰性或者lazy的，不如说其是delayed有延时的

3. 关于`async block`的语法，本质上是以async block作为body的closure

   可以将其看作是立刻声明立刻调用的`async closure`

   ```rust
   || {
       async {
           println!("hello");
       }
   }
   async {
       println!("hello");
   }

   // is equivalent to 
   (async || {
       println!("hello");
   })()
   ```

   正如`async closure`一样，`async block`也可以使用`move`关键字来捕捉所有权

   ```rust
   fn main() {
       let str: String = String::new();
       async move {          // value moved here
           drop(str);
       };
       println!("{}", str);  // use of moved value
   }
  ```

4. async function的2种写法(((

   ```rust
   use std::future::Future;

   fn main() {
   }

   // 这就是async fn
   async fn foo_async() -> () {
       ()
   }
   
   // async block
   fn foo() -> impl Future<Output = ()> {
       async {}
   }
   ```

5. async lifetime

   Unlike traditional functions, async fns which take `references or other 
   non-'static arguments` return a Future which is bounded by the lifetime 
   of the arguments:

   ```rust
   // This function:
   async fn foo(x: &u8) -> u8 { *x }

   // Is equivalent to this function:
   fn foo_expanded<'a>(x: &'a u8) -> impl Future<Output = u8> + 'a {
       async move { *x }
   }
   ```

   > what is `reference or other non-'static arguments`

6. mulithreaded future executor

   Note that, when using a multithreaded Future executor, a Future may move between 
   threads, so any variables used in async bodies must be able to travel between 
   threads
