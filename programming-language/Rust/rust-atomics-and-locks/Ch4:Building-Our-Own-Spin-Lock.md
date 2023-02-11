> After learning the theory, we put it to practice in the next three chapters 
> by building our own versions of several common concurrency primitives. The 
> first of these chapters is a short one, in which we implement a `spin lock`.
>
> We’ll start with a very minimal version to put `release` and `acquire` memory 
> ordering to practice, and then we’ll explore Rust’s concept of safety to 
> turn it into an ergonomic and hard-to-misuse Rust data type.

> # TOC
>
> * [Minimal Implementation](minimal-implementation)
> * [An Unsafe Spin Lock](an-unsafe-spin-lock)
> * [A Safe Interface Using a Lock Guard](a-safe-interface-using-a-lock-guard)
> * [Summary](summary)

1. What is `SpinLock`?

   A mutex will suspend a thread when the data protected by that mutex is currently
   not available, a spinlock won't block it but repeatedly try to acquire that lock
   (spinning).

   > This can **waste processor cycles**, but can sometimes result in **lower 
   > latency** when locking.

   > Many real-world implementations of mutexes, including `std::sync::Mutex`
   > on some platforms, briefly behave like a `spin lock` before asking the 
   > operating system to put a thread to sleep. This is an attempt to combine 
   > the best of both worlds, although it depends entirely on the specific 
   > use case whether this behavior is beneficial or not.

# A Minimal Implementation
# An Unsafe Spin Lock
# A Safe Interface Using a Lock Guard
# Summary
