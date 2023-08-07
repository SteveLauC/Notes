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

# Latches Overview

1. Diff between Locks and Latches

   * Locks
     
     A lock is a higher-level, logical primitive that protects the contents of 
     a database (e.g., tuples, tables, databases) **from other transactions**. 
     **Transactions** will hold a lock for its entire duration. Database systems
     can expose to the user the locks that are being held as queries are run. 
     **Locks need to be able to rollback changes**.

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

   2. Reader Writer Lock

      > std::sync::RwLock

      * Pros

        1. Support concurrent readers. 

# Hash Table Latching

We use Linear Probe Hashing to demo the Hash Table Latching, because it 
is simple and it is easy to make a simple structure concurrent.

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


2. We resolve the above two problems (especially the last one) by Latch 
   Crabbing/Coupling

   The basic idea is, every time we traverse through a B+Tree:

   1. Get the latch for the parent node
   2. Get the latch for the child node
   3. Release all the latches accumulated along the path if the current node 
      is "safe"
      
      By "safe", we mean that the operation that this thread will do won't
      **effect parent nodes**:

      1. For a thread that tries to delete an item, the current node has at least
         "half" nodes + 1 so that if a coalescence will happen below it, it is
         "safe" to remove an item from the current node and finish the recursion.

         > The recursive coalescence and be ended in this node so that we don't
         > need to worry about the parent nodes, and thus can release the latches
         > on them.

      2. For a thread that tries to insert an item, the current node is not full,
         meaning that **if we will do a split below it, there is room to accommodate
         the node that will be inserted into this node**. 

         > The cecursive split can be finished in this node so that we don't
         > need to worry about the parent nodes, and thus can release the latches
         > on them.

      3. For a thread that tries to find an item, it will always be safe.

      > Pro tip on realeasing latches on the parent nodes: 
      >
      > Releasing the collected latchs **from top to bottom** would be more 
      > efficient as latches that are closer to the Root blocks a larger 
      > portion of nodes.

3. How the Latch Crabbing would solve the last issue:

   When thread t1 reaches node A, then acquire the latch on the node B, for 
   deletion, node B is not safe, we keep the latches and acquire the latch for
   node D, node D is safe and thus we release the latches for node A and B, then
   acquire the latch on node I, I is NOT safe, we keep the latches on node D and 
   I, then remove 44 from node I, t1 now gets suspended.

   Thread t2 reaches node B, which is safe for it, latches on node A and B are
   released, then it tries to acquire the latch on node D but can not make it
   since a write latch is held by suspended thread t1, issue solved!

   For thread t1, node D and its children nodes will possibly get modified by
   it, and thus it will hold the latches on these nodes for the whole operation.

4. Latch Crabbing does solve the issue, but every time we want to do some operations
   on a B+Tree, we always acquire a latch on the root node, for a insert/delete 
   operation, this latch will always be a write latch, this almost makes our 
   data structure single-threaded.

   A better algorithm (for insert/delete) is that: we just assume that path 
   from root node to the target leaf node is safe, and modifications won't 
   happen along the path, and thus we can always acquire a shared (read) 
   latch, and release it after we land on the child node. When reach the 
   target leaf node, acquire the write latch on the target leaf node, we 
   judge if we are safe, if we are safe, then do the things we need.

   If we are not safe, release all the latches (one or multiple read latches and one 
   write latch), abort our current opeartion and fall back to the "bad" 
   algorithm.

   > This better algorithm is more like a gamble. 

# Leaf Node Scans

1. All threads acquire latches in a top-down manner, which means there won't
   be deadlocks.

   But, with range scans, there could be deadlocks:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-05%2017-45-17.png)

   Tread 1 tries to delete(4) and thread 2 tries to find keys that are larger
   than 2, they reach the leaf nodes, holding corresponding latches, now thread
   2 tries to get a read latch on rightmost node, it cannot as a write latch 
   is held by thread 1, deadlock happens.

2. How do we solve deadlock issues:

   1. write good code
   2. Set a timer within the thread, when the timer buzzs, and it still cannot 
      make any progress, just kill itself.
