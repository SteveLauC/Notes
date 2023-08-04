> Today's agenda
>
> * Latches Overview
> * Hash Table Latching
> * B+Tree Latching
> * Leaf Node Scans

# Before we start

1. We want our data structure to be concurrent to hide I/O stalls. But redis,
   voltDB, they don't do these things.

   > Redis is single thread per process, voltDB can have multiple threads within
   > a single process, but they are all bound to differnt cores so that they 
   > won't touch the same query at the same time.


# Latches Overview

1. Diff between Locks and Latches

   * Locks
     
     A lock is a higher-level, logical primitive that protects the contents of 
     a database (e.g., tuples, tables, databases) **from other transactions**. 
     **Transactions** will hold a lock for its entire duration. Database systems
     can expose to the user the locks that are being held as queries are run. 
     **Locks need to be able to rollback changes**.

   * Latches

     > Basically this is what we call locks in a programming language, for 
     > example, `Mutex`, `RwLock`, `SpinLock`...

     Latches are the low-level protection primitives used for **critical sections**
     the DBMS’s internal data structures (e.g., data structure, regions of 
     memory) from other threads. Latches are held **for only the duration of the 
     operation being made**. **Latches do NOT need to be able to rollback changes**.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-04%2011-38-54.png)

2. Comparison between different latches

   1. Blocking OS mutex

      > std::sync::Mutex

      * Pros

        1. Easy to use

      * Cons

        1. Blocking
        2. Scheduled by OS, not the DBMS
        3. Non-scalable

           About 25ns per lock/unlock

   2. Reader Writer Lock

      > std::sync::RwLock

      * Pros

        1. Support concurrent readers. 

# Hash Table Latching

We use Linear Probe Hashing to demo the Hash Table Latching, because it 
is simple and it is easy to make a simple structure concurrent.

And, with Linear Probe Hashing, all threads will access the table in the 
same direction, deadlock is impossible to happen.

These are two modes you can use when implementing Hash Table Latching:

> Assumes it is on disk, slots are stored within the database page.

1. Page Latches

   One latch per page, this will reduce parallelism but takes less metadata.

2. Slot Latches

   One latch per slot (multiple latches per page), more parallelism but also 
   more metadata.

# B+Tree Latching

1. Two problems we need to deal with in B+Tree Latching:

   1. Modifications from multiplt threads at the same time

      > This is easy. 

   2. One thread traversing the tree while another thread splits/merges 
      nodes.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-04%2016-16-13.png)
      
      Let's give an example to demo this issue:

      With the above B+Tree, a thread t1 wants to delete(44), because it is the 
      first thread that accesses this B+Tree, it traverses down to the leaf node
      without any issue, it deletes 44 from the leaf node, then somehow, it gets
      suspended by the OS holding the write lock to the leaf node.

      At this time, the leaf node that just has 44 removed becomes empty, let's 
      assume that we should use `redistribution` instead of `coalescence` to handle
      this issue. But remeber that t1 hasn't done this yet.

      Then another thread t2 tries to find(41), it traverses down to the node `D`, 
      then it gets suspended as well...

      t1 is awoke now, it borrows `41` from node `H` to node `I`, at this time. 
      t2 is also awoke, it follows the pointer in node `D`, reaches node `H`, then
      it cannot find `41`, returns None.

      But 41 is actually in the tree. This is actually a "good" result, the 
      thread t1 can acually merge node `H` to node `I`, then thread t2 will 
      get a dangling pointer, it deref that pointer, bombing, segfault.


2. We resolve the above two problems (especially the last one) by Latch 
   Crabbing/Coupling

   

# Leaf Node Scans
