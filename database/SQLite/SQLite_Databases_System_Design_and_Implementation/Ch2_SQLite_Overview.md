> This chapter is mainly about:
>
> * THe SQLite database system and what it is
> * What salient features SQLite supports
> * How SQLite stores tables in database files
> * How to write, compile, and execute SQLite applications
> * Some most frequently used SQLite APIs
> * The modular SQLite architecture
> * SQLite limitations


> * 2.1 Introduction to SQLite
>   * 2.1.1 Salient SQLite characteristics
>   * 2.1.2 Usage simplicity
>   * 2.1.3 SQL features and SQLite commands
>   * 2.1.4 Database Storage
>   * 2.1.5 Limited Concurrency
>   * 2.1.6 SQLite Usage
> * 2.2 Sample SQLite Applications
>   * 2.2.1 A Simple Application
>   * 2.2.2 SQLite APIs
>   * 2.2.3 Direct SQL execution
>   * 2.2.4 Multithreaded applications
>   * 2.2.5 Working with multiple databases
>   * 2.2.6 Working with transactions
>   * 2.2.7 Working with a catalog
>   * 2.2.8 Using the sqlite3 executable
> * 2.3 Transaction Support
>   * 2.3.1 Concurrency control
>   * 2.3.2 Database recovery
> * 2.4 SQLite Catalog

# Before the first section

1. A single SQLite database file consists of multiple B+Trees, i.e., multiple
   tables belong to this database.

# 2.1 Introduction to SQLite

## 2.1.1 Salient SQLite characteristics
## 2.1.2 Usage simplicity
## 2.1.3 SQL features and SQLite commands

1. SQL features that are supported in SQLite

   > https://sqlite.org/omitted.html

   1. Complete `ALTER TABLE` support
   2. Complete trigger support
   3. Writing to VIEWs
   4. `GRANT` and `REVOKE`

## 2.1.4 Database Storage

1. SQLite stores all the data that a database uses in a single file

## 2.1.5 Limited Concurrency

1. The concurrency level supported by SQLite is database level, and it supports

   1. multiple readers to a database file
   2. One exclusive writer to a database file

## 2.1.6 SQLite Usage
# 2.2 Sample SQLite Applications
## 2.2.1 A Simple Application

1. demo code in Rust

   ```rust
   fn main() {
       let db = sqlite::open("MyDB").unwrap();

       let statement =
           db.prepare("select SID from students order by SID").unwrap();

       for row in statement {
           let row = row.unwrap();
           let sid = &row[0];
           println!("SID = {:?}", sid);
       }
   }
   ``` 
## 2.2.2 SQLite APIs

> Reference: [C-language Interface Specification for SQLite](https://www.sqlite.org/capi3ref.html)

1. `sqlite3_open`

   The db file opening or creation is done in a **lazy mode**, it will be ONLY 
   opened or created when you access it for the first time.

   > It is recommended that new programs should use the `sqlite3_open_v2` 
   > interface.

2. `sqlite3_prepare`

   Compile a SQL statement into the SQLite bytecodes.

   It returns a `Statement` that can be seen as an iterator of the result rows.

3. `sqlite3_step`

   Call `next()` on the `Statement` iterator by executing the bytecode program.

   For SQL statements that does not return rows, the iterator yields nothing.

4. `sqlite3_column_*`

   Extract a specific column from a row `SQLITE_ROW`. 

5. `sqlite3_finalize`

   Closes and destroys a statement handle, and user will get a sign indicating
   the excuting result.

6. `sqlite3_close`

   Closes a library connection, if there are prepared statement that has not been
   finalized, this function returns `SQLITE_BUSY`.

7. Other userful functions

   1. `sqlite3_bind_*`

      This function is for binding variables.

   2. `sqlite3_reset`

      Reset the statement to its initial state, except that the bind variables 
      remain same.

      To clear bind variables, use `sqlite3_clear_bingdings`a.

## 2.2.3 Direct SQL execution

1. Demo code in Rust

   ```rust
   use std::{env::args, process::exit};

   fn main() {
       let av = args().collect::<Vec<_>>();
       if av.len() != 3 {
           eprintln!("Usage: {} DATABASE-NAME SQL-STATEMENT", av[0]);
           exit(-1);
       }

       let db = sqlite::open(av[1].as_str()).unwrap();

       db.iterate(av[2].as_str(), |pairs| {
           for (name, value) in pairs.iter() {
               print!("{} = ", name);
               if let Some(value) = value {
                   println!("{}", value);
               } else {
                   println!("NULL");
               }
           }
           true
       })
       .unwrap();
   }
   ```

## 2.2.4 Multithreaded applications

1. Thread model in SQLite is controlled by the `SQLITE_THREADSAFE` macro:

   * 0: single-threaded mode: unsafe to use in multiple-threaded context

        > `!Send + !Sync`

   * 1: multi-thread mode: can be used by multiple threads as long as no database 
        connection is used by two or more threads simultaneously

        > Each database connection is `Send` but not `Sync`.

   * 2: serialized mode: thread-safe, no restriction

        > `Send + Sync`

   > ref: https://www.sqlite.org/threadsafe.html 

2. In the Rust library `sqlite`, there are actually two `Connection` types

   1. [`Connection`](https://docs.rs/sqlite/latest/sqlite/struct.Connection.html)
      
      This corresponds to `1: multi-thread` mode 

   2. [`ConnectionWithFullMutex`](https://docs.rs/sqlite/latest/sqlite/struct.ConnectionWithFullMutex.html)

      This corresponds to `2: thread-safe` mode 

3. For multiple processes support, SQLite uses file system RwLock to handle 
   concurrency.

   But file locks on POSIX platforms are `advisory locks`, not `mandatory locks`,
   if a process is not aware of the locks, then it can access the file without
   any issues.

4. It is not safe to use SQLite after the `fork()` syscall:

   SQLite uses file system locks to handle process concurrency, but these locks 
   are mostly `advisory locks` instead of `mandatory locks`, i.e., if a process is 
   not aware of the lock, then it can still access (read or write) the file 
   without any issues. For a spawned child process, it is not aware of the lock 
   (as the connection has already been created in the program, parent process 
   and child process are the same program until the child process involves `exec_*`
   functions), thus it can do anything with the database file without any 
   restrictions, which is totally unsafe.

   > [File locking in Linux](https://gavv.net/articles/file-locks/)

## 2.2.5 Working with multiple databases
## 2.2.6 Working with transactions
## 2.2.7 Working with a catalog

1. The catalog table used by SQLite are started with `sqlite_*`, e.g., `sqlite_master`.

   Users are not allowedd to drop or modify it.

   ```
   $ litecli -D :memory:
   Version: 1.9.0
   Mail: https://groups.google.com/forum/#!forum/litecli-users
   GitHub: https://github.com/dbcli/litecli

   :memory:> drop table sqlite_master;
   You're about to run a destructive command.
   Do you want to proceed? (y/n): y
   Your call!
   table sqlite_master may not be dropped

   :memory:> insert into sqlite_master values(1);
   table sqlite_master may not be modified
   ```

   And we are not allowed to create tables named like the catalog tables:

   ```
   :memory:> create table sqlite_xxxx (id int);
   object name reserved for internal use: sqlite_xxxx
   ```

## 2.2.8 Using the sqlite3 executable

# 2.3 Transaction Support

1. By default, SQLite runs at *autocommit* mode, i.e., a transaction will be 
   created for every SQL statement (without manual `BEGIN`), and every transaction
   will be automatically committed.

   Autocommit can be detrimental to performance, as for each transaction, SQLite
   requires to open, access and close the database file. If it is used in multi-threaded
   context, then there is also the lock overhead.

2. The `BEGIN` command takes SQLite out of the *autocommit* state, into the manual
   commit mode, and thus starts a transaction.

   Command `COMMIT` or `ROLLBACK` ends the transaction, making SQLite back to
   the *autocommit* mode.

   If a `COMMIT` or `ROLLBACK` is not provided, then SQLite automatically rolls 
   back the transaction when the connection closes.

3. Nested transaction is NOT supported in SQLite, i.e.:

   ```
   :memory:> BEGIN;
   Query OK, 0 rows affected
   Time: 0.000s

   :memory:> BEGIN;
   cannot start a transaction within a transaction
   ```

4. Try this:

   SQLite:

   ```rust
   :memory:> create table student (id int unique);
   Query OK, 0 rows affected
   Time: 0.000s

   :memory:> begin;
   Query OK, 0 rows affected
   Time: 0.000s

   :memory:> insert into student values (1);
   Query OK, 1 row affected
   Time: 0.000s

   :memory:> insert into student values (1);
   UNIQUE constraint failed: student.id
   :memory:> commit;
   Query OK, 0 rows affected
   Time: 0.000s

   :memory:> select * from student;
   +----+
   | id |
   +----+
   | 1  |
   +----+
   1 row in set
   Time: 0.006s
   ```

   PostgreSQL:

   ```
   steve@(none):steve> create table student ( id int unique);
   CREATE TABLE
   Time: 0.009s

   steve@(none):steve> begin;
   BEGIN
   Time: 0.001s

   steve@(none):steve> insert into student values (1);
   INSERT 0 1
   Time: 0.001s

   steve@(none):steve> insert into student values (1);
   duplicate key value violates unique constraint "student_id_key"
   DETAIL:  Key (id)=(1) already exists.
   Time: 0.002s

   steve@(none):steve> commit;
   ROLLBACK
   Time: 0.001s

   steve@(none):steve> select * from student;
   +----+
   | id |
   |----|
   +----+
   SELECT 0
   Time: 0.003s
   ```

   Differnet DBMSes handle failure in a transaction differently.

## 2.3.1 Concurrency control

## 2.3.2 Database recovery

1. The rollback is supported via a separate journal file

   > Such a rollback is mostly for write-transactions, there is no need to rollback
   > a read-transaction.

   The jouranl file will always be created in the same directory where the database
   file resides and have the same name but with `-journal` appended.

   ```
   # Shell A

   $ litecli -D db
   Version: 1.9.0
   Mail: https://groups.google.com/forum/#!forum/litecli-users
   GitHub: https://github.com/dbcli/litecli

   db> begin
   Query OK, 0 rows affected
   Time: 0.000s

   db> create table student (id int unique);
   Query OK, 0 rows affected
   Time: 0.000s
   db>
   ```

   ```shell
   # Shell B

   $ ls -l db*
   .rw-r--r--@   0 steve 14 Aug 20:14 db
   .rw-r--r--@ 512 steve 14 Aug 20:14 db-journal
   ```

   ```
   # Shell A

   db> commit
   ```

   ```
   # Shell B

   $ ls -l db*
   .rw-r--r--@ 12k steve 14 Aug 20:16 db
   ```

   > SQLite logging is inefficient: every log record contains the image of the
   > **entire database page** even when the transaction modifies a single byte in
   > the page.
   >
   > Because the log stores everything, the recovery logic becomes simple.

2. Lifecycle of a journal file 

   By default, SQLite creates a journal file for every write-transaction, and 
   deletes it when the transaction commits.

   But there are some options so that journal file won't be deleted on commit,
   see the `journal_mode` macro.

# 2.4 SQLite Catalog
