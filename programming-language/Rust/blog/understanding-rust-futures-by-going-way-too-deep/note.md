1. `tokio::task::spawn`

   ```rust
   async fn bar() {
       println!("bar"); 
   }

   async fn foo() {
       bar().await;
   }

   #[tokio::main]
   async fn main() {
       foo().await;
   }
   ```

   The above code snippet, that `bar()` future, can be seen that it is running 
   inside the main coroutine.

   ```rust
   async fn foo() {
       let bar = bar();
       tokio::spawn(bar);
   }
   ```
   If we use `spawn()` to execute it, then a new dedicated coroutine will be 
   created just for it.

   > This is pretty similar to `thread::spawn`.

   > Note that when the main coroutine exits, all coroutines exit.
