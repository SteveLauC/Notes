> How to implement undo

1. A DBMS's concurrency control and recovery components permeate throughout 
   the design of its entire architecture.

We need formal correctness criteria to determine
whether an interleaving is valid.

1. With shadow paging, there is no need to use logs

   Is this right, what about torn writes?


2. Isolation: They do not see the effects of concurrent transactions

   A tx will read the data committed by other transactions

   tx1      tx2
   read(A) 
            write(A)
            commit;
   read(A) 

   > Postgres store metadata using tables, `DROP TABLE` will also be executed
   > within a tx, if the isolation level is read committed, then a long-running 
   > transaction that replies on a table could see that the table is gone.
   >
   > FUTURE steve: As long as the long-running tx accesses a table, another tx
   > that tries to drop it blocks until the long-running tx completes. 
   >
   > QUES: figure out why

* There are 2 types of consistency:

  1. Database consistency 
  2. Transaction consistency 

* Conflicts

  * read-write conflicts

    Nonrepeatable read

-----

# Atomicity

1. 2 ways to implement Atomicity

   1. Use a log

      When a txn aborts, you undo the changes according to the log.

   2. Page shadowing

      Whenever a txn modifies a page, you copy the old page to a new one and do
      the update there, a pointer is used to track the page location, when the
      txn commits, you update this pointer (this has to be done atomically).
      
      > SQLite3 uses this approach

   Most DBs use the first approach. However, the second approach does have its
   advantages. Aborting a txn is easy as you only need to drop the changes made
   in the new page, and regularly gc the pages. With the first approach, you have
   to take a look at the log and undo changes accordingly.

# Consistency

# Isolation

1. Concurrency control protocol is how DBMS interleaves txns to maximize 
   concurrency but still guarantee a correct output.

   There are 2 kinds of concurrency control protocol:

   1. Pessimistic: assume there are a lot of issue, we should kill the issue at
      the first place. 
   2. Optimistic: assume issues are rare, and only deal with them when they happen. 

2. Anomalies that could happen with conflict

   1. Read-write conflict -> Non-repeatable read

      ```
      T1       T2

      begin;
      read(A);
               begin; 
               read(A); 
               write(A);
               commit;
      read(A);            (non-repeatable read)
      commit;
      ```

      This is not conflict-serializable.

   2. Write-read conflict -> dirty read

      ```
      T1        T2
      begin;
      read(A);
      write(A);
                read(A);   (dirty read)
                write(A);
                commit;
      abort;
      ```

      QUES: Is this schedule conflict-serializable?

   3. Write-write conflicts -> lost update

      ```
      T1         T2
      begin;
      write(A);
                 begin;
                 write(A);
                 write(B);
                 commit;
      write(B);
      commit;
      ```

      A is overwritten by T2, B is overwritten by T1, which could never happen
      if T1 and T2 execute in serial order, all values should be overwritten 
      by one transaction.

      This is not conflict-serializable.

3. Most DBMS's serializable isolation level is "conflict serializable"

4. TIL term "interactive transaction", where the users submit txn queries 
   interactively so that the DBMS knows nothing about the txn intent.

    

I don't fucking understand these things: https://stackoverflow.com/q/79358190/14092446