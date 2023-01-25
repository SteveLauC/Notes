> In the second chapter we’ll learn about Rust’s atomic types and all their 
> operations. We start with simple load and store operations, and build our way
> up to more advanced compare-and-exchange loops, exploring each new concept 
> with several real-world use cases as usable examples.
>
> While memory ordering is relevant for every atomic operation, that topic is 
> left for the [next chapter](../Ch3:Memory-Ording.md). This chapter only 
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

# Atomic Load and Store Operations
