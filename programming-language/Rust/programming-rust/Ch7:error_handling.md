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

8. `std::time::SystemTime::elapsed()`

   ```rust
   pub fn elapsed(&self) -> Result<Duration, SystemTimeError>
   ```

   Returns the Duration elspased from the time point `self`.

   ```rust
   use std::{
       time::{
           SystemTime,
           Duration,
       },
       thread::sleep,
   };
   
   fn main() {
       let now = SystemTime::now();
       sleep(Duration::from_secs(2));
   
       println!("{:?}", now.elapsed().unwrap())
   }
   ```

   ```shell
   $ cargo r -q
   2.000248375s
   ```

   This function may fail as the `SystemTime` can go backwards. If you don't want
   this, use `Instant::elapsed()` instead. `Instant` is guaranteed to be 
   monotonically nondecreasing.

9. `exit(3)` in cpp will not call destructor to clean up the left resources,
   and thus is not recommended to use in your program.

   In C, it is [mostly fine to use `exit(3)`](https://stackoverflow.com/a/31501322/14092446).
   However, there may be some resources that won't be freed by `exit(3)`, in such
   cases, you need to call `atexit(3)` to define a callback to manually clean
   those stuff.

   ```c
   #include <stdlib.h>

   int atexit(void (*function)(void));
   ```

   How does Rust handle this:

   ```rust
   // std::process::exit

   pub fn exit(code: i32) -> ! {
       crate::rt::cleanup();
       crate::sys::os::exit(code)
   }
   ```

   ```rust
   // crate::sys::os::exit
   // a wrapper for libc::exit

   pub fn exit(code: i32) -> ! {
       unsafe { libc::exit(code as c_int) }
   }
   ```

   ```rust
   // One-time runtime cleanup.
   // Runs after `main` or at program exit.
   // NOTE: this is not guaranteed to run, for example when the program aborts.
   pub(crate) fn cleanup() {
       static CLEANUP: Once = Once::new();
       CLEANUP.call_once(|| unsafe {
           // Flush stdout and disable buffering.
           crate::io::cleanup();
           // SAFETY: Only called once during runtime cleanup.
           sys::cleanup();
       });
   }
   ```

   ```rust
   // io::cleanup()

   // Flush the data and disable buffering during shutdown
   // by replacing the line writer by one with zero
   // buffering capacity.
   pub fn cleanup() {
       let mut initialized = false;
       let stdout = STDOUT.get_or_init(|| {
           initialized = true;
           ReentrantMutex::new(RefCell::new(LineWriter::with_capacity(0, stdout_raw())))
       });
   
       if !initialized {
           // The buffer was previously initialized, overwrite it here.
           // We use try_lock() instead of lock(), because someone
           // might have leaked a StdoutLock, which would
           // otherwise cause a deadlock here.
           if let Some(lock) = stdout.try_lock() {
               *lock.borrow_mut() = LineWriter::with_capacity(0, stdout_raw());
           }
       }
   }
   ```

   ```rust
   // sys::cleanup(), defined in `unix.rs`
   
   // SAFETY: must be called only once during runtime cleanup.
   // NOTE: this is not guaranteed to run, for example when the program aborts.
   pub unsafe fn cleanup() {
       stack_overflow::cleanup();
   }
   ```

   ```rust
   // stack_overflow.rs: cleanup()

   pub unsafe fn cleanup() {
       drop_handler(MAIN_ALTSTACK.load(Ordering::Relaxed));
   }
   ```

10. The `main()` function in Rust can return `Result<T, E>` 
    [since 1.26](https://blog.rust-lang.org/2018/05/10/Rust-1.26.html#main-can-return-a-result)

    If `main` returns an `Error`, it will exit the program with an error code and
    print the `Debug` representation of that `Error`.

11. `#[derive(std::errror::Error)` using [`thiserror`](https://docs.rs/thiserror/latest/thiserror)


    ```rust
    use thiserror::Error;
    
    #[derive(Error, Debug)]
    #[error("Error with code: {0}")]
    struct ErrorWithCode(i32);
    
    fn main() {
        let f = ErrorWithCode(1);
        println!("{}", f);
    }
    ```

    ```shell
    $ cargo r -q
    Error with code: 1
    ```

    It is a convenient macro to help you avoid the manual implementation of
    `std::fmt::Display` and `std::error::Error`.
