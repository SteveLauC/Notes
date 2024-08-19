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
   
     When 2 write transations both:
     
     * read the data into its local memory
     * update the data in the local memory
     * commit the transaction
     
     The slower tx will be overwritten by the faster tx.
     
     SQL standard forbids this anomoly in all the isolcation levels.
     
     > Standard is just standard, in Postgres, you need at least `Repeatable read`
     > to avoid this: https://stackoverflow.com/a/77249495/14092446
     
     > In case of optimistic Hekaton MVCC, first-writer wins and the other 
     > transaction is aborted
     
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
    
  * Serialziation anomaly

    No anomalies could ever happen.