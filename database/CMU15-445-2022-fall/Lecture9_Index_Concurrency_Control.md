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
   > won't touch the same query at the same time (violated)

2. Concurrency control protocol

   > Protocol seems to be a quite common term.
   
   A concurrency control protocol is the method that the DBMS uses to ensure
   "correct" result for concurrent operations on a shared object.

   A protocol's correctness criteria can vary:

   * Logical correctness: Can a thread see the data that it is supposed to see?

     This is a high level concept, like transaction violation.
     
   * Physical correctness: Is the internal representation of the object sound?

     For example, we won't deref a dangling pointer. This would make sure that 
     we won't crash.

# Latches Overview

1. Diff between Locks and Latches

   * Locks
     
     A lock is a higher-level, logical primitive that protects the contents of 
     a database (e.g., tuples, tables, databases) **from other transactions**. 
     **Transactions** will hold a lock for its entire duration. Database systems
     can expose to the user the locks that are being held as queries are run. 
     **Locks need to be able to rollback changes**.

     > We will cover this in Ch15: Concurrency Control Theory

   * Latches

     > Basically this is what we call lock in a programming language, for 
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

           > It is roughly 500ns on my host to lock a Rust std::sync::Mutex

   2. Reader Writer Lock

      > std::sync::RwLock

      * Pros

        1. Support concurrent readers. 

   > `RwLock` can be slower than `Mutex` as the implementation is more complicated,
   > per the Java doc:
   >
   > Whether or not a read-write lock will improve performance over the use of 
   > a mutual exclusive lock depends on:
   >
   > * The frequency that the data is read compared to being modified
   > * The duration of the read and write operations
   > * The contention for the data, i.e., the number of threads that will try to
   >   read or write the data at the same time
   >
   > Some general idea:
   >
   > * If write is rare, use `RwLock`
   > * If the read operations are short that it cannot beat the overhead of
   >   maintaining the state of `RwLock`, then use `Mutex`
   > * Always profile before making a conclusion

   > On Linux, how is the Mutex implemented
   >
   > * Before 2.6, Linux have 
   >   1. [LinuxThreads](https://en.wikipedia.org/wiki/LinuxThreads)
   >   2. NGPT 
   >
   > * In Linux 2.6, Linux has a syscall (futex(2)) to enable developers to 
   >   implement userspace Mutexes. Then [NPTL](https://en.wikipedia.org/wiki/Native_POSIX_Thread_Library)
   >   comes to the world (which uses futex).
   >
   > The Mutex in the Rust std, builds its own Mutex using `futex(2)`
   >
   > How `futex(2)` generally work?
   > 
   > There is a lock in userspace, when you try to acquire it, if you did it, you 
   > are done. If you didn't, then you enter the kernel space.

# Hash Table Latching

We use Linear Probe Hashing (fix-sized array) to demo the Hash Table Latching,
because it is simple and it is easy to make a simple structure concurrent.

And, with Linear Probe Hashing, **all threads will access the table in the 
same direction, deadlock is impossible to happen**.

These are two modes you can use when implementing Hash Table Latching:

> Assumes it is on disk, slots are stored within the database page.

1. Page Latches

   One latch per page, this will reduce parallelism but takes less metadata.

   > Takes less metadata as latches are implemented and stored on disk.
   >
   > How?

2. Slot Latches

   One latch per slot (multiple latches per page), more parallelism but also 
   more metadata.

# B+Tree Latching

1. Two problems we need to deal with in B+Tree Latching:

   1. Modifications from multiple threads at the same time

      > This is easy cause write latches inheriently prohibits this.

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

      > The above case happens if a thread that tries to update the tree only
      > holds the write to the target leaf node.

2. We resolve the above two problems (especially the last one) by Latch 
   Crabbing/Coupling

   The basic idea is, every time we traverse through a B+Tree:

   1. Get the latch for the parent node
   2. Get the latch for the child node
   3. Release the latches on its ancestors if the parent node is "safe"

      > A safe node is one that won't split or merge.
      
      By "safe", we mean that the operation that this thread will do won't
      **affect/modify the parent node**:

      1. For a thread that tries to delete an item, if the current node is more
         than half full, then the parent node is safe.

         If it is more than half full, even though you need to delete an item 
         from it (By 1. directly removing an item from it 2. coalescence made
         by childre nodes), it won't need a coalescence so that the parent node
         won't be touched and modified.

      2. For a thread that tries to insert an item, if the current node is not 
         full, the parent node is safe.

         Even though we will insert an item to the current node, it won't be 
         splitted so that the parent node won't be touched and modified.

      3. For a thread that tries to find an item, it will always be safe.

      > Pro tip on realeasing latches on the parent nodes: 
      >
      > Releasing the collected latchs **from top to bottom** would be more 
      > efficient as latches that are closer to the Root block a larger 
      > portion of nodes.
  
   > By this approach, after traversing to the target leaf node (we only konw
   > if it is really safe until we reach the leaf, the origin of all changes), 
   > for insert and delete, we ONLY hold the write locks to the nodes that 
   > we will really modify.
   >
   > Beforing reaching the leaf node, we are holding lock **defensively**.

3. How the Latch Crabbing would solve the last issue:

   When thread t1 reaches node A, then acquire the latch on the node B, for 
   deletion, node B is not safe, we keep the latches and acquire the latch for
   node D, node D is safe and thus we release the latches for node A and B, then
   acquire the latch on node I, I is NOT safe, we keep the latches on node D and 
   I, then remove 44 from node I, t1 now gets suspended.

   Thread t2 reaches node B, reads are always safe, latches on node A and B are
   released, then it tries to acquire the latch on node D but can not make it
   since a write latch is held by suspended thread t1, issue solved!

   For thread t1, node D and its children nodes will possibly get modified by
   it, and thus it will hold the latches on these nodes for the whole operation.

4. Latch Crabbing does solve the issue, but every time we want to do some operations
   on a B+Tree, we always acquire a latch on the root node, for a insert/delete 
   operation, this latch will always be a write latch, this almost makes our 
   data structure single-threaded.

   A better algorithm (for insert/delete) is that: we just assume that the
   modification will **ONLY happen in the target leaf node, it won't propagate
   at all**. With this, we can acquire read locks from root to the parent node
   of the target leaf node, then acquire the write lock to the leaf node.

   If our assumption is correct, then we can release the read latches and do 
   the modification. If the assumption is wrong, then we release all the latches
   (one or multiple read latches and one write latch), abort the current operation
   and fall back to latch crabbing.

   > This better algorithm is more like a gamble. 

# Leaf Node Scans

1. All threads acquire latches in a top-down manner, which means there won't
   be deadlocks.

   But, with range scans, things can be bad:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-05%2017-45-17.png)

   Thread 1 tries to delete(4) and thread 2 tries to find keys that are larger
   than 2, they reach the leaf nodes, holding corresponding latches, now thread
   2 tries to get a read latch on rightmost node, it cannot as a write latch 
   is held by thread 1.

   This case, technically, is not a deadlock, but the thing is that thread 2
   doesn't know if its gets a deadlock or not. To handle this, we can maintain
   a lock table, then if the system finds that thread 2 is waiting for too long,
   just kill thread 2 and redo the operation.

2. How do we solve deadlock issues:

   1. Write good code
   2. Set a timer within the thread, when the timer buzzs, and it still cannot 
      make any progress, just kill itself.
