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
>   * [Example: Process Reporting](#example-process-reporting)
>     * [Synchronization](#synchronization)
>   * [Example: Lazy Initialization](#example-lazy-initialization)
> * [Fetch-and-Modify Operations](#fetch-and-modify-operations)
>   * [Example: Progress Reporting from Multiple Threads](#example-progress-reporting-from-multiple-threads)
>   * [Example: Statistics](#example-statistics)
>   * [Example: ID Allocation](#example-id-allocation)
> * [Compare-and-Exchange Operations](#compare-and-exchange-operations)
>   * [Example: ID Allocation Without Overflow](#example-id-allocation-without-overflow)
>   * [Example: Lazy One-Time Initialization](#example-lazy-one-time-initialization)
> * [Summary](#summary)

1. Atomic operation is an operation that is indivisible, which means it is either
completed, or it did't happen yet.

2. Data races will never happen with atomic types as their operations are atomic.

   For `Mutex` and `RwLock`, data races won't happen because it blocks the 
   threads that try to access the locked variable.

2. Atomic operations are the **main building block** for anything involving 
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
