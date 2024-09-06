> This chapter is about:
>
> * Before I read it
>   * ...
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case

> What have you learned from it
>
> *
> *

> * 2.1 Consistency


# 2.1 Consistency

1. What is transaction and ACID

   A transaction is one or more operations that bring the data from one correct
   state to another correct state (consistency), these operations must be done
   atomically (atomicity), and won't be affected by other transactions(isolation).

   Once a transaction is committed, it can survive system failures (durability),
   which is guaranteed by WAL.

2. Properties ACD are very basics for databases, and should be implemented well.
   I is hard to do, so we have different Isolation levels.

# 2.2 Isolation Levels and Anomalies in SQL Standard

1. The SQL standards defines 4 isolation levels by whether some anomalies could
   happen.
   
2. Anomalies

   * Lost Update
   
     > QUES: 
   
     Lost update can be interrupted in 2 ways:
     
     1. When 2 write transations both:
      
        * read the data into its local memory
        * update the data in the local memory
        * commit the transaction
      
        The slower tx will be overwritten by the faster tx.
        
        In Postgres, with the Read committed isolation level, you can reproduce
        it with:
        
        ```sql
        select it from students where name = 'steve';
        0
        ```
        
        ```sql
        tx1: begin;
        tx2: begin;
        tx1: update students set it = (select it from students where name = 'steve') + 1 where name = 'steve';
        tx2: update students set it = (select it from students where name = 'steve') + 1 where name = 'steve';
        # tx2 blocks due to the usage of locks (try to write the same row)
        tx1: commit;
        tx2: commit; 
        
        tx1&tx2: select it from students where name = 'steve';
        1
        ```
        
        We reproduces it because the sub-query in both transactions will return 
        0 due to the snapshot isolation.
        
        If you slightly change the `update` statement, then you cannot reproduce
        it. tx2 gets blocked because the usage of lock, but once the tx1 commits,
        tx2 will see the modified value (value: 1, read committed isolation level),
        and updates it to 2.
        
        ```sql
        tx1: begin;
        tx2: begin;
        tx1: update students set it = it + 1 where name = 'steve';
        tx2: update students set it = it + 1 where name = 'steve';
        # tx2 blocks due to the usage of locks (try to write the same row)
        tx1: commit;
        tx2: commit; 
        
        tx1&tx2: select it from students where name = 'steve';
        2
        ```
        
        
     2. 
     
     SQL standard forbids this anomoly in all the isolcation levels.
     
     > Standard is just standard, in Postgres, you need at least `Repeatable read`
     > to avoid this: https://stackoverflow.com/a/77249495/14092446. You won't
     > have it in this isolation level or above because Postgres will abort
     > the second transaction.
     
     > In case of optimistic Hekaton MVCC, first-writer wins and the other 
     > transactions are aborted
     
  * Dirty read
  
    A transaction reads uncommitted data made by another transaction. If the tx
    that made the change will be rolled back, then the first tx will read the
    data that never exist.
    
  * Nonrepeatable read
  
    If a tx reads **a row** twice and gets different results because there is 
    another committed write tx happens between the 2 reads, this is nonrepeatable
    read.
    
  * Phantom read
  
    If a tx queries **rows** satisfying specific conditions twice and gets different
    results because there is another committed write tx happens between the 2 reads,
    this is a phantom read.
    
    > QUES(Solved): difference between "Nonrepeatable read"
    >
    > Nonrepeatable read considers 1 row, while Phantom read emaphsizes multiple
    > rows. From implementation's view, implementing Phantom read needs more 
    > locks.
    
  * Serialziation anomaly

    No anomalies could ever happen.
    
    > Note: it is more than the above listed anomalies, there is much higher
    > number of anomalies than the standard specifies.

    > QUES: then why they choose the above 4 anomalies to define the standard.
    >
    > Looks like no one konws this as theory is far from the practice.
    
3. Transaction isolation levels

   SQL standard defines 4 isolactoin levels accrording to the above anomalies:
   
   ![isolation](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-08-19%20at%201.36.55%20PM.png)
   
4. For a single row, if we put it in an `RwLock`, then for read txes, we acquire
   the read lock, and write locks for write tx, non-repeatable read can occur,
   and it seems to be a normal/expected behavior.
   

# 2.3 Isolation Levels in PostgreSQL

1. Postgres does not use lock-based transaction protocols, it uses Snapshot
   isolation, more specifically, MVCC (short for Multiversion concurrency 
   control).
   
   > QUES: I wanna know why
   
2. Because of using snapshot isolcation, different transactions would have a
   unique snapshot, they are isolcated, so dirty read will never happen, thus
   the Read Uncommitted isolation level is same as the Read Committed isolation
   level.
   
3. To show the default transaction isolation level in Postgres:

   ```sql
   steve=# show default_transaction_isolation;
    default_transaction_isolation
   -------------------------------
    read committed
   (1 row)
   ```
   
   When you use `BEGIN` (or don't) to start a transaction without explicitly
   specified isolation level, it uses the default one:
   
   ```sql
   steve=# show transaction_isolation;
    transaction_isolation
   -----------------------
    read committed
   (1 row)
   ```
   
   With `BEGIN`, one can specify the isolation level to use via:
   
   ```sql
   steve=# begin isolation level serializable;
   BEGIN
   steve=*# show transaction_isolation;
    transaction_isolation
   -----------------------
    serializable
   (1 row)
   
   steve=*# commit;
   COMMIT
   ```
   
4. Anomalies that are not specified by the SQL standard:

   1. Read skew
   
      Within the same transactions, same query, execute it twice, and you get
      different results due to a committed write transaction that happens 
      between the 2 queries.
      
      > This is different from Phantom Read, and I would say it is worse than
      > the Phantom Read as it happens within a single transaction.
      
      In Postgres, this is possible under the Read Committed isolation level,
      as that transaction indeed reads the committed change, Repeatable read
      would prohibit this.
      
   2. Lost update
   
      > This has been specified by the SQL standard, just want to demo it under
      > the Read Committed isolation level.
      
      Just let 2 write transaction set different values to the same row, due to
      the usage of lock, the second transaction will 
      
5. Latches usage under different isolation level in Postgres

   > NOTE: this is not 100% correct, it comes from my observation, not the source
   > code
   
   * Read Committed
   
     * Mutex when 2 write tranactions update the same row