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
> * [Threads In Rust](#threads-in-rust)
> * [Scoped Threads](#threads-in-rust)
> * [Shared Ownership and Reference Counting](#shared-ownership-and-reference-counting)
> * [Borrowing and Data Races](#borrowing-and-data-races)
> * [Interior Mutability](#interior-mutability)
>   * [Cell](#cell)
>   * [RefCell](#RefCell)
>   * [Mutex and RwLock](#mutex-and-rwlock)
>   * [Atomic](#atomic)
> * [Locking: Mutexes and RwLocks](#locking-mutexes-and-rwLocks)
>   * [Rust’s Mutex](#rusts-mutex)
>   * [Lock Poisoning](#lock-poisoning)
>   * [Reader-Writer Lock](#reader-writer-lock)
> * [Waiting: Parking and Condition Variables](#waiting-parking-and-condition-variables)
>   * [Thread Parking](#thread-parking) 
>   * [Condition Variables](#condition-varables)
> * [Summary](#summary)

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

   Rust stdlib assigns every thread an unique number as its identifier.

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

2. Compare two `spawn()` functions, notice the trait bound:


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
   parameter bound of `F` and `T`.

# Shared Ownership and Reference Counting 

> Any data shared between threads will need to live as long as the longest living thread.

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

      // Cell is NOT Sync
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
      >    a compile-time replacement.
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
      > The past is simply NOT relevant.
      >
      > see 
      > [common lifetime micconceptions for more details.](https://github.com/SteveLauC/Notes/blob/main/programming-language/Rust/blog/rust-blog/common-rust-lifetime-misconceptions.md)

      > The downside of leaking an object is that we are leaking our memeory.

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

   3. Reference Counting (Rc or Arc)

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
      // Def of `Rc`

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
      In `RcBox`, `strong` field has type `Cell<usize>`. In `ArcInner`, `strong`
      field is of type `AtomicUsize`, you can see that `Atomic` is the concurrent
      version of `Cell`.

      ```rust
      // Def of `Arc`

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

2. Pro tip on cloning `Arc` for a newly spawned thread

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
   > `readdir(3)` has a race condition:
   > [Why is fs::read_dir() thread safe on POSIX platforms](https://stackoverflow.com/q/74160999/14092446)

# Interior Mutability

> A data type with interior mutability slightly bends the borrowing rules. 
> Under certain conditions, those types can allow mutation through an 
> **immutable** reference.
>
> But keep in mind that interior mutability only allows mutation when the object
> being shared, `exclusive access` is still guaranteed. 这段话的意思是，可以共享
> ，但在共享的基础上想要改变它，在任何时间，改变必须是独占的。From the 
> [documentation](https://doc.rust-lang.org/std/cell/struct.RefCell.html#method.borrow_mut) 
> of:
>
> ```rust
> pub fn borrow_mut(&self) -> RefMut<'_, T>
> ```
>
> **Panics** if the value is currently borrowed.

1. Types with `interior mutability`

   | Types | Thread-Safe |
   |-------|-------------|
   |Cell   |No           |
   |RefCell|No           |
   |Mutex  |Yes          |
   |RwLock |Yes          |
   |AtomicX from `sync::atomic` module |Yes          |
   |UnsafeCell | Definitely NO |

## Cell<T>

> interior mutability **by the whole value** and thus the APIs of `Cell` won't panic.

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

## RefCell<T>

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

Fun fact: `RefCell` does not protect you from data races since `RefCell` is 
NOT thread-safe and data races only happen when multiple threads are involved.

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

2. `Mutex` is similar to `RwLock` but is simpler, which eliminates the accessing
   purpose (read or write).

## Atomics

> This will be explained in detail in Ch2 and Ch3.

1. The `AtomicXXX`s from `sync::atomics` module are the concurrent and concrete
   versions of `Cell`

   > In `RcBox`, `strong` field has type `Cell<usize>`. In `ArcInner`, `strong`
   > field is of type `AtomicUsize`, you can see that `Atomic` is the concurrent
   > version of `Cell`.

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

An `UnsafeCell<T>` wraps a T, but does not come with any conditions or restrictions 
to avoid undefined behavior. Instead, its get() method just gives a raw pointer 
to the value it wraps, which can only be meaningfully used in unsafe blocks. It 
leaves it up to the user to use it in a way that does not cause any undefined behavior.

Most commonly, an UnsafeCell is not used directly, but wrapped in another type 
that provides safety through a limited interface, such as Cell or Mutex. All 
types with interior mutability—including all types discussed above—are built on 
top of UnsafeCell.

# Thread Safety: Send and Sync

1. Definitions

   * Send: 
     A type is Send if it **can be sent to another thread**. In other words, **if 
     ownership of a value of that type can be transferred to another thread.**
     For example, Arc<i32> is Send, but Rc<i32> is not.

   * Sync:
     A type is Sync if it can be **shared with another thread**. In other words,
     a type T is Sync if and only if a shared reference to that type, &T, is
     Send. For example, an i32 is Sync, but a Cell<i32> is not. (A Cell<i32>
     is Send, however.)

     > If you wanna **share** T with another thread, you create a shared reference
     > `&T` and pass it to that thread. To make this happen, `&T` must be `Send`.

2. Typical types that are NOT `Sync`

   > If the following types are used in Multi-threaded environments, data races
   > may happen.

   |Type         |  Reason                  |
   |-------------|--------------------------|
   |Rc           |The counter is NOT atomic |
   |Cell         |No synchronization applied|
   |RefCell      |No synchronization applied|
   | Raw Ptr     |                          | 

3. Typical types that are NOT `Send`

   |Type         |  Reason                  |
   |-------------|--------------------------|
   |Rc           |Because `Rc` is NOT `Sync`|
   |Raw Ptr      |                          |  

   > Explanation of the reason why `Rc` is NOT `Send`:
   >
   > For `Rc`, I think it is fine to pass it to another thread as long as you 
   > don't clone it, in this perspective, it should be `Send`. But clone is 
   > allowed, if multiple threads have an Rc to the same allocation, they might
   > try to modify the reference counter at the same time, which can give
   > unpredictable results.

4. Types that are `Send` but not `Sync`

   1. Cell
   2. RefCell

5. Types that are `Sync` but not `Send`
  
   1. MutexGuard
   2. RwLockReadGuard
   3. RwLockWriteGuard

   > ref: [Sync but not Send?](https://users.rust-lang.org/t/sync-but-not-send/21551)

6. How to make your own type NOT `Send` or `Sync`

   `Send` and `Sync` are markder traits that are mostly controlled by the 
   compiler, if the compiler thinks your type is `Send` or `Sync`, it will
   mark it auotmatically.

   ```rust
   // Foo is Send and Sync

   struct Foo;
   ```

   If you try to use `!` to explicitly mark it un`Send` or un`Sync`:

   ```rust
   struct Foo;

   impl !Send for Foo {}
   impl !Sync for Foo {}

   fn main() {}
   ```

   ```shell
   error[E0658]: negative trait bounds are not yet fully implemented; use marker types for now
    --> src/main.rs:3:6
     |
   3 | impl !Send for Foo {}
     |      ^^^^^
     |
     = note: see issue #68318 <https://github.com/rust-lang/rust/issues/68318> for more information

   error[E0658]: negative trait bounds are not yet fully implemented; use marker types for now
    --> src/main.rs:4:6
     |
   4 | impl !Sync for Foo {}
     |      ^^^^^
     |
     = note: see issue #68318 <https://github.com/rust-lang/rust/issues/68318> for more information

   For more information about this error, try `rustc --explain E0658`.
   error: could not compile `rust` due to 2 previous errors
   ```

   The current workaround is to add a non-`Send` or non-`Sync` field to your type:

   ```rust
   use std::{marker::PhantomData, rc::Rc};

   // Foo is not Send and Sync due to the usgae of `Rc`.
   //
   // Use `PhantomData<Cell<()>>` if you wanna make it `Send` but `!Sync`
   // Use `PhantomData<MutexGuard<'a, ()>>` if you wanna make it `Sync` but `!Send`
   struct Foo {
       _not_send_and_sync: PhantomData<Rc<()>>,
   }

   fn main() {}
   ```

7. How to make your own type `Send` or `Sync`

   ```rust
   use std::rc::Rc;
   struct Foo {
       rc: Rc<()>
       // raw ptr also works 
   }

   unsafe impl Send for Foo {}
   unsafe impl Sync for Foo {}
   ```

   use `unsafe impl` to overwrite the compiler's auto impl.

8. Demostrate the data races that may happen with `Rc` if it can be shared 
   with threads:

   > The demo below is WRONG. Haven't figured out a way to achieve this.

   ```rust
   use std::{rc::Rc, thread::spawn};

   struct MyRc(Rc<i32>);

   unsafe impl Send for MyRc {}
   unsafe impl Sync for MyRc {}

   impl Clone for MyRc {
       fn clone(&self) -> Self {
           MyRc(Rc::clone(&self.0))
       }
   }

   fn main() {
       let rc = MyRc(Rc::new(0));

       let t1 = spawn({
           let rc = MyRc::clone(&rc);
           move || {
               let _another_clone = MyRc::clone(&rc);
           }
       });

       let t2 = spawn({
           let rc = MyRc::clone(&rc);
           move || {
               let _another_clone = MyRc::clone(&rc);
           }
       });

       t1.join().unwrap();
       t2.join().unwrap();

       println!("strong count: {}", Rc::strong_count(&rc.0));
   }
   ```

   ```shell
   $ cargo r -q
   strong count: 1
   ```

9. Traits implementations on closures

   > Remeber the trait bound of `F` in `std::thread::spawn()`?

   > ref: [Closure Types](https://doc.rust-lang.org/reference/types/closure.html#other-traits)

   All closure types implement Sized. Additionally, closure types implement the
   following traits if allowed to do so by the types of the captures it stores:

   * Clone
   * Copy
   * Sync
   * Send
   The rules for Send and Sync match those for normal struct types, while Clone 
   and Copy behave as if derived. For Clone, the order of cloning of the captured
   variables is left unspecified.

   Because captures are often by reference, the following general rules arise:

   * A closure is Sync if all captured variables are Sync.
   * A closure is Send if all variables captured by non-unique immutable reference
     are Sync, and all values captured by unique immutable or mutable reference, 
     copy, or move are Send.
   * A closure is Clone or Copy if it does not capture any values by unique 
     immutable or mutable reference, and if all values it captures by copy or
     move are Clone or Copy, respectively.

# Locking: Mutexes and RwLocks

When a thread locks an unlocked mutex, the mutex is marked as locked and the 
thread can immediately continue. When a thread then attempts to **lock an 
already locked mutex**, that operation will **block**. The thread is put to sleep 
while it waits for the mutex to be unlocked. 

Unlocking is only possible on a locked mutex, and **should be done by the same
thread that locked it**. If other threads are waiting to lock the mutex, 
unlocking will **cause one of those threads to be woken up**, so it can try to 
lock the mutex again and continue its course.

## Rust’s Mutex

1. Why `MutexGuard` is not `Send`

   A locked mutex should be unlocked by the thread that locked it, in Rust, this
   is implemented by an RAII type `MutexGuard`, when it goes out of scope, the 
   lock will be released.

   If `MutexGuard` is `Send` and can be passed to another thread, then the thread
   that locks it and the one that unlocks it can be different threads.

2. Atomic operation

   ```rust
   use std::sync::Mutex;

   fn main() {
       let n = Mutex::new(0);
       thread::scope(|s| {
           for _ in 0..10 {
               s.spawn(|| {
                   let mut guard = n.lock().unwrap();
                   for _ in 0..100 {
                       *guard += 1;
                   }
               });
           }
       });
       assert_eq!(n.into_inner().unwrap(), 1000);
   }
   ```

   Each thread incrases n by 100, and due to the usage of `Mutex<T>`, this 
   increment is **atomic**.

3. One great example to understand the concurrency of multi-threads

   This shows the importance of keeping the amount of time a mutex is locked
   **as short as possible**. Keeping a mutex locked longer than necessary can 
   completely nullify any benefits of parallelism, effectively forcing everything
   to happen serially instead.

   ```rust
   // This prpgram takes 10 seconds to complete since now the 10 threads
   // can execute their one-second sleep at the same time.
   use std::time::Duration;

   fn main() {
       let n = Mutex::new(0);
       thread::scope(|s| {
           for _ in 0..10 {
               s.spawn(|| {
                   let mut guard = n.lock().unwrap();
                   for _ in 0..100 {
                       *guard += 1;
                   }
                   thread::sleep(Duration::from_secs(1)); // New!
               });
           }
       });
       assert_eq!(n.into_inner().unwrap(), 1000);
   }
   ```

   ```rust
   // This program takes ONLY 1 second to complete

   fn main() {
       let n = Mutex::new(0);
       thread::scope(|s| {
           for _ in 0..10 {
               s.spawn(|| {
                   let mut guard = n.lock().unwrap();
                   for _ in 0..100 {
                       *guard += 1;
                   }
                   drop(guard); // New: drop the guard before sleeping!
                   thread::sleep(Duration::from_secs(1));
               });
           }
       });
       assert_eq!(n.into_inner().unwrap(), 1000);
   }
   ```

## Lock Poisoning

1. What is Posioning

   A Mutex in Rust gets marked as poisoned when a thread panics while holding
   the lock. When that happens, the Mutex will **no longer be locked**(i.e., the
   lock will be released). Calling `lock()` on a poisoned lock will result 
   in **an Err** to indicate it has been poisoned, but **STILL locks that 
   lock** and acquires the gurad (carried by that `Err` variant).

   ```rust
   pub type LockResult<Guard> = Result<Guard, PoisonError<Guard>>;

   // A type alias for the result of a lock method which can be poisoned.

   // The Ok variant of this result indicates that the primitive was not poisoned,
   // and the Guard is contained within. The Err variant indicates that the 
   // primitive was poisoned. **Note that the Err variant also carries the associated
   // guard**, and it can be acquired through the into_inner method.
   ```

   This is a mechanism to protect against leaving the data that’s protected 
   by a mutex **in an inconsistent state**, make the lock poisoned to force
   the user (programmer) to handle such cases.

   While lock poisoning might seem like a powerful mechanism, recovering from 
   a potentially inconsistent state is not often done in practice. Most code 
   either disregards poison or uses unwrap() to panic if the lock was poisoned,
   effectively propagating panics to all users of the mutex.


   ```rust
   use std::{
       sync::{Arc, Mutex},
       thread::{current, spawn},
   };

   fn main() {
       let v = Arc::new(Mutex::new(vec![1]));

       let _ = spawn({
           let v = Arc::clone(&v);
           move || {
               let _lock_guard = v.lock().unwrap();
               panic!("thread {:?} panicked while hold the lock guard, the lock becomes poisoned.", current().id());
           }
       }).join();

       if v.is_poisoned() {
           println!("Main thread: Ohh, the lock is poisoned.");
       }
   }
   ```
   ```shell
   $ cargo r -q
   thread '<unnamed>' panicked at 'thread ThreadId(2) panicked while hold the lock guard, the lock becomes poisoned.', src/main.rs:13:13
   stack backtrace:
      0: rust_begin_unwind
                at /rustc/90743e7298aca107ddaa0c202a4d3604e29bfeb6/library/std/src/panicking.rs:575:5
      1: core::panicking::panic_fmt
                at /rustc/90743e7298aca107ddaa0c202a4d3604e29bfeb6/library/core/src/panicking.rs:65:14
      2: rust::main::{{closure}}
                at ./src/main.rs:13:13
   note: Some details are omitted, run with `RUST_BACKTRACE=full` for a verbose backtrace.
   Main thread: Ohh, the lock is poisoned.
   ```

2. The lifetime of `MutexGuard`

   > The lifetime here does not mean the generic lifetime parameter of 
   > `MutexGuard<'a, T>` but the duration (or scope) it exists.

   The lifetime of a `MutexGuard` means how long we want to lock that item
   (RAII), which really matters in concurrent programming.

   If we create a `Mutex` to protect a vector:

   ```rust
   let v = Mutex::new(Vec::new());  
   ```

   And wanna to push an item to it, we can do this:

   ```rust
   v.lock().unwrap().push(1); // MutexGuard dropped here
   ```

   In the above single statement, `MutexGuard` was destructured at the end of
   that statement.

   If we wanna pop an item from it and process that item with some function, we
   may write something like this:

   ```rust
   if let Some(item) = v.lock().unwrap().pop() {
       process(item);
   }
   ```
   One should note the lifetime of that `MutexGuard`, it lives up to the end of
   that `if let` statement.

   > This will happen in:
   > 1. if let
   > 2. while let
   > 3. match
   >
   > Perhaps surprisingly, this does not happen for a similar `if` statement:
   > ```rust
   > if v.lock().unwrap().pop() == Some(1) {
   >     // do something
   > }
   > ```
   > as the condition of an `if` statement is a plain `bool` value.

   Here is the trick: if the operation you are gonna do borrows (e.g., `front()`
   of `Vec`) the data protected by the lock, then it will be necessary to keep 
   the Guard around. If not, then just drop that guard.

## Reader-Writer Lock

`RwLock` is similar to `Mutex` but has more concrete accessing purposes, read
or write, while `Mutex` is just about exclusive access.

1. The states of a `RwLock`

   1. unlocked
   2. locked by a single writer (exclusive access)
   3. locked by one or multiple reader (shared access)

   > As a contrast, `Mutex` has ONLY two states, `locked` and `unlocked`.

3. Both `Mutex<T>` and `RwLock<T>` requires `T` to be `Send`, `RwLock<T>` also
   requires `T` to be `Sync`.

   > Strictly speaking, you can create a lock for a T that doesn’t fulfill 
   > these requirements, but you wouldn’t be able to share it between threads
   > as the lock itself won’t implement Sync.

   > Currently IDK why...

# Waiting(Sleeping): Parking and Condition Variables

When data is mutated by multiple threads, there are many situations where 
they would need to wait(sleep/suspended) for some event, for some condition about the data 
to become true. For example, if we have a mutex protecting a Vec, we might
want to wait until it contains anything.

While a mutex does allow threads to wait **until it becomes unlocked**, it does
not provide functionality for **waiting for any other conditions**. If a mutex 
was all we had, we’d have to keep locking the mutex to repeatedly check if 
there’s anything in the Vec yet.

## Thread Parking

1. Locks (Mutex and RwLock) are used for synchronization (protection), while 
   parking is used for **collaboration** (wait for something to happen to data
   protected).

   Let's give an example where two thread communicate through a queue, spawned
   thread consumes that queue, main thread fills that queue. Spawned thread
   will be parked until the queue is not empty (when main thread fills something,
   it unparks the spawned thread).

   ```rust
   use std::{
       collections::VecDeque,
       sync::{Arc, Mutex},
       thread::{park, sleep, spawn},
       time::Duration,
   };

   fn main() {
       let queue = Arc::new(Mutex::new(VecDeque::new()));

       let t1_handle = spawn({
           let queue = Arc::clone(&queue);
           move || loop {
               let opt_item = queue.lock().unwrap().pop_front();
               if let Some(item) = opt_item {
                   println!("Spawned Thread: get item {:?}", item);
               } else {
                   park();
               }
           }
       });
       let spawned_thread = t1_handle.thread();

       loop {
           queue.lock().unwrap().push_back(0);
           spawned_thread.unpark();
           sleep(Duration::from_secs(1));
       }
   }
   ```

   ```shell
   $ cargo r -q
   Spawned Thread: get item 0
   Spawned Thread: get item 0
   Spawned Thread: get item 0
   Spawned Thread: get item 0
   ^C
   ```

2. APIs provided by stdlib

   1. To park

      A thread can park itself, which **puts it to sleep**(pausing it), stopping
      it from consuming any CPU cycles. This function will NOT return unitil the
      parked thread is unparked.

      ```rust
      std::thread::park()
      ```

      Blocks unless or until the current thread’s token is made available.

      A call to park does not guarantee that the thread will remain parked forever,
      and callers should be prepared for this possibility. However, it is 
      guaranteed that this function will not panic (it may abort the process if 
      the implementation encounters some rare errors).
  
   2. To unpark

      Another thread can then unpark the parked thread, waking it up from its
      nap.

      ```rust
      std::thread::Thread::unpark(&self);
      ```

      Atomically makes the handle’s token available if it is not already.

      Every thread is equipped with some basic low-level blocking support, 
      via the park function and the unpark() method. These can be used as a
      more CPU-efficient implementation of a spinlock.

      See the park documentation for more details.

3. An important property of thread parking

   If you `unpark()` a thread that is not parked, this call does not get lost
   and is recorded, when that thread gets parked in the future, it clears that
   `unpark` record and **continue without going to sleep**.

   However, unpark requests **don’t stack up**(i.e., there will be at most 1 
   request saved). Calling unpark() two times and then calling park() two 
   times afterwards still results in the thread going to sleep. The first 
   park() clears the request and returns directly, but the second one goes 
   to sleep as usual.

## Condition Variables

Think about this example, we have multiple consumer threads, if we still use
the mechanism used in the above example, the producer thread must know exactly
which thread is waiting and should be waken up, parking and unparking is not
suitable for such a complicated case, `condition variables` come to rescue.

1. Two basic operations
   
   1. wait 
   2. notify. 

   Both can be used with one thread or multiple threads.

   > The condition variable takes care of delivering the notifications to 
   > **whichever thread is interested**.


2. Rewrite the example in `thread parking` with `Condvar`

   In the following code, we combine our queue and the condvar together and wrap
   them in an `Arc` so that we can share it with other threads. I guess this should
   be a typical usage.

   ```rust
   use std::{
       collections::VecDeque,
       sync::{Arc, Condvar, Mutex},
       thread::{sleep, spawn},
       time::Duration,
   };

   fn main() {
       // our queue
       // and
       // a condition variable for the event that the queue becomes not empty
       let queue_and_condvar =
           Arc::new((Mutex::new(VecDeque::new()), Condvar::new()));

       spawn({
           let queue_and_condvar = Arc::clone(&queue_and_condvar);
           move || loop {
               // .wait() will block the current thread until the event happened.
               let mut guard = queue_and_condvar
                   .1
                   .wait(queue_and_condvar.0.lock().unwrap())
                   .unwrap();

               // Alright, queue becomes non-empty, pop it.
               let item = guard.pop_front().unwrap();
               // drop it once it is not needed.
               drop(guard);
               println!("Spawned thread get item: {}", item);
           }
       });

       for i in 0.. {
           queue_and_condvar.0.lock().unwrap().push_back(i);
           queue_and_condvar.1.notify_all();
           sleep(Duration::from_secs(1));
       }
   }
   ```
   ```shell
   $ cargo r -q
   Spawned thread get item: 0
   Spawned thread get item: 1
   Spawned thread get item: 2
   Spawned thread get item: 3
   Spawned thread get item: 4
   ^C
   ```

# Summary

1. Cell and atomics only allow replacing the value as a whole, while RefCell, 
   Mutex, and RwLock allow you to mutate the value directly by dynamically 
   enforcing access rules.

2. Thread parking can be a convenient way to wait for some condition.

3. When a condition is about data protected by a Mutex, using a Condvar is 
   more convenient, and can be more efficient, than thread parking.
