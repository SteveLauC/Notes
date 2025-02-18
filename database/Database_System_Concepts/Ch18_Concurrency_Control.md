> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Introduces some concurrency control implementation approaches:
>     1. 2pl
>     2. timestamp-based approach
>     3. MVCC 
>        1. Snapshot isolation
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading and recheck it after reading)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>   >
>   > 1. If you design the thing from scratch, how would you do this?
>   > 2. If several solutions/approaches/implementations are introduced for the 
>   >    same issue/problem, compare them and list their pros and cons.
>
>   1. Understand the idea of these approaches 
>   2. QUES: 2pl and snapshot isolation are the most commonly used approaches, why?
>   3. The pros and cons of these approaches
>   4. If you design the thing from scratch, how would you do this?
>
>      Using locks feels most feasible to me, and the snapshot isolation is also 
>      a natural approach but we need to handle conflicts.

> What have you learned from it
>
> *
> *


> TOC
>
> * 18.1 Lock-Based Protocols
>   * 18.1.1 Locks
>   * 18.1.2 Granting of Locks
>   * 18.1.3 The Two-Phase Locking Protocol
>   * 18.1.4 Implementation of Locking (lock manager)
>   * 18.1.5 Graph-Based Protocols
> * 18.2 Deadlock Handling
>   * 18.2.1 Deadlock Prevention
>   * 18.2.2 Deadlock Detection and Recovery
>     * 18.2.2.1 Deadlock Detection
>     * 18.2.2.2 Recovery from deadlock
> * 18.3 Multiple Granularity
> * 18.4 Insert Operations, Delete Operations, and Predicate Reads
> * 18.5 Timestamp-Based Protocols
> * 18.6 Validation-Based Protocols
> * 18.7 Multiversion Schemas
> * 18.8 Snapshot isolation
> * 18.9 Weak Levels of Consistency in Practice
> * 18.10 Advanced Topics in Concurrency Control
> * 18.11 Summary


# 18.1 Lock-Based Protocols
## 18.1.1 Locks

1. Deadlock is preferred over inconsistent state because deadlock can be handled 
   by the database system by rolling back the transactions.

2. Terms

   * Locking protocol: A set of rules that a transaction has to follow when 
     locking and unlocking a data item (acquiring and releasing a lock).
     
   * Precede: We say that a transaction Ti precedes Tj if:

     1. Ti acquires a lock in mode A on data item Q
     2. Tj holds a lock in mode B on data item Q later
     3. mode A and mode B are not compatible, one of them is an exclusive lock (write lock)

     > This is quite similar to how to determine if a schedule is conflict-serializable
     > using dependency graph (precedence graph).
    
   * Legal schedule: We say a schedule S is legal schedule under a locking protocol 
     if S is a *possible schedule* for its transactions that follow the locking protocol
     rules.

     > QUES: this concept is kinda confusing, I don't quite understand it.
     >
     > Am I right in say that: A schedule S is legal under a locking protocol if 
     > all the transactions of S follow the locking protocol rules.

   * A locking protocol *ensure*s conflict serializability if all its legal 
     schedules are conflict serializable.

## 18.1.2 Granting of Locks
## 18.1.3 The Two-Phase Locking Protocol

> Two-phase locking protocol is a *locking protocol*, i.e., a set of rules.
     
1. What is the Two-Phase Locking Protocol

   This locking protocol consists of 2 phases:

   1. Growing phase: transactions may obtain locks but may not release any locks
   2. Shrinking phase: transactions may release locks but may not obtain any locks

   Initially, a transaction is in the growing phase, it obtains locks as needed.
   Once it releases a lock, it enters the shrinking phase, and it can issue **no
   more** lock requests.

2. Two-phase locking protocol ensure conflict serializability

   QUES: figure out why
   
   The dependency graphs of the schedules that 2PL generates, have no cycles, 
   so it will ensure conflict serializable.

2. Two-phase locking protocol ensures conflict serializability, but a schedule that
   is legal under the two-phase locking protocol is not necessarily cascadeless.

   For example, the following partial schedule (partial because all 3 transactions
   do not contain the final commit/abort statements), T2 reads the A written by 
   T1, T3 reads the A written by T2, if T2 aborts, then T2 and T3 have to be aborted
   in a chained way.

   So two-phase locking protocol does not prevent dirty read.

   ```
   T1                  T2                  T3

   begin;
   Lock-exclusive(A);  
   read(A);
   write(A);
   unlock(A);          
                       Lock-exclusive(A);
                       read(A);
                       write(A);
                       unlock(A);          
                                            Lock-shared(A);
                                            read(A);
                                            unlock(A);
   ```

   There is a variant (called "strict 2pl") of Two-Phase locking protocol that 
   prevents dirty read, with a additional rule: all exclusive (write) locks 
   should not be released until the transaction ends (commit/abort), which 
   effectively prevents other transactions from reading uncommitted writes.
   
   By holding the exclusive lock until the transaction ends, strict 2PL does not
   only prevent dirty read, but also dirty write.

   Another variant of Two-Phase locking protocol is rigorous two-phase locking 
   protocol (or strong strict 2 PL), which is more strict! It requires the 
   transaction should not release all the locks until the it ends.

3. Lock conversion

   We allow lock conversion in two-phase locking protocol to maximize concurrency:

   * update: convert a shared lock to an exclusive lock

     > Happen in only the growing phase
     >
     > QUES: why

   * downgrade: convert an exclusive lock to a shared lock

     > Happen in only the shrinking phase
     >
     > QUES: why

## 18.1.4 Implementation of Locking (lock manager)

1. If a transaction aborts, then the lock manager removes all the waiting
   request made by it. And, for the granted locks, the lock manager only
   removes their records after the undo process.

## 18.1.5 Graph-Based Protocols

This section introduces other locking protocols, which requires us to know the 
order of how data items will be accessed, which is impossible with interactive
transaction.

So I skipped this section.

# 18.2 Deadlock Handling

1. There are generally 2 ways to handle deadlock:

   1. Deadlock prevention (pessimistic?)
    

   2. Deadlock detection and recovery (optimistic?)


   We will see that both approaches can have deadlock

2. Which approach is preferred in certain cases, why?

   If the probability of deadlock is relatively high, deadlock prevention is 
   preferred. Otherwise, deadlock detection and recovery is more efficient.


   > QUES: figure out why
   >
   > Guess: Deadlock detection and recovery has runtime cost, if the deadlock
   > possibility is high, then this cost can be dominating. Deadlock prevention
   > is more like a static rule (no runtime cost), but if the possibility is 
   > low, then it will harm concurrency?


## 18.2.1 Deadlock Prevention

1. There are 3 kinds of deadlock prevention

   1. Only allow lock acquisition in an ordered way to prevent cyclic acquisition

      Deadlock only happens if a cyclic acquisition occurs:

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-19%20at%2010.45.30%E2%80%AFAM.png)

      The above example is a deadlock. If we require that locking data items has
      to be done in the order of data items, i.e., should lock data item 1 first
      then data item 2, since for the same data item, T1 and T2 require incompatible
      locks on them, either T1 or T2 will acquire the lock on data item 1, the other
      transaction has to wait.

      QUES: How can we implement this for interactive transaction?

      ```sql
      T1                                                    T2

      begin;                                                begin;
      update table set int = int + 1 where id = 0;          update table set int = int + 1 where id = 1;
      update table set int = int + 1 where id = 1;          update table set int = int + 1 where id = 0;
      commit;                                               commit;
      ```

      For T2, after the user typing `update table set int = int + 1 where id = 1`,
      obviously we could not lock the date item whose id is 1, but should we wait?

      The textbook mentions a variant of this approach: once a transaction has 
      locked an item, then it cannot request locks on items that precede that
      item. If we implement this, then T2 in the above example would be rejected.
      
   2. Rollback a transaction to prevent cyclic wait

      When a transaction T1 tries to lock an item locked by another transaction 
      T2, T1 could:

      1. roll back itself
      2. roll back T2
      3. wait

      We introduce 2 approaches here, they choose different above measures.

      Assign each transaction a unique timestamp, T1 should:

      1. wait-die: T1 is allowed to wait (wait) if it has a timestamp smaller than that 
         of T2. Otherwise, **T1** gets rolled back (die).

         > This approach chooses `<roll back itself>` and `wait`

         > T2 won't be affected in this approach
         >
         > This approach is non-preempted

      2. wound-wait: T1 is allowed to wait if it has a timestamp larger than that
         of T2. Otherwise, **T2** (NOTE, it is T2) gets rolled back.

         In other words, if T1 has a smaller timestamp, then it would (rollback)
         T2. Otherwise, T1 waits (wait).

         > This approach chooses `<roll back T2>` and `wait`

         > T2 can be rolled back, this approach is preempted

      Both approaches prefer transactions with smaller timestamp (older transaction), 
      keep them alive, younger transaction will be rolled back. (I think they
      assume rolling back an older transaction is more costly, needs to undo more stuff)
      In wait-die, older transaction is allowed to wait. In would-wait, older 
      transaction will wound the other transaction and roll it back so that 
      it can acquire the lock.

      The transaction that is rolled back will be restarted, using the same 
      timestamp so that ultimately, it will become the oldest transaction, i.e.,
      it is guaranteed to acquire the lock with retry.

      Approach naming: `<what-will-happen-to-it-if-it-is-older, what-will-happen-to-it-if-it-is-younger>`

      How this prevents deadlock? Both approaches ensure that when 2 transaction
      try to acquire the lock held by the other transaction, one transaction will
      be rolled back, so no cyclic wait could happen, no deadlock.

      Which approach should I use?

   3. Wait, but only for a period of time

      The third approach allows transaction to wait for the lock acquisition, but
      only for a specific period of time, once timeout reaches, it aborts itself.
      (This can be seen as a variant of `wait-die`)

      Generally, it is hard to choose the right timeout value. Too large timeout will
      lead to unnecessary delay, too small timeout lead to unnecessary rollback.

      > Same hard as picking a timeout to decide if a node is down in a distributed
      > cluster.

## 18.2.2 Deadlock Detection and Recovery
### 18.2.2.1 Deadlock Detection

To detect deadlock, we first need to know some transaction waiting information, 
which can be collected by the lock manager when receiving lock grant requests. 
This information is described using a directed graph, where the vertices 
represent transactions, edges represent wait-for relationship, an edge from Transaction Ti to Transaction 
Tj means that Ti is waiting for a data item locked by Tj. 

Cycle in the graph means deadlock, the involved transactions are deadlocked. So
to detect the deadlock, we scan the wait-for graph and look for cycles.

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-20%20at%2010.28.23%E2%80%AFPM.png)

The above diagram contains a deadlock, cycle `T18 -> T20 -> T19` means these 3
transactions are deadlocked.

How often should we run deadlock detection? If deadlock occur frequently, then 
the detection can be invoked more frequently. The worst case, we could do the
detection upon every lock allocation request.

### 18.2.2.2 Recovery from deadlock

Once we find a deadlock, we need to recover from it, i.e., break the deadlock,
break the wait-for graph cycle, by rolling back a deadlocked transaction in the 
wait-for cycle.

Three actions need to be taken:

1. More than 1 transaction is deadlocked, which one should we rollback

   Rolling back transactions means undo, which has cost. After the rollback, wen
   need to restart it, which also has cost. We want to pick the transaction with 
   the minimum cost. However, cost eatimation is hard.

2. Rollback the transaction

   * Roll back the whole transaction
   
     This is easy to implement, but has higher cost.
   
   * Partial Rollback
   
     It is more efficitive to do the partial rollback, i.e., only roll back the
     part that cause the deadlock. To do this, the **deadlock detection mechanism**
     should tell us the locks that the selected transaction needs to release in
     order to break the deadlock.

3. Avoid starvation

   It is possible that restarted transaction will run into deadlock and be selected
   as the transaction to rollback again. We need to ensure it won't be startved, i.e.,
   will always run into deadlock and be rolled back.
   
   The simplest solution is to consider the number of rollback when choosing
   the transaction to roll back.


# 18.3 Multiple Granularity

1. In the previous sections, we were assuming that locks are performed on data
   items, i.e., tuples. However, if we want to lock a table, then it is unlikely
   that this can be implemented by locking tuples because:
   
   1. If we lock a table by locking all its tuples, when tuples get inserted/deleted,
      we need to acquire/releae locks.
      
      > QUES: When you lock a table, is insertion/deletion allowed?
      >
      > Will this be answered in section 18.4?
      
   2. A table may not fit in memory so that locking them is impossible if we put
      latches (the locks in the programming language) on them.
   3. Locking many tuples is costly
   
   What we want instead is a single lock call that locks the table. To do this, we
   need multi-granularity locking mechanisms.
   
2. With this multi-granularity locking mechanism, the whole hierarchy is a tree,
   every node in the tree can be individually locked (in either shared or 
   exclusive mode, same as before). 
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/multi-granularity-lock-from-2025-02-08-11-29.png)
   
   Locking a node means *implicitly* locks all its descendants in the same lock 
   mode. If a transaction wants to lock a node that is **implicitly** locked, how
   can this transaction knows if this lock will be granted or not (since the node
   it tries to lock is implicitly locked)? 
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-02-08%20at%205.55.23%E2%80%AFPM.png)
   
   It will traverse the hierarchy tree from root to the node it tries to lock, 
   attempting to lock every node it traverses, if a lock cannot be acquired, 
   then it cannot lock that node.
   
   > NOTE: 
   >
   > "attempting to lock the node it traverses along the way"
   >
   > If this tx wants to acquires a S lock on the target node, it won't acquire
   > S locks on the nodes it traverses, instead, it will acquire IS locks, which
   > we will cover later.
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-02-08%20at%205.59.50%E2%80%AFPM.png)
   
   Since locking a node implicitly locks all its decendants, when a transaction
   tries to lock a node in mode `M` (locks the whole sub-tree in mode `M`), if
   any node in sub-tree was already locked in a mode that is incompatible with
   `M`, then this
   
   TODO: finish this
   
   
   -----
   
   If you want to lock a node, then you need to ensure:
   
   1. It is not locked in incompatible mode (obviously)
   2. All of its ancestors are not locked in incompatible mode (since locking a 
      node means locking its descendants)
   3. Non of its descendants are locked in incompatible mode (since locking a
      node means locking its descendants, which cannot be done if any descendants
      is locked in incompatible mode)
      
   If we want to lock a node, how can I know if the requirements are satisfied? 
   For requirement 1 and 2, it is easy to verify, we can traverse the tree to
   the target node and check every node traversed. For requirement 3, how can we
   verify it? 
   
   One possible way is to search all the descendants, which is costly. We introduce
   a new type of locking modes call *intention locking modes*. When you lock a node,
   all its ancestors should be locked in this mode.
   
   > Explain what "intention" means here
   > 
   > When 
   
   There are 3 *intention locking modes*:
   
   1. intention-shared mode
   
      Indicates explicit locking at a lower level with shared locks.
      
   2. intention-exclusive mode
   
      Indicates explicit locking at a lower level with exclusive or shared locks.
   
   3. shared and intention-exclusive mode
   
      The *sub-tree* rooted at that node is locked explicitly in shared mode and 
      explicit locking is **being** done at a lower level with exclusive-mode locks.
      
      > QUES: 
      >
      > 1. What is this?
      > 2. When will it be used?
   

# 18.4 Insert Operations, Delete Operations, and Predicate Reads
# 18.5 Timestamp-Based Protocols
# 18.6 Validation-Based Protocols
# 18.7 Multiversion Schemas
# 18.8 Snapshot isolation
# 18.9 Weak Levels of Consistency in Practice
# 18.10 Advanced Topics in Concurrency Control
# 18.11 Summary
