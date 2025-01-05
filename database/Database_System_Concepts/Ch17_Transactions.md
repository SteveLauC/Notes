> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Looks like this chapter is more like an introduction to tx
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>
>   * Let's see if my understanding to ACID is correct or not

> What have you learned from it
>
> * ACID should be rewritten as ADI-C, atomicity and durability are done by the
>   recovery system (WAL), and isolation is another important property, consistency
>   is a high-level, abstract property.
> * Isolation and serializable schedule


> TOC
>
> * 17.1 Transaction Concept
> * 17.2 A Simple Transaction Model
> * 17.3 Storage Structure
> * 17.4 Transaction Atomicity and Durability
> * 17.5 Transaction Isolation
> * 17.6 Serializability
> * 17.7 Transaction Isolation and Atomicity
>   * 17.7.1 Recoverable Schedules
>   * 17.7.2 Cascadeless Schedules
> * 17.8 Transaction Isolation Levels
> * 17.9 Implementation of Isolation Levels
> * 17.10 Transactions as SQL Statements
> * 17.11 Summary

1. Transaction: a collection of operations that form a single logical unit of work.

2. To quote the book: 

   "Chapter 17 describes the concept of a transaction in detail, including the 
   properties of atomicity, durability, isolation and other properties provided 
   by the transaction abstraction"

   This is interesting, it does not mention "Consistency"

# 17.1 Transaction Concept

1. If a transaction fails for whatever reason, any changes made to the database
   must be undone. The failure cause **is not necessarily** execution failure,
   it could be:

   * OS crash
   * Power loss
   * ...

2. What is "Consistency"

   To quote the book: "If a transaction is run atomically in isolation starting 
   from a consistent database, the database must again be consistent at the end 
   of the transaction."

   What does "consistent" mean? It is more like a property depending on the 
   application, for bank transfer from account A to B, consistent means that
   the sum of A and B won't change after the transaction.

   However, inconsistent state could occur during the transaction execution,
   which is expected. As long as the Atomicity property holds, this state will 
   eventually be consistent.


# 17.2 A Simple Transaction Model
# 17.3 Storage Structure

1. This section mentions "Stable Storage", a kind of storage that would "never"
   lost data.

   Hmm, kinda think S3 belongs to this.

# 17.4 Transaction Atomicity and Durability

1. For a write transaction, in WAL log entry, we log:

   1. The transaction identifier
   2. The tuple identifier
   3. Old and new tuple value

2. A transaction must be in one of the following states

   1. Active: the initial state, it is executing
   2. Partially committed: the last statement has been executed
   3. Failed
   4. Aborted: the tx has been rolled back
   5. Committed

   ![d](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-05%20at%201.39.10%E2%80%AFPM.png)

   For an aborted tx, the DBMS can either:

   1. restart it

      This could be desirable if the tx failure is not caused by developer's
      code but rather some hardware issue or software issue that is not related
      to developer's code.

   2. Just let it fail

# 17.5 Transaction Isolation & 17.6 Serializability

> QUES: I think the schedules listed these 2 sections execute tx concurrently, 
> not in parallel. I wonder what should we do with parallel tx?

1. Term

   1. Schedule: The order in which the DBMS executes operations is called an 
      execution schedule.
   2. Equivalent schedules: If 2 schedules, will result in the same database 
      state, we say they are equivalent
   3. Serial schedule: A schedule in which transactions are executed in sequence

      > If we have $ n $ transactions, then there will be $ n!$ serial schedules
      >
      > Note that different serial schedules can produce different results, but
      > they are all considered "correct".

   4. Serializable schedule: If a schedule is equivalent to a serial schedule,
      then it is a serializable schedule  
   5. Conflicting operations: 2 **consecutive** operations conflict with each 
      other if:

      1. They belong to different transactions
      2. They refer to the same data item 
      2. At least one of them is a write operation

      If 2 consecutive operations that:

      1. belong to different transactions 

         > We cannot swap the order of 2 operations that belong to the same tx.

      2. They do not conflict

      Then we can swap their order to produce a new schedule, which is equivalent
      to the original schedule.

   6. Conflict-equivalent schedules: We say 2 schedules are conflict equivalent
      if, by swapping non-conflicting operations, one schedule can be transformed
      into the other one.
   7. Conflict serializable schedule: A schedule that is conflict-equivalent to
       a serial schedule

2. How to determine if a schedule is conflict serializable?

   Draw a directed graph that contain vertexes and edges, where vertexes correspond
   to the transactions, for a vertex pair $(T_{i}, T_{j})$, if any of the following
   conditions hold, then there would be an edge from $T_{i}$ to $T_{j}$:

   For data item $Q$

   1. $T_{i}$ executes write($Q$) before $T_{j}$ executes read($Q$)
   2. $T_{i}$ executes read($Q$) before $T_{j}$ executes write($Q$)
   3. $T_{i}$ executes write($Q$) before $T_{j}$ executes write($Q$)

   Having an edge from $T_{i}$ to $T_{j}$ means that, if there are serial schedules
   that are conflict-equivalent to it, i.e., this schedule is conflict serializable, 
   then $T_{i}$ must be executed before $T_{j}$.

   > Explanation:
   >
   > No matter which above condition holds, it means that there are 2 conflicting
   > operations that one is before another (Say $T_{i}.op1$ is before $T_{j}.op2$). 
   > And conflicting operations' order cannot be swapped.
   >
   > Assume its conflict equivalent serial schedule exists, then in it, the order
   > of these 2 conflicting operations has to be guaranteed as well. Since it is 
   > a serial schedule, if $T_{i}.op1$ is before $T_{j}.op2$, then all the operations 
   > of $T_{i}$ has to be before $T_{j}$'s operations, which is $T_{i}$ has to be 
   > before $T_{j}$.

   If there is an edge from $T_{i}$ to $T_{j}$, and also an edge from $T_{j}$ to 
   $T_{i}$, then $T_{i}$ must be executed before $T_{j}$, and $T_{j}$ must be 
   executed before $T_{i}$, which is impossible, then we know that this schedule
   is not conflict serializable.

3. Even though a non-conflict serializable schedule can produce the same result
   as a serial schedule, e.g., the above schedule is not conflict serializable, 
   but it would produce the same result as serial schedule `T1, T5`, so it is 
   actually serializable but not conflict serializable:

   ![d](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-05%20at%209.03.25%E2%80%AFPM.png)

   Serial schedule `T1, T5` is equivalent to the above schedule, but they are not 
   conflict equivalent. 
   
   From this, we know that 
   
   1. conflict equivalence is more strict than equivalence.
   2. If 2 schedules are conflict equivalent, they are equivalent as well. 
      However, 2 equivalent schedules are not necessarily conflict equivalent.

      If a schedule is conflict serializable, it is serializable. A serializable
      schedules is not necessarily conflict serializable.

   This is because determining if 2 schedules are conflict equivalent ONLY needs
   to analyze read($Q$) and write($Q$) operations. Which is not sufficient to 
   infer if they are equivalent, to do so, we have to analyze all the computation 
   that these 2 schedules do. 
   
   We are being lazy to use a **more strict** definition, so that we could do 
   less analyzing work, as a result, we only know if a schedule is conflict 
   serializable, but ideally, we should know if it is serializable.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-05%20at%209.29.50%E2%80%AFPM.png)
   

# 17.7 Transaction Isolation and Atomicity
## 17.7.1 Recoverable Schedules
## 17.7.2 Cascadeless Schedules
# 17.8 Transaction Isolation Levels
# 17.9 Implementation of Isolation Levels
# 17.10 Transactions as SQL Statements
# 17.11 Summary