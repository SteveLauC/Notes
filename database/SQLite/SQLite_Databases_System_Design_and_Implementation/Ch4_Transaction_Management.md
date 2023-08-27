> This chapter is mainly about:
>
> * How SQLite implements ACID properities
> * The SQLite way of managing various locks and their mappings to native file
>   locks and lock transactions
> * How SQLite avoids deadlocks
> * How SQLite implements journal protocols
>
>   > BTW, what is journaling protocols
>
> * How SQLite managers savepoints in user transactions
>
>   > A SAVEPOINT is a point in a transaction in which you can roll the transaction
>   > back to a certain point **without rolling back the entire transaction**. 

> * 4.1 Transaction Types
>   * 4.1.1 System transaction
>   * 4.1.2 User transaction
>   * 4.1.3 Savepoint
>   * 4.1.4 Statement subtransaction
> * 4.2 Lock Management


# 4.1 Transaction Types
## 4.1.1 System transaction

1. When in *autocommit* mode, those transactions are called system or auto or 
   implicit transactions

2. Under the *autocommit* mode, SQLite will create a read-transaction for a 
   `SELECT` statement. For a non-`SELECT` statement, SQLite first creates a
   read-transaction, and then converts it into a write-transaction when it
   actually modifies the database file.

   > Why would SQLite do this for non-`SELECT` statement, my thought would be
   > for maximizing concurrency as multiple read-transactions can occur at
   > the same time.

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

   COMMIT; (or rollback)
   ```

   > I would call this *manual mode* rather than a user transaction

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

   > Currently I don't understand how a savepoint outside a transaction works,
   > per the [document](https://www.sqlite.org/lang_savepoint.html), it seems
   > to work like a `BEGIN DEFERRED TRANSACTION`(Ok, what is this then?).

3. We are pretty clear that SQLite does not support nested transactions, but
   the document says that you can emulate this using savepoints. 

## 4.1.4 Statement subtransaction


# 4.2 Lock Management
