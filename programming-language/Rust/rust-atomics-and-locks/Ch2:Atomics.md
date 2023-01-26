> In the second chapter we’ll learn about Rust’s atomic types and all their 
> operations. We start with simple load and store operations, and build our way
> up to more advanced compare-and-exchange loops, exploring each new concept 
> with several real-world use cases as usable examples.
>
> While memory ordering is relevant for every atomic operation, that topic is 
> left for the [next chapter](./Ch3:Memory-Ording.md). This chapter only 
> covers situations where relaxed memory ordering suffices, which is the case 
> more often than one might expect.

> TOC
>
> * [Atomic Load and Store Operations](#atomic-load-and-store-operations)
>   * [Example: Stop Flag](#example-stop-flag)
>   * [Example: Progress Reporting](#example-progress-reporting)
>   * [Example: Lazy Initialization](#example-lazy-initialization)
> * [Fetch-and-Modify Operations](#fetch-and-modify-operations)
>   * [Example: Progress Reporting from Multiple Threads](#example-progress-reporting-from-multiple-threads)
>   * [Example: ID Allocation](#example-id-allocation)
> * [Compare-and-Exchange Operations](#compare-and-exchange-operations)
>   * [Example: ID Allocation Without Overflow](#example-id-allocation-without-overflow)
>   * [Example: Lazy One-Time Initialization](#example-lazy-one-time-initialization)
> * [Summary](#summary)

1. Atomic operation is an operation that is indivisible, which means it is either
completed, or it did't happen yet.

2. Data races will never happen with atomic types as their operations are atomic.

   It is inherently safe...

   For `Mutex` and `RwLock`, data races won't happen because it blocks the 
   threads that try to access the locked variable.

2. Atomic operations are the **main building blocks** for anything involving 
   multiple threads. All the other concurrency primitives, such as mutexes and 
   condition variables, are implemented using atomic operations.

3. Interfaces exposed by `AtomicXXX` types:
  
   1. Load and store
   2. Fetch and modify
   3. Compare and exchange

4. An important concept: `Memory Ordering`

   > This concept will be explained in detail in Ch3.

   Atomic variables are primarily used to synchronize shared memory accesses 
   between threads. Typically one thread creates data, then stores to an atomic.
   Other threads read from this atomic, and when the expected value is seen, 
   the data the other thread was creating is going to be complete and visible 
   in this thread.

   The different Memory Orderings are used to indicate **how strong this 
   data-sharing bond is** between threads. Knowledgeable programmers can utilize
   the weaker orderings to make more efficient software.

   Rust has the same memory ordering as C++20.

   ```rust
   pub enum Ordering {
       Relaxed,
       Release,
       Acquire,
       AcqRel,
       SeqCst,
   }
   ```

   Ref:
   * [Wikipedia: Memory Ordering](https://en.wikipedia.org/wiki/Memory_ordering)
   * [GCC wiki: AtomicSync](http://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync)

# Atomic Load and Store Operations

```rust
impl AtomicI32 {
    /// Atomically loads the value stored in the atomic variable `self`
    pub fn load(&self, ordering: Ordering) -> i32;
    /// Atomically stores a new value in `self` 
    ///
    /// NOTE: store() takes a shared ref instead of a exclusive ref (interior 
    /// mutability)
    pub fn store(&self, value: i32, ordering: Ordering);
}
```

## Example: Stop Flag

In this example, we use an `AtomicBool` as a STOP flag to indicate another thread
it is time to stop.

```rust
use std::{
    io::stdin,
    sync::atomic::{AtomicBool, Ordering},
    thread::spawn,
};

static STOP_FLAG: AtomicBool = AtomicBool::new(false);

fn main() {
    // Spawn a thread to do its job: looping...
    let t = spawn(|| loop {
        if STOP_FLAG.load(Ordering::Relaxed) {
            println!("Spawned Thread: STOP");
            break;
        }
    });

    // read user's cmd
    for res_line in stdin().lines() {
        match res_line.unwrap().as_str() {
            // time to stop, set the STOP flag.
            "stop" => {
                STOP_FLAG.store(true, Ordering::Relaxed);
                break;
            }
            unknown_cmd => eprintln!("Unknown CMD: {}", unknown_cmd),
        }
    }

    t.join().unwrap();
}
```

```shell
$ cargo r -q
help
Unknown CMD: help
stop
Spawned Thread: STOP
```

Recall the example give in 
[Waiting: Parking and Condition Variables](https://github.com/SteveLauC/Notes/blob/main/programming-language/Rust/rust-atomics-and-locks/Ch1:Basics-of-Rust-Concurrency.md#waiting-parking-and-condition-variables),
we use parking and `Condvar` to inform the thread (wake it up). In this Stop flag
example, we notify the thread to stop.

This simple solution works great as long as the flag is regularly checked by 
the background thread. If it gets stuck in `some_work()` for a long time, that
can result in an unacceptable delay between the stop command and the program 
quitting.

## Example: Progress Reporting

In this example, we process 100 items one by one on a background thread, 
while the main thread gives the user regular updates on the progress:

```rust
use std::{
    sync::atomic::{AtomicUsize, Ordering},
    thread::{sleep, spawn},
    time::Duration,
};

static PROGRESS: AtomicUsize = AtomicUsize::new(0);

fn main() {
    // process 10 items, assume one item needs 2 seconds
    spawn(|| {
        for time in 1..=10 {
            sleep(Duration::from_secs(2));
            PEOGRESS.store(time, Ordering::Relaxed);
        }
    });

    loop {
        let progress = PROGRESS.load(Ordering::Relaxed);
        println!("Progress: {:2}/10", process);

        // The main thread is waiting for the spawn thread so that we don't 
        // need to manually join it.
        if progress == 10 {
            break;
        }
        sleep(Duration::from_secs(1));
    }
    println!("Done");
}
```

In the above implementation, the main thread repeatedly queries the `PROCESS`
variable once per second, which is inefficient, we can park the main thread 
and let the spawn thread to wake up the main thread when an item is processed.

```rust
use std::{
    sync::atomic::{AtomicUsize, Ordering},
    thread::{current, park, scope, sleep},
    time::Duration,
};

fn main() {
    let progress = AtomicUsize::new(0);
    let main_thread = current();

    // use scoped thread so that we can borrow `main_thread`
    scope(|s| {
        s.spawn(|| {
            for time in 0..=10 {
                sleep(Duration::from_secs(2));
                progress.store(time + 1, Ordering::Relaxed);
                // wake up the main thread
                main_thread.unpark();
            }
        });

        loop {
            park_timeout(Duration::from_secs(2));

            // When the main thread is back, inform the user of the process.
            let progress = process.load(Ordering::Relaxed);
            println!("Progress: {:2}/10", progress);
            if progress == 100 {
                break;
            }
        }
    });

    println!("Done");
}
```
```shell
$ cargo r -q
Process:  1/10
Process:  2/10
Process:  3/10
Process:  4/10
Process:  5/10
Process:  6/10
Process:  7/10
Process:  8/10
Process:  9/10
Process: 10/10
Done
```

Now, any status updates are immediately reported to the user, while still 
repeating the last update every 2 seconds to show that the program is still 
running.

## Example: Lazy Initialization

Imagine there is a `X`, whose value is obtained at runtime. All threads in our
process need this value, and the first thread that tries to access it will be
responsible for getting it, since it is needed by all threads, after the first
initialization, it will be stored in a static atomic variable.

```rust
use std::sync::atomic::{AtomicUsize, Ordering};

/// Value 0 represents that X is uninitialized.
static X: AtomicUsize = AtomicUsize::new(0);

pub fn init_x() -> usize {
    /// Emulate a runtime obtainment.
    fn obtain_x() -> usize {
        1
    }

    let mut x = X.load(Ordering::Relaxed);
    if x == 0 {
        // obtain and set it
        x = obtain_x();
        X.store(x, Ordering::Relaxed);
    }
    x
}

fn main() {}
```

The first thread to call `init_x()` will check the static X and see it is still 
zero, obtain its value, and store the result back in the static to make it 
available for future use. 

However, if a second thread calls `init_x()` while the first one is still 
calculating x, the second thread will also see a zero and also calculate x in 
parallel. One of the threads will end up overwriting the result of the other,
depending on which one finishes first. This is called a **race (race condition)**. Not a data 
race, which is undefined behavior and impossible in Rust without using unsafe, 
**but still a race with an unpredictable winner**.

# Fetch-and-Modify Operations

You may wonder why do we need these operation since we already have `load()`
and `store()`, they are different, `fetch-and-modify` is an **atomic** operation

Take `AtomicI32` as an example, if we want to add 10 to the original value:

```rust
let value = AtomicI32::new(0);
let mut load_value = value.load(Ordering::Relaxed);
load_value += 10;
value.store(load_value, Ordering::Relaxed);
```
```rust
let value = AtomicI32::new(0);
value.fetch_add(10, Ordering::Relaxed); // Atomic!!!
```

In the perspective of functionality, the above two code snippets are same, but
the second one turns three steps (load, add, store) into an indivisible, atomic
operation.

1. Signitures, using `AtomicI32` as an example:
  
   ```rust
   impl AtomicI32 {
       pub fn fetch_add(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_sub(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_or(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_and(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_nand(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_xor(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_max(&self, v: i32, ordering: Ordering) -> i32;
       pub fn fetch_min(&self, v: i32, ordering: Ordering) -> i32;
       // Alias for "fetch_store"
       pub fn swap(&self, v: i32, ordering: Ordering) -> i32; 
   }
   ```

2. Overflow behaviors of `fetch_add()` and `fetch_sub()`: **Wrapping**

   An important thing to keep in mind is that `fetch_add()` and `fetch_sub()` 
   implement **wrapping behavior** for overflows. Incrementing a value past 
   the maximum representable value will wrap around and result in the minimum
   representable value. This is different than the behavior of the plus and 
   minus operators on regular integers, which will panic in debug mode on 
   overflow.

   ```rust
   use std::sync::atomic::{AtomicU8, Ordering};

   fn main() {
       let n = AtomicU8::new(u8::MAX);
       n.fetch_add(1, Ordering::Relaxed);

       assert_eq!(n.into_inner(), 0);
   }
   ```

   > This will be explained in later 
   > [section:Compare-and-Exchange Operations](#compare-and-exchange-operations)

## Example: Progress Reporting from Multiple Threads

> In the previous [section: Example Progress Reporting](#example-progress-reporting),
> we have ONE thread working in the background, and our main thread is responsible
> for notify the user of the progress. In this example, we have 4 thread for 
> working in the background.

If we simply change the threads number to 4, then the program will be wrong:

```rust
use std::{
    sync::atomic::{AtomicUsize, Ordering},
    thread::{current, park_timeout, scope, sleep},
    time::Duration,
};

fn main() {
    let progress = AtomicUsize::new(0);
    let main_thread = current();

    scope(|s| {
        for _ in 0..4 {
            s.spawn(|| {
                for _ in 0..5 {
                    // work
                    sleep(Duration::from_secs(2));
                    // NOTE: race condition (not data race) happens here
                    progress.store(
                        progress.load(Ordering::Relaxed) + 1,
                        Ordering::Relaxed,
                    );
                    main_thread.unpark();
                }
            });
        }

        loop {
            park_timeout(Duration::from_secs(2));

            let progress = progress.load(Ordering::Relaxed);
            println!("Progress: {:2}/20", progress);
            if progress == 100 {
                break;
            }
        }
        println!("Done");
    })
}
```

```shell
$ cargo r -q
Progress:  0/20
Progress:  1/20
Progress:  3/20
Progress:  5/20
Progress:  6/20
Progress:  9/20
Progress: 10/20
Progress: 11/20
Progress: 12/20
Progress: 14/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
Progress: 15/20
^C
```

If we change:
```rust
// NOTE: race condition (not data race) happens here
progress.store(
    progress.load(Ordering::Relaxed) + 1,
    Ordering::Relaxed,
);
```
to:
```rust
// NOTE: race condition resolved
progress.fetch_add(1, Ordering::Relaxed);
```

```shell
$ cargo r -q
Progress:  2/20
Progress:  3/20
Progress:  4/20
Progress:  7/20
Progress:  8/20
Progress: 10/20
Progress: 12/20
Progress: 16/20
Progress: 20/20
Done
```

## Example: ID Allocation

1. Those `assert!` macros can be used with customized message:

   ```rust
   assert!(1 < 0, "message");
   assert_eq!(1, 0, "message");
   ```

2. `static` variables can ONLY be initialized once

   We create `ID` in the scope of function `allocate_id()`, then it can be ONLY
   used within this scope. Calling this function multiple times WON'T initialize
   `ID` for that amount of times, but ONLY ONCE.

   ```rust
   use std::sync::atomic::{AtomicUsize, Ordering};

   fn main() {
       for _ in 0..10 {
           println!("{}", allocate_id());
       }
   }

   fn allocate_id() -> usize {
       static ID: AtomicUsize = AtomicUsize::new(0);
       dbg!(&ID);
       ID.fetch_add(1, Ordering::Relaxed)
   }
   ```
   ```shell
   $ cargo r -q
   [src/main.rs:11] &ID = 0
   0
   [src/main.rs:11] &ID = 1
   1
   [src/main.rs:11] &ID = 2
   2
   [src/main.rs:11] &ID = 3
   3
   [src/main.rs:11] &ID = 4
   4
   [src/main.rs:11] &ID = 5
   5
   [src/main.rs:11] &ID = 6
   6
   [src/main.rs:11] &ID = 7
   7
   [src/main.rs:11] &ID = 8
   8
   [src/main.rs:11] &ID = 9
   9
   ```

3. `fetch_add()` wraps around on overflow

   `std::sync::Arc` uses `AtomicUsize` internally as counters, how does it handle
   that overflow:

   ```rust
   // impl of `Arc::clone()`
   fn clone(&self) -> Arc<T> {
       let old_size = self.inner().strong.fetch_add(1, Relaxed);

       if old_size > MAX_REFCOUNT {
           abort();
       }

       unsafe { Self::from_inner(self.ptr) }
   }
   ```

   > Mara Bors says that: That (overflow) would take hundreds of years on a 
   > 64-bit computer, but is achieveable in seconds if isize is only 32 
   > bits. So I guess this behavior is fine.

# Compare-and-Exchange Operations

1. `compare_and_exchange()`
   
   > Compare first, if equal, exchange it with the `new` value.

   ```rust
   // Signature

   pub fn compare_exchange(
       &self,
       current: i32,
       new: i32,
       success: Ordering,
       failure: Ordering
   ) -> Result<i32, i32>
   ```

   This function will check the value to see if it is equal to `current`, if 
   so, `new` will be stored to it. All these procedures will be finished as
   an atomic operation.

   Returns the previous value on either error and success.

   Ignore the `Ordering` arguments, the implementation can be simplified as,
   expect that all the operations are executed as a single atomic op:

   ```rust
   impl AtomicI32 {
       pub fn compare_exchange(&self, current: i32, new: i32) -> Result<i32, i32> {
           // In reality, the load, comparison and store,
           // all happen as a single atomic operation.
           let v = self.load();
           if v == current {
               // Value equals to current.
               // Replace it and report success.
               self.store(new);
               Ok(v)
           } else {
               // Leave it untouched and report failure.
               Err(v)
           }
       }
   }
   ```
