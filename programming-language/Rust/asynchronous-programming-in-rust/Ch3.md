1. 有2种使用`async`的方法，`async fn`和`async block`，均会返回实现了`Future`
   trait的值

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
