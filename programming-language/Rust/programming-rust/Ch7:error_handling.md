#### Ch7: Error Handling

> ##### Panic
>
> ##### Result

##### Panic

1. When Rust process panics, it will eithter unwind the stack, or just abort the
   program.

   Unwinding is the default option, if you don't like it, change it in `Cargo.toml`:

   ```toml
   # For debug mode
   [profile.dev]
   panic = "abort"

   # For release mode
   [profile.release]
   panic = "abort"
   ```

   When `abort` is chosen, the panicing program will receive a `SIGABRT`.

2. Panic is per thread, one thread can be panicking while other threads are
   going on about their normal business.

   ```rust
   use std::thread::spawn;
   
   fn main() {
       let handle = spawn(|| {
           panic!("One thread panicked");
       });
       handle.join().unwrap();
   
       println!("Hello from another thread!");
   }
   ```

   ```shell
   $ cargo r -q
   thread '<unnamed>' panicked at 'One thread panicked', src/main.rs:5:9
   stack backtrace:
      0: rust_begin_unwind
                at /rustc/897e37553bba8b42751c67658967889d11ecd120/library/std/src/panicking.rs:584:5
      1: core::panicking::panic_fmt
                at /rustc/897e37553bba8b42751c67658967889d11ecd120/library/core/src/panicking.rs:142:14
      2: rust::main::{{closure}}
                at ./src/main.rs:5:9
   note: Some details are omitted, run with `RUST_BACKTRACE=full` for a verbose backtrace.
   Hello from another thread!
   ```

3. When panic is implemented using `unwind`, one can catch it and allow
   the thread to survive and continue running using `std::panic::catch_unwind()`

   Thus, `unwind` does not necessarily terminates the thread. A typical
   app that would use this this feature is the web server, when the thread 
   in the server panics, you can `catch_unwind()` and return `500: server 
   internal error`.

4. Two cicumstances where `abort` is employed:

   1. When unwinding the stack, a second `panic` occurred, this is considered
      fatal, Rust stops unwinding the stack and aborts the whole process.

   2. When compiling with `-C panic=abort`, the **first** panic in the program
      immediately aborts the process.
 
##### Result

1. To convert a `Result<T, E>` to `Option<E>`, use `std::result::Result::err()`

   ```rust
   let x: Result<u32, &str> = Ok(2);
   assert_eq!(x.err(), None);
   
   let x: Result<u32, &str> = Err("Nothing here");
   assert_eq!(x.err(), Some("Nothing here"));
   ```

2. Use `unwrap_or(self, default: T)` on `Result<T, E>` to fallback to `default`
   when `self` is an `Err(E)`.

3. Difference between `writeln!(stderr(), "msg")` and `eprintln!("msg")`:
   
   `eprintln!` may panic if writing to stderr fails, whereas `writeln!` won't
   do this.

4. When a test's normal behavior is panicking, you can use:

   ```rust
   #[cfg(test)]
   mod  test {
       #[test]
       #[should_panic(expected = "this test should panic")]
       fn test() {
          panic!();
       }
   }
   ```

   to specify that this test should panic.

5. The question mark in Rust

   When the function on which `?` is called returns an `Error`, `From::from` is
   called to convert the error type to the type `E` (`Err(E)`) and returns to the
   calling code immediately:

   ```rust
   fn _parse(str: &str) -> Result<i32, &str> {
       if let Ok(num) = str.parse::<i32>() {
           Ok(num)
       } else {
           Err(str)
       }
   }
   
   fn parse(str: &str) -> Result<(), String> {
       let num = _parse(str)?;
       println!("{}", num);
       Ok(())
   }
   ```

   In the above code, when `_parse(str)?` fails, it will call `<String as 
   From<&str>::from(str)` to convert `&str` to `String`, which can be rewritten
   as:

   ```rust
   fn parse(str: &str) -> Result<(), String> {
       match _parse(str) {
           Ok(n) => {
               println!("{}", n);
               Ok(())
           }
           Err(str) => Err(<String as From<&str>>::from(str)),
       }
   }
   ```

6. You can implment methods for a trait object, look at this `downcast()` method
   for `Error` trait object.

   > See [this answer](https://stackoverflow.com/a/34446463/14092446) for more
   > information.

   ```rust
   impl dyn Error {
       #[inline]
       #[stable(feature = "error_downcast", since = "1.3.0")]
       #[rustc_allow_incoherent_impl]
       /// Attempts to downcast the box to a concrete type.
       pub fn downcast<T: Error + 'static>(self: Box<Self>) -> Result<Box<T>, Box<dyn Error>> {
           if self.is::<T>() {
               unsafe {
                   let raw: *mut dyn Error = Box::into_raw(self);
                   Ok(Box::from_raw(raw as *mut T))
               }
           } else {
               Err(self)
           }
       }
   }
   ``` 

   The above implementation does not explicitly specify the lifetime argument,
   in this case, `'static` is used by default, see 
   [this](https://doc.rust-lang.org/std/error/trait.Error.html#impl-dyn%20Error%20+%20%27static-2)

   > Why do we need lifetime here, trait object is the pointer pointing to the
   > trait and the **specific value**, i.e., we are borrowing that value, and 
   > thus we need lifetime.
   >
   > Available forms: 
   > 1. `& (mut) T`
   > 2. `Box/Rc/Arc<T>`
   >
   > (`Box<T>` owns its value, and thus has lifetime `'static`)

7. Use `downcast_ref()` to get the concrete type of an `Error`:
  
   These methods are implemented for `Error` trait object instead of types
   `E: Error`.

   This function is generic, and thus you have to provide a generic type `T`
   when calling it:

   ```rust
   pub fn downcast_ref<T>(&self) -> Option<&T>
   where
       T: 'static + Error,
   ```

   ```rust
   use std::{
       error::Error,
       fmt::{Display, Formatter},
       str::Utf8Error,
   };
   
   #[derive(Debug)]
   struct Foo;
   
   impl Display for Foo {
       fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
           write!(f, "{:?}", self)
       }
   }
   
   impl Error for Foo {}
   
   fn main() {
       let f = Box::new(Foo) as Box<dyn Error>;
       let bad_guess = f.downcast_ref::<Utf8Error>();
       let good_guess = f.downcast_ref::<Foo>();
       println!("{:?}", bad_guess);
       println!("{:?}", good_guess);
   }
   ```

   ```shell
   $ cargo r -q
   None
   Some(Foo)
   ```

   > There are actually three `downcast_ref()` methods on `dyn Error`, the 
   > difference lies in the trait bound of the targeting trait object:
   >
   > ```rust
   > impl dyn Error + 'static 
   > impl dyn Error + 'static + Send
   > impl dyn Error + 'static + Send + Sync
   > ```
   >
   > For the implementation of these three functions, the last two implementations
   > rely on the first one.
