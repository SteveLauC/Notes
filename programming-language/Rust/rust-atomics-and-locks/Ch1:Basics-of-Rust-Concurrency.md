> This chapter introduces all the tools and concepts we need for basic concurrency 
> in Rust, such as threads, mutexes, thread safety, shared and exclusive references,
> interior mutability, and so on, which are foundational to the rest of the book. 
>
> For experienced Rust programmers who are familiar with these concepts, this 
> chapter can serve as a quick refresher. For those who know these concepts 
> from other languages but aren’t very familiar with Rust yet, this chapter will 
> quickly fill you in on any Rust-specific knowledge you might need for the rest 
> of the book.

> TOC
>
> * [Threads In Rust]()
> * [Scoped Threads]()
> * [Shared Ownership and Reference Counting]()
> * [Borrowing and Data Races]()
> * [Interior Mutability]()
>   * [Cell]()
>   * [RefCell]()
>   * [Mutex and RwLock]()
>   * [Atomic]()
> * [Interior Mutability]()

1. Operating systems isolate processes from each other as much as possible, 
   allowing a program to do its thing while completely unaware of what any 
   other processes are doing. For example, a process cannot normally access 
   the memory of another process, or communicate with it in any way, **without 
   asking the operating system’s kernel first.**

# Threads in Rust

1. What is main thread
  
   Every program starts with exactly one thread: **the main thread**. This thread 
   will execute your **main function** and can be used to spawn more threads if
   necessary.

   In the following code snippet, we spawn another thread from our own thread, 
   and pass `f` as **its main function**.

   Returning from main will exit the entire program, even if other threads are
   still running.

   ```rust
   use std::thread::{current, spawn};

   fn main() {
       spawn(f).join().unwrap();
   }

   // Use `f()` as the new thread's main function.
   fn f() {
       let id = current().id();
       println!("Hello from another thread: {:?}", id);
   }
   ```

2. `ThreadId` in Rust

   Rust stdlib assigns every thread an unique number as their identifier.

   > NOTE: Rust stdlib will NOT reuse `ThreadId` even when this thread terminates.
   > 
   > And there is no guarantee that these IDs will be assigned consecutively, only 
   > that they will be different for each thread.

   ```rust
   /// A unique identifier for a running thread.
   ///
   /// A `ThreadId` is an opaque object that uniquely identifies each thread
   /// created during the lifetime of a process. **`ThreadId`s are guaranteed not to
   /// be reused, even when a thread terminates.** `ThreadId`s are under the control
   /// of Rust's standard library and there may not be any relationship between
   /// `ThreadId` and the underlying platform's notion of a thread identifier --
   /// the two concepts cannot, therefore, be used interchangeably. A `ThreadId`
   /// can be retrieved from the [`id`] method on a [`Thread`].
   ///
   /// # Examples
   ///
   /// ```
   /// use std::thread;
   ///
   /// let other_thread = thread::spawn(|| {
   ///     thread::current().id()
   /// });
   ///
   /// let other_thread_id = other_thread.join().unwrap();
   /// assert!(thread::current().id() != other_thread_id);
   /// ```
   ///
   /// [`id`]: Thread::id
   #[stable(feature = "thread_id", since = "1.19.0")]
   #[derive(Eq, PartialEq, Clone, Copy, Hash, Debug)]
   pub struct ThreadId(NonZeroU64);
   ```

3. `println!()` macro

   This macro and `print!()` uses `std::io::Stdout::lock()` to guarantee that 
   it outputs are NOT interruptable, or if we run the following code, we could
   possibily get:

   ```rust
   fn main() {
       pritnln!("hello");
       std::thread::spawn(||{
           println!("World");
       });
   }
   ```

   ```shell
   // The following output is not a REAL result
   $ cargo r -q
   HelWorlold


   ```

4. A thread can return values

   ```rust
   use std::thread::{spawn,JoinHandle};

   fn main() {
       let t1: JoinHandle<i32> = spawn(|| 4 / 2);

       let ret_of_t1 = t1.join().unwrap();
       println!("{}", ret_of_t1);
   }
   ```

   ```shell
   $ cargo r -q
   2
   ```

   `JoinHanlde<T>` is generic over `T`, this `T` is the type of the return value.

# Scoped Threads

1. If we know for sure that a spawned thread will definitely NOT outlive a certain
   scope, that thread could safely borrow things that do not live forever, such as 
   local variables, as long as they outlive that scope.
 
   All threads spawned within the scope that haven’t been manually joined will 
   **be automatically joined** before this function returns, which guarantees 
   that none of the threads spawned in the scope can outlive the scope. 

2. Compare two `spawn()` functions


   ```rust
   // thread::spawn(F)

   pub fn spawn<F, T>(f: F) -> JoinHandle<T>
   where
       F: FnOnce() -> T,
       F: Send + 'static,
       T: Send + 'static,
   {
       Builder::new().spawn(f).expect("failed to spawn thread")
   }
   ```

   ```rust
   // thread::Scope::spawn(&self, F)

   pub fn spawn<F, T>(&'scope self, f: F) -> ScopedJoinHandle<'scope, T>
   where
       F: FnOnce() -> T + Send + 'scope,
       T: Send + 'scope,
   {
       Builder::new().spawn_scoped(self, f).expect("failed to spawn thread")
   }

   // The trait bound in this function can be rewritten as:
   F: FnOnce() -> T
   F: Send + 'scope
   T: Send + 'scope
   ```

   The difference between these two functions is pretty obvious, the lifetime
   parameter.

# Shared Ownership and Reference Counting 

> Any data shared between them will need to live as long as the longest living thread.

1. Ways to share objects between threads

   1. `static` variables

      `static` variables are "owned" by the entire program, which is definitely 
      safe to access from different threads.

      ```rust
      use lazy_static::lazy_static;
      use std::thread::spawn;
      
      lazy_static! {
          static ref V: Vec<i32> = {
              let mut v = Vec::new();
              v.push(1);
              v
          };
      }
      
      fn main() {
          spawn(|| {
              for item in V.iter() {
                  println!("{}", item);
              }
          })
          .join()
          .unwrap();
      }
      ```

      `static` variables in Rust have to be `Sync` as it may be shared across
      threads:

      ```rust
      use std::cell::Cell;

      static C: Cell<()> = Cell::new(());

      fn main() {}
      ```

      ```shell
      $ cargo b -q
      error[E0277]: `Cell<()>` cannot be shared between threads safely
       --> src/main.rs:3:11
        |
      3 | static C: Cell<()> = Cell::new(());
        |           ^^^^^^^^ `Cell<()>` cannot be shared between threads safely
        |
        = help: the trait `Sync` is not implemented for `Cell<()>`
        = note: shared static variables must have a type that implements `Sync`

      For more information about this error, try `rustc --explain E0277`.
      error: could not compile `rust` due to previous error
      ```

      > BTW, diff between `static` and `const`:
      >
      > 0. `const` is `immutable`.
      > 1. `const` is similar to the macros in C, which is actually the replacement
      >    occurrs at compile time.
      > 2. You should NEVER use `interior mutability` with `const` as it is just
      >    a compile-time replacement (as `const` variables are just C macros).
      >
      >    ```rust
      >    use std::sync::Mutex;
      >    const N_CONST: Mutex<i32> = Mutex::new(0);
      >    static N_STATIC: Mutex<i32> = Mutex::new(0);
      >
      >    fn main() {
      >        *N_CONST.lock().unwrap() = 1;
      >        *N_STATIC.lock().unwrap() = 1;
      >
      >        println!("N_CONST: {}", N_CONST.lock().unwrap());
      >        println!("N_STATIC: {}", N_STATIC.lock().unwrap());
      >    }
      >    ```
      >    ```shell
      >    $ cargo b -q
      >    N_CONST: 0
      >    N_STATIC: 1
      >    ```
      > 3. `static` variables must be `Sync`, which is not needed for `const` vars.
      
   2. Leaking

      One can use `pub fn leak<'a>(b: Box<T, A>) -> &'a mut T` to leak an allocated
      memory, the return type can have `'statis` lifetime.

      ```rust
      fn main() {
          let three = Box::new(3);
          let leaked: &'static i32 = Box::leak(three);
          println!("{}", leaked);
      }
      ```

      > NOTE: `'static` lifetime doesn’t mean that the value lived since the 
      > start of the program, but only that **it lives to the end of the program.**
      > The past is simply not relevant.
      >
      > see [common lifetime micconceptions for more details.]
      > (https://github.com/SteveLauC/Notes/blob/main/programming-language/Rust/blog/rust-blog/common-rust-lifetime-misconceptions.md)

      The downside of leaking an object is that we are leaking our memeory.

      ```rust
      use std::thread::spawn;

      fn main() {
          let three = Box::new(3);
          let leaked: &'static i32 = Box::leak(three);
          spawn(move || {
              println!("{}", leaked);
          })
          .join()
          .unwrap();

          // We can still access it as `immutable ref` is `Copy`.
          println!("{}", leaked);
      }
      ```

   3. Reference Counting (Arc)

      Leaking an object gives up its ownership, in most cases, this is not what
      we want. We want to share its ownership, using `Reference Counting`.


      ```rust
      use std::rc::Rc;

      fn main() {
          let v = Rc::new(vec![1]);
          let a_clone = Rc::clone(&v);
          assert_eq!(2, Rc::strong_count(&v));
          drop(a_clone);
          assert_eq!(1, Rc::strong_count(&v));
      }
      ```

      `Rc` is not thread-safe as the inner counter types are not `atomic`:

      ```rust
      pub struct Rc<T: ?Sized> {
          ptr: NonNull<RcBox<T>>,
          phantom: PhantomData<RcBox<T>>,
      }

      struct RcBox<T: ?Sized> {
          strong: Cell<usize>,
          weak: Cell<usize>,
          value: T,
      }
      ```

      ```rust
      pub struct Arc<T: ?Sized> {
          ptr: NonNull<ArcInner<T>>,
          phantom: PhantomData<ArcInner<T>>,
      }

      struct ArcInner<T: ?Sized> {
          strong: atomic::AtomicUsize,

          // the value usize::MAX acts as a sentinel for temporarily "locking" the
          // ability to upgrade weak pointers or downgrade strong ones; this is used
          // to avoid races in `make_mut` and `get_mut`.
          weak: atomic::AtomicUsize,

          data: T,
      }
      ```

2. Pro tip on cloning `Arc` for a newly spawn thread

   Instead of `clone`ing it before calling `spawn()`, we should `clone` it in
   the body of `spawn()`.

   ```rust
   use std::{sync::Arc, thread::spawn};

   fn main() {
       let v = Arc::new(vec![1]);
       let v_clone = Arc::clone(&v);

       spawn(move || {
           println!("{:?}", v_clone);
       })
       .join()
       .unwrap();
   }
   ``` 

   ```rust
   use std::{sync::Arc, thread::spawn};

   fn main() {
       let v = Arc::new(vec![1]);

       spawn({
           let v = Arc::clone(&v);
           move || {
               println!("{:?}", v);
           }
       })
       .join()
       .unwrap();
   }
   ```

   With doing this, we can shadow the original vaiable with the same name in
   the new scope.

# Borrowing and Data Races

1. What is `data race`

   > `data race` will NOT happen in safe Rust due to the existence of the ownership
   > system and the borrow checker.

   A `data race` is:
   1. Two or more threads are accessing the same memory concurrently.
   2. At least one thread tries to write to that memory 
   3. NO synchronization is applied between these threads.

   > [Are "data races" and "race condition" actually the same thing in context 
   > of concurrent programming](https://stackoverflow.com/a/75134842/14092446)
   >
   > `readdir(3)` has a data race.
   >
   > [Why is fs::read_dir() thread safe on POSIX platforms](https://stackoverflow.com/q/74160999/14092446)

# Interior Mutability

> A data type with interior mutability slightly bends the borrowing rules. 
> Under certain conditions, those types can allow mutation through an 
> **immutable** reference.
>
> And keep in mind that interior mutability only allows mutation when the object
> being shared, `exclusive access` is still guaranteed. From the documentation of:
>
> ```rust
> pub fn borrow_mut(&self) -> RefMut<'_, T>
> ```
>
> Panics if the value is currently borrowed.

1. Types with `interior mutability`

   | Types | Thread-Safe |
   |-------|-------------|
   |Cell   |No           |
   |RefCell|No           |
   |Mutex  |Yes          |
   |RwLock |Yes          |
   |AtomicX from `sync::atomic` module |Yes          |

## Cell

> interior mutability by the whole value and thus the APIs of `Cell` won't panic.

Keep in mind that the interior mutability of `Cell` is limited, you need to operate
it as a whole:

```rust
pub fn get(&self) -> T
pub fn replace(&self, val: T) -> T
pub fn set(&self, val: T)
pub fn take(&self) -> T
```

You can mutable the inner value by mutable reference, but this requires that `self`
is mutable, which is gererally not the case when using `Cell`.

```rust
pub fn get_mut(&mut self) -> &mut T
```

## RefCell

> interior mutability by **reference** (Note the `Ref` prefix), which means 
> that the APIs will panic when the **borrow rules** are violated (this is 
> checked at runtime rather than compile time).

```rust
// panics if self is currently mutably borrowed
pub fn borrow(&self) -> Ref<'_, T>
// panics if self is currently borrowed (mutably or immutably)
pub fn borrow_mut(&self) -> RefMut<'_, T>

pub fn try_borrow(&self) -> Result<Ref<'_, T>, BorrowError>
pub fn try_borrow_mut(&self) -> Result<RefMut<'_, T>, BorrowMutError>
```

## Mutex and RwLock

1. `RwLock` is the concurrent version of `RefCell`

   | RefCell    | RwLock |
   |------------|--------|
   |borrow()    |read()  |
   |borrow_mut()|write() |

   There is a notable difference between them is that you can violate borrow rules
   when using `RefCell` and it panics the program. You can not do this with `RwLock`
   as when you try to break the rule, `Gurad` stops you from doing that through
   blocking the program (thread).

2. `Mutex` is similar to `RwLokc` but is simpler, which eliminates the accessing
   purpose (read or write).

## Atomics

> This will be explained in detail in Ch2 and Ch3.

1. The `AtomicXXX`s from `sync::atomics` module are the concurrent and concrete
   versions of `Cell`

   There is no gerneric version of `Atomic<T>` but some specific types:

   ```rust
   AtomicBool	A boolean type which can be safely shared between threads.
   AtomicI8	An integer type which can be safely shared between threads.
   AtomicI16	An integer type which can be safely shared between threads.
   AtomicI32	An integer type which can be safely shared between threads.
   AtomicI64	An integer type which can be safely shared between threads.
   AtomicIsize	An integer type which can be safely shared between threads.
   AtomicPtr	A raw pointer type which can be safely shared between threads.
   AtomicU8	An integer type which can be safely shared between threads.
   AtomicU16	An integer type which can be safely shared between threads.
   AtomicU32	An integer type which can be safely shared between threads.
   AtomicU64	An integer type which can be safely shared between threads.
   AtomicUsize	An integer type which can be safely shared between threads
   ```

   Not all types are available since they need support from hardware.

## UnsafeCell

The core primitive for interior mutability in Rust.

An UnsafeCell<T> wraps a T, but does not come with any conditions or restrictions 
to avoid undefined behavior. Instead, its get() method just gives a raw pointer 
to the value it wraps, which can only be meaningfully used in unsafe blocks. It 
leaves it up to the user to use it in a way that does not cause any undefined behavior.

Most commonly, an UnsafeCell is not used directly, but wrapped in another type 
that provides safety through a limited interface, such as Cell or Mutex. All 
types with interior mutability—including all types discussed above—are built on 
top of UnsafeCell.

# Thread Safety: Send and Sync
