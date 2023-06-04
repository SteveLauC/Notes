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

5. Side effects with `select!`
   
   This macro will wait on multiple concurrent branches, returning when the 
   first branch completes, cancelling the remaining branches.

   When a branch completes, there is a possibility that the other branches
   are still doing theirs work, resulting in a partial state.

   You need to make sure the partial branches will be finished.

   > [ref](https://docs.rs/tokio/latest/tokio/macro.select.html#cancellation-safety)


6. Functions from `std` know nothing about async, when they are blocked, the whole
   thread just gets blocked, there is no `yield` in that world. You need to use
   the counterparts from runtime.

7. `join!/join_all/FuturesUnordered/FuturesOrdered`, they all run futures 
   concurrently. 

8. By default, `tokio` creates the runtime with multiple threads, but only one
   thread will be busy as there is ONLY one future (task) to be executed.

   ```rust
   use tokio::runtime::Runtime;

   async fn a_future() {
       println!("Hello");
   }

   fn main() {
       let rt = Runtime::new().unwrap();

       // `main_future` is the ONLY future we have
       let main_future = async {
           a_future().await; 
       };
       
       rt.block_on(main_future);
   }
   ```

   `a_future()` is a sub future inside of `main_future`, so in the executor's view,
   there is ONLY one future to execute, and thus ONLY one thread will be busy.

   How can we solve this? Use `spawn()` to create more tasks:

   ```rust
   fn main() {
       let rt = Runtime::new().unwrap();
       let main_future = async {
           
           // Alright, `a_future` is no longer a sub future, another task,
           // another thread can do its job
           let handle = spawn(a_future());
           handle.await.unwrap();
       };

       rt.block_on(main_future);
   }
   ```

   With `spawn()`, tasks are no longer executed concurrently, they are in parallel.

   > If you call `spawn()` without a tokio runtime, it panics.

9. Why is async function not allowed in trait

   ```rust
   trait AsyncRead {
       async fn read(&mut self, buf: &mut [u8]) -> Result<usize, std::io::Error>;
   }
   ``` 

   We know that async function is basically a syntax sugar for:

   ```rust
   trait AsyncRead {
       fn read(&mut self, buf: &mut [u8]) -> impl Future<Output = Result<usize, std::io::Error>>;
   }
   ```

   And that `impl Future<Output = Result<usize, std::io::Error>>`, the anonymous 
   State machine is not something that has fixed size.

   > State machine needs to store the state, i.e., the used variables.

   ```rust
   struct Foo;

   impl AsyncRead for Foo {
       fn read(&mut self, buf: &mut [u8]) -> impl Future<Output = Result<usize, Error>> {
           async { Ok(0) }
       }
   }

   fn main() {
       let mut foo = Foo;
       let mut buf = [0; 1024];

       // What is the size for this fut?
       let fut = <Foo as AsyncRead>::read(&mut foo, &mut buf);
   }
   ```


   How does `async-trait` solve this problem? It turns your function into 
   something like this:

   ```rust
   trait AsyncRead {
       fn read(&mut self, buf: &mut [u8]) -> Pin<Box<dyn Future<Output = Result<usize, Error>>>>;
   }

   struct Foo;

   impl AsyncRead for Foo {
       fn read(&mut self, buf: &mut [u8]) -> Pin<Box<dyn Future<Output = Result<usize, Error>>>> {
           let fut = async { Ok(0) };

           Box::pin(fut)
       }
   }
   ```

   Great, now the type of `fut` becomes `Box<Pin<dyn Future>>` and is fixed-sized
   now as `Future` is behind a pointer.

   But this is inefficient because every call of `async read()` does a heap allocation,
   and an address jump is needed to locate the actual `Future`.


