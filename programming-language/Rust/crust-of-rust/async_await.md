1. What does `async` mean when it decorates a function, it changes the return
   type of that function from its `ORIGINAL_RETURN_TYPE` to `impl Future<Output=
   ORIGINAL_RETUN_TYPE>`:

   ```rust
   use std::future::Future;

   async fn main() {
       foo_async().await;
       foo().await;
   }

   async fn foo_async() {}

   // is converted into something like this:

   fn foo() -> impl Future<Output = ()> {
       ansync {

       }
   }
   ```

2. `impl Future<Output = TYPE>` can be seen as a promise that I will give you
   a value of type `TYPE`, but I am not clear when, evetually, that value will
   be returned.

   And such a future does nothing unitl it's been `await`ed.


   ```rust
   #[tokio::main]
   async fn main() {
       let foo = foo();
   }
   
   async fn foo() {
       println!("hello");
   }
   ```

   ```shell
   $ cargo r -q
   warning: unused variable: `foo`
    --> src/main.rs:3:9
     |
   3 |     let foo = foo();
     |         ^^^ help: if this is intentional, prefix it with an underscore: `_foo`
     |
     = note: `#[warn(unused_variables)]` on by default
   ```

   You can see that `hello` is not printed out.

3. Each `.await` is an opporunity for things above you that they can do something


   ```rust
   async fn do_stuff_async() {
       // async work
   }
   
   async fn more_async_work() {
       // more here
   }
   
   #[tokio::main]
   async fn main() {
       tokio::select! {
           _ = do_stuff_async() => {
               println!("do_stuff_async() completed first")
           }
           _ = more_async_work() => {
               println!("more_async_work() completed first")
           }
       };
   }
   ```

4. `main()` function can NOT be annocated with `async` because if it is marked 
   with `async`, it becomes a `Future`, and a `Future` has to be executed by an
   `executor`, this is where `#[tokio::main]` comes in. 

   ```rust
   #[tokio::main]
   async fn main() {
       let foo = foo().await;
   }
   
   async fn foo() {
       println!("hello");
   }
   ```

   is turned into something like:

   ```rust
   use tokio::runtime::Runtime;
   
   fn main() {
       let runtime = Runtime::new().unwrap();
       runtime.block_on(async {
           let foo = foo().await;
       })
   }
   
   async fn foo() {
       println!("hello");
   }
   ```

