> This chapter is mainly about:
>
> * How SQLite implements ACID properities
>   
>   * A (Atomicity): each statement in a transaction (to read, write, update or 
>     delete data) is treated as a single unit. Either the entire statement is 
>     executed, or none of it is executed. This property prevents data loss and 
>     corruption from occurring if, for example, if your streaming data source 
>     fails mid-stream.
>
>   * C (Consistency): ensures that transactions only make changes to tables in 
>     predefined, predictable ways. Transactional consistency ensures that corruption 
>     or errors in your data do not create unintended consequences for the 
>     integrity of your table.
>
>     > QUES: I don't think I really understand what does this C mean
>
>   * I (Isolation): when multiple users are reading and writing from the same 
>     table all at once, isolation of their transactions ensures that the concurrent
>     transactions don't interfere with or affect one another. Each request can occur 
>     as though they were occurring one by one, even though they're actually occurring 
>     simultaneously.
>
>   * D (Durability): ensures that changes to your data made by successfully executed
>     transactions will be saved, even in the event of system failure.
>
> * The SQLite way of managing various locks and their mappings to native file 
>   locks and lock transactions
>
> * How SQLite avoids deadlocks
> * How SQLite implements journal protocols
>
>   > QUES: what are journaling protocols, it does not seem to be a general 
>   > database term
>
> * How SQLite manages savepoints in user transactions
>
>   > A `SAVEPOINT` is a point in a transaction in which you can roll the transaction
>   > back to a certain point **without rolling back the entire transaction**. 

> Questions I have:
>
> 1. Relationship between transactions and the underlying lock states
>
>    "4.2.2 Lock acquisition protocol" talks about this.
>
> 2. The root cause of a transaction conflict is the conflict of the underlying 
>    lock, right?

> * 4.1 Transaction Types
>   * 4.1.1 System transaction (*autocommit* mode)
>   * 4.1.2 User transaction (*manual* mode)
>   * 4.1.3 Savepoint
>   * 4.1.4 Statement subtransaction
> * 4.2 Lock Management
>
>   > There are some notes on transaction isolation in this section.
>
>   * 4.2.1 Lock types and their compatibilities
>   * 4.2.2 Lock acquisition protocol
>   * 4.2.3 Explicit locking
>   * 4.2.4 Deadlock and starvation
>   * 4.2.5 Linux lock primitives
>   * 4.2.6 SQLite lock implementation
>     * 4.2.6.1 Translation from SQLite locks to native file locks
>     * 4.2.6.2 Exgineering issues with native locks
>     * 4.2.6.3 Linux system issues
>     * 4.2.6.4 Multithreaded applications
>   * 4.2.7 Lock APIs
>     * 4.2.7.1 The `sqlite30sLock` API
>     * 4.2.7.2 The `sqlite30sUnlock` API
> * 4.3 Journal Management
>   * 4.3.1 Logging protocol
>   * 4.3.2 Commit protocol
> * 4.4 Subtransaction Management

# 4.1 Transaction Types
## 4.1.1 System transaction

1. When in *autocommit* mode, those transactions are called system or auto or 
   implicit transactions

2. Under the *autocommit* mode, SQLite will create a read-transaction for a 
   `SELECT` statement. For a non-`SELECT` statement, SQLite first creates a
   read-transaction, and then converts it into a write-transaction when it
   **actually modifies** the database file.

   > Why would SQLite do this for non-`SELECT` statement, my thought would be
   > for maximizing concurrency as multiple read-transactions can occur at
   > the same time.

   > Under the manual mode, SQLite would also do something similar.

3. Simultaneous Read and Write transactions

   Under the rollback journal mode, since a write-transaction would directly write
   to the database file and store the original pages in a rollback journal, it is
   not possible to have read and write transactions at the same time.

   However, under WAL mode, write-transaction won't modify the database file, but
   write the changes in a WAL file, and ONLY commit these changes to the database
   file when the transaction commits, under such scenario, you can have read and
   write transactions at the same time.

   > But you can ONLY have one write-transaction at the same time
   >
   > QUES: Is this limited by the number of WAL files?

## 4.1.2 User transaction

1. System transaction is inefficient, especially for those write-heavy applications,
   because for every non-select statement, it reopens, writes to and closes the 
   journal file.

   > For a write-transaction on a single database, a rollback journal file would
   > be created to implement atomicity.

   And there is also a locking overehead.

2. The application creates a user transaction by:

   ```SQL
   BEGIN <transaction>

   COMMIT; (or ROLLBACK)
   ```

   > I would call this *manual mode* rather than a user transaction
   >
   > Future steve: well, it kinda makes sense to me now...

3. When a user transaction aborts, the write-transaction is rolled back and **some
   of read-transactions that read tables updated by the write-transaction** are 
   aborted too.

## 4.1.3 Savepoint

1. A savepoint is a point in a user transaction established by the application 
   so that application can roll back to this point instead of the start of the 
   whole transaction.

   One can establish multiple savepoints within a transaction.
   
2. Savepoint is supported in SQLite, an application can execute a `savepoint`
   command inside or **outside** a user transaction.

   > QUES: Currently I don't understand how a savepoint outside a transaction 
   > works, per the [document](https://www.sqlite.org/lang_savepoint.html), it 
   > seems to work like a `BEGIN DEFERRED TRANSACTION`(Ok, what is this then?).
   >
   > Not an answer to the question:
   > `BEGIN DEFERRED TRANSACTION` won't create any transaction until the actual
   > SQL statements are passed, see the `Explicit locking` section for more.

3. We are pretty clear that SQLite does not support nested transactions, but
   the document says that you can emulate this using savepoints. 

## 4.1.4 Statement subtransaction

1. Each non-`SELECT` statement in the transaction is executed in a separate 
   statement level *sub-transaction*.

   At any point of time, there can be at most one subtransaction in the user
   transaction.

   > Remeber the statement journal we have introduced in Ch3, it is just for
   > this.
   >
   > Statement journal is for statement abort, not for rollback.

# 4.2 Lock Management

1. What is transaction isolation?

   Isolation determines how transaction integrity is **visible to other users 
   and systems**

2. Transaction isolation phenomena/problems:

   * Dirty Read

     A read transaction can see uncommitted changes made by another 
     write-transaction

   * Non-repeatable read
     
     A read-transaction reads **a row** twice and it gets **different results**.

     > This is specifically related to the `UPDATE` statement.

   * Phantom Read

     A transaction **re-executes** a query returning a set of rows that satisfy 
     a search condition and finds that the set of rows satisfying the condition
     has changed when compared to the last run due to another recently committed
     transaction.

     > This is specifically related to the `INSERT` or `DELETE` statement, i.e.,
     > the number of rows has been changed, there are new rows or some row that
     > are deleted.

   > Here is a question from SO: 
   > [What is the difference between Non-Repeatable Read and Phantom Read?](https://stackoverflow.com/q/11043712/14092446)

3. There are four levels of transaction isolation, from lowest to highest:

   > This is defined by the SQL standard.

   > Apart from the first one, all the three levels are defined according to
   > a isolation phenomenon. And under the serializable level, none of these
   > phenomena will happen.

   * Read uncommitted
     
     A read-transaction can read data that are not committed by a write-transction,
     transactions are **NOT isolated at all**.

   * Read committed

     This isolation level guarantees that any data read is committed at the 
     moment it is read. Thus it does not allow dirty read.
    
   * Repeatable read

     This is the most restrictive isolation level. The transaction holds read 
     locks on all rows it references and writes locks on referenced rows for 
     update and delete actions. Since other transactions cannot read, update 
     or delete these rows, consequently it avoids non-repeatable read.

     > QUES: 

   * Serializable

     This is the highest isolation level. A serializable execution is guaranteed
     to be serializable. Serializable execution is defined to be an execution of 
     operations in which **concurrently executing transactions appears to be 
     SERIALLY executing**.

     > 并发，但看上去是串行。很好实现，只要真的把并发干掉就好了:<

   Serializable is the strictest isolation level, SQLite implements it >:

4. Relationships between phenomenon and isolication levels

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-27%2013-55-15.png)

   > From the [documentation](https://www.postgresql.org/docs/current/transaction-iso.html)
   > of PostgreSQL.

5. In SQLite, Serializable is implemented using locks at the **database level**.

   > And it uses two-phase locking (2PL) protocol.
   >
   > No idea what 2PL is...

6. For the underlying file system locks, SQLite uses 
   [POSIX record locks](https://gavv.net/articles/file-locks/#posix-record-locks-fcntl).

## 4.2.1 Lock types and their compatibilities

1. From the view of a single transaction, a database file can be in one of the 
   following **five** locking states:

   > SQLite has 5 lock states, but ONLY 4 types of locks. The underlying file 
   > system lock, only supports 2 types of locks:
   >
   > 1. read (shared)
   > 2. write (exclusive)

   1. NOLOCK

      No locks are hold by this transaction

   2. SHARED

      This transaction holds a shared (read) lock on the database file, this 
      lock ONLY permits reading from the database file.
   
   3. EXCLUSIVE

      This transaction holds a exclusive (write) lock on the database file, this
      lock allows both read and write.

   4. RESERVED

      This lock permits reading from the file, but it is a little **more than a
      shared lock**.

      A reserved lock informs other transactions that this transaction (the lock 
      holder) is planning to **write** to the database file in the near future,
      **but it is now just reading the file**.

      There can be at most 1 reserved lock on the file.

      Reserved lock can coexist with shared locks, and other transactions are 
      **allowed to acquire new shared locks**.

   5. PENDING

      This lock permits reading from the file.

      A pending lock means that the transaction **will write to the database 
      file immediately**. This transaction is just waiting on all current shared
      locks to release them.

      There can be at most 1 pending lock on the file.

      It can coexist with shared locks, but other transaction are NOT allowed to
      acquire shared locks.

2. Compatibility between different locks in SQLite

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-27%2016-05-58.png)

## 4.2.2 Lock acquisition protocol

> Lock acquisition protocol basically means how different transactions acquire
> locks.

1. The transaction lock management is done in the Pager module.

2. For a read-transaction, it acquires:

   1. shared lock

   > State change:
   >
   > ```
   > nolock -> shared lock -> nolock
   > ```

3. For a write-transaction, it acquires:
   
   1. Shared lock

      When the `BEGIN TRANSACTION` starts.

   2. Reserved Lock

      We say that this transaction now becomes a semi-write-transaction, shard 
      locks are allowed, but

      * reserved lock
      * pending lock
      * exclusive lock

      are NOT allowed.

      This semi-write-transaction can write to in-cache (in-memory) pages

   3. Exclusive Lock

      > No pending locks here as it is internal lock that is not visible to client.
      >
      > Pager will never request a pending lock, it will ONLY require a exclusive
      > lock.
     
      After acquiring a exclusive lock, it becomes a (full) write-transaction.

   > State change:
   >
   > ```
   > nolock -> shared -> reserved lock -> pending lock -> exclusive lock -> no lock
   > ```

4. State machine of lock acquisition

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-27%2017-43-02.png)

   > You can see that the you can go back to the nolock state from every state
   > except itself, this is probably because scheduling.

5. Source code functions:

   1. [`sqlite3OsLock()`](https://github.com/sqlite/sqlite/blob/60aca33a8b8f8d8364ecaa6565128fc4c1f298fd/src/os.c#L107)

      This fucntion can be used to:
      
      1. Obtain a new lock
      2. Upgrade an existing lock

   2. [`sqlite3OsUnlock()`](https://github.com/sqlite/sqlite/blob/60aca33a8b8f8d8364ecaa6565128fc4c1f298fd/src/os.c#L112)
      
      This function can be used to:

      1. Release a lock
      2. Downgrade a lock

## 4.2.3 Explicit locking

1. By default, locks in SQLite are implicit, they is totally managed by the 
   Pager module, however, as an user, there are two ways to explicitly tell
   SQLite to lock the underlying database files

   1. `BEGIN EXCLUSICE TRANSACTION`
      
      Tell SQLite start a write transaction immediately, which would try to acquire
      an **exclusive** lock on all the open database files (main and other database files)

      > It can fail with `database is locked` if a exclusive lock cannot be acquired.

   2. `BEGIN IMMEDIATE TRANSACTION`

      Tell SQLite start a write transaction immediately, which would try to acquire
      a **reserved** lock on all the open database files (main and other database files)
   
   > QUES: to acquire an **exclusive** lock, a connection has to go through the 
   > lock stages: `nolock -> shared -> reserved -> pending -> exclusive`, right?

   > From the official SQLite [doc](https://www.sqlite.org/lang_transaction.html),
   > the above 2 commands are same under the WAL journal mode. Under the legacy
   > rollback journal mode, `BEGIN IMMEDIATE TRANSACTION` allows other connections
   > to read the database file.

2. `BEGIN DEFERRED TRANSACTION`

   This technically does not belong to the section `Explicit locking`, but since
   we have coverd 2 transaction modes here, let's do this as well.

   When you type:

   ```sh
   sqlite> BEGIN TRANSACTION
   ```

   You are using `BEGIN DEFERRED TRANSACTION`, this transaction starts without 
   creating any transaction, the actual transactions will be created when needed.

   If the first statement after BEGIN DEFERRED is a SELECT, then a read transaction
   is started. Subsequent write statements will upgrade the transaction to a write
   transaction if possible, or return SQLITE_BUSY. 

   If the first statement after BEGIN DEFERRED is a write statement, then a write 
   transaction is started.


## 4.2.4 Deadlock and starvation

1. Deadlock can happen

   Assume 2 transactions are holding two shared locks on a database file, they both
   require the reserved lock, transaction 1 gets it, since there is ONLY one 
   reserved lock allowed on a file, transaction 2 waits. Then transaction 1 wants
   an exclusive lock, it waits for transction 2 to release the shared lock, but
   transaction 1 is actually also waiting for transaction 2, then deadlock.

2. How to prevent deadlock

   1. prevention
      
      SQLite always requires a lock in the non-blocking mode, when a transaction
      requires a lock, it will either;

      1. get the lock
      2. get an error message indicating that the lock is not available, so that
         it won't be blocked, and this transaction will retry a finite number of
         times

         > The retry number can be adjusted at runtime, the default value is 0.

         If all retires fail, SQLite returns the `SQLITE_BUSY` error code.

         ```sh
         # Session 1
         $ sqlite3 MyDB
         SQLite version 3.40.1 2022-12-28 14:03:47
         Enter ".help" for usage hints.
         sqlite> begin exclusive transaction;

         # Session 2
         $ sqlite3 MyDB
         SQLite version 3.40.1 2022-12-28 14:03:47
         Enter ".help" for usage hints.
         sqlite> begin exclusive transaction;
         Runtime error: database is locked (5)
         sqlite>
         ```


   2. detection and break

      > SQLite does not use this

3. startvation

   Deadlock isn't possible, but starvation is, when a transaction continuously
   failed to acquire a lock.


## 4.2.5 Linux lock primitives

1. For the Linux file system locks, check out [this post](https://gavv.net/articles/file-locks/)

2. Locks on a file are not a content of the file, tey are merely in-memory data
   objects maintained by the kernel and thus won't survive system failures. If
   a process crashes or exits, the kernel cleans up all locks hold by the 
   applications.

## 4.2.6 SQLite lock implementation

> QUES: Well, I don't understand this section, the offical doc says that SQLite
> does NOT use the lock-byte page at all, and for a SQLite file smaller than 1G,
> it has 0 lock-byte page.
>
> This section says that the SQLite locks are implemented using the lock-byte 
> page, if a SQLite lock-byte page does not exist, how can this work??????


> QUES: From the official [doc](https://www.sqlite.org/fileformat.html),
> SQLite does not use lock-byte page, it is ONLY used by some specific 
> VFS implementations, I am confused

> Also, you may want to update the note in Ch3/Page types when you have the answer
> to the above question.

> TODO: revisit this section in the future.

1. SQLite won't lock the entire file but only specific regions needed.

2. SQLite implements its own lock types based on the file system locks provided
   by the underlying OS.

   ```
   Linux fs locks (read/write) -> SQLite locks (shared/reserved/pending/exclusive)
   ```

   | SQLite lock | Linux fs lock |
   |-------------|---------------|
   |shared       | read lock     | 
   |reserved     |





### 4.2.6.1 Translation from SQLite locks to native file locks
### 4.2.6.2 Exgineering issues with native locks
### 4.2.6.3 Linux system issues
### 4.2.6.4 Multithreaded applications
## 4.2.7 Lock APIs
### 4.2.7.1 The `sqlite30sLock` API
### 4.2.7.2 The `sqlite30sUnlock` API

# 4.3 Journal Management

1. The functionalities of a journal

   1. roll back a transaction
   2. survive a system crash

2. Different journal modes

   1. delete (delete the journal afte the write-transaction, the default one)
   2. truncate (set the size to 0 afater the write-transaction)
   3. persist (do nothing to it after the transaction)
   4. memory (the journal is stored in memory)

      > What will happen to the in-memory journal after the write-transaction

   5. off (no journaling)
      
      > This is the default option for memory databases
      >
      > ```sh
      > sqlite :memory:
      > ```

3. SQLite keeps track of which pages are journaled (modified) by the current 
   transaction using a bitmap in the memory.

   > Seems that SQLite does not use roaring bitmap though

   > QUES: why does SQLite need to track this?


## 4.3.1 Logging protocol

1. SQLite writes the log before modifing the database file

2. SQLite ensures that the log is persisted to the disk before actually modifying
   the database file.

## 4.3.2 Commit protocol

> QUES: When a commit failed because a power failure, what should the DBMS do
> after the power recovery, re-commit or rollback?
>
> In SQLite, it seems that it will roll back the system

1. The default commit logic in SQLite is:

   1. Persist the database file 
   2. Persist the rollback journal

   Step 2 is necessary because we need this to survive the power failure in
   the commit.

   Step 1 does not makes sense to me, the book says this is needed because
   SQLite does not have a redo logic under the legacy/rollback journal, but 
   we rollback it rather than "redoing" it.

2. Asynchoronous transactions and lazy commit

   By default, transactions in SQLite are synchronous, it strictly follows the 
   logging and commit protocol mentioned above.

   Though not recommanded, SQLite also permits applications to run transactions
   in the lazy commit mode, under this mode, no journal and database flushing are
   done at the commit time, and thus it will be very fast.

   > No flush, no data durability.

   One can enable asynchronous transactions by seting the synchronous pragma 
   variable to 0.

# 4.4 Subtransaction Management
