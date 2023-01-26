> After learning about the various atomic operations and how to use them, the 
> third chapter introduces the **most complicated topic of the book**: memory 
> ordering.
> 
> We’ll explore how the memory model works, what happens-before relationships 
> are and how to create them, what all the different memory orderings mean, 
> and why sequentially consistent ordering might not be the answer to everything.
> And you will know which one to use when need them.

> Memory Ordering in Rust:
>
> ```rust
> pub enum Ordering {
>     Relaxed,
>     Release,
>     Acquire,
>     AcqRel,
>     SeqCst,
> }
> ```

> TOC
>
> * [Reordering and Optimizations](#reordering-and-optimizations)
> * [The Memory Model](#the-memory-model)
> * [Happens-Before Relationship](#happens-before-relationship)
>   * [Spawning and Joining](#spawning-and-joining)
> * [Relaxed Ordering](#relaxed-ordering)
> * [Release and Acquire Ordering](#release-and-acquire-ordering)
>   * [Example: Locking](#example-locking)
>   * [Example: Lazy Initialization with Indirection](#example-lazy-initialization-with-indirection)
> * [Consume Ordering(Not available in Rust but is present in C++)](#consume-ordering)
> * [Sequentially Consistent Ordering](#sequentially-consistent-ordering)
> * [Fences](#fences)
> * [Common Misconceptions](#common-misconceptions)
> * [Summary](#summary)

# Reordering and Optimizations

1. Processors and compilers perform all sorts of tricks to make your programs 
   run as fast as possible. A processor might determine that two particular 
   consecutive instructions in your program will not affect each other, and 
   execute them out of order, if that is faster. Similarly, a compiler might 
   decide to reorder or rewrite parts of your program if it has reason to 
   believe it might result in faster execution. But, again, only if that wouldn’t
   change the behavior of your program.

   Take a look at this example:

   ```rust
   fn f(a: &mut i32, b: &mut i32) {
       *a += 1;
       *b += 1;
       *a += 1;
   }
   ```

   Here, the compiler will most certainly understand that the order of these 
   operations does not matter, since nothing happens between these three 
   addition operations that depends on the value of *a or *b. (Assuming overflow
   checking is disabled.) Because of that, it might reorder the second and 
   third operations, and then merge the first two into a single addition:

   ```rust
   fn f(a: &mut i32, b: &mut i32) {
       *a += 2;
       *b += 1;
   }
   ```

   Later, while executing this function of the optimized compiled program, 
   a processor might for a variety of reasons end up executing the second 
   addition before the first addition, possibly because *b was available 
   in a cache, while *a had to be fetched from the main memory.

   Regardless of these optimizations, the result stays the same: `*a` is 
   incremented by two and `*b` is incremented by one. The order in which 
   they were incremented is entirely invisible to the rest of your program.

   The logic for verifying that a specific reordering or other optimization 
   won’t affect the behavior of your program **does not take other threads 
   into account**. In our example above, that’s perfectly fine, as the 
   unique references (&mut i32) guarantee that nothing else can possibly 
   access the values, making other threads irrelevant. **The ONLY situation 
   where this is a problem is when mutating data that’s shared between threads.
   Or, in other words, when working with atomics.** This is why we have to 
   **explicitly** tell the compiler and processor what they can and can’t 
   do with our atomic operations, since **their usual logic ignores interactions 
   between threads** and **might allow for optimizations that do change the result 
   of your program**.

   We tell compiler and processor which kind of optimization is allowed through
   memory ordering.

   ```rust
   pub enum Ordering {
       // Relaxed Ordering
       Relaxed,


       // Release and Acquire Ordering (3 variants)
       Release,
       Acquire,
       AcqRel,


       // Sequentially consistent Ordering
       SeqCst,
   }
   ```
   The set of available options (only 5 variants) is very limited, but has been 
   carefully picked to fit most use cases well. 

# The Memory Model
 
