> This chapter is mainly about:
>
> * The SQLite database system and what it is
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
> * 2.5 SQLite Limitations
> * 2.6 SQLite Architecture
>   * 2.6.1 Frontend
>   * 2.6.2 Backend
>   * 2.6.3 The Interface
> * 2.7 SQLite Source Organization
>   * 2.7.1 SQLite APIs
>   * 2.7.2 Tokenizer
>   * 2.7.3 Parser
>   * 2.7.4 Code generator
>   * 2.7.5 Virtual Machine
>   * 2.7.6 The Tree
>   * 2.7.7 The Pager
>   * 2.7.8 OS interface
> * 2.8 SQLite Build Process

# Before the first section

1. A single SQLite database file consists of multiple B+Trees, i.e., multiple
   tables belong to this database.

# 2.1 Introduction to SQLite

## 2.1.1 Salient SQLite characteristics
## 2.1.2 Usage simplicity
## 2.1.3 SQL features and SQLite commands

1. SQL features that are not supported in SQLite

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

      Reset the statement iterator to its initial state, except that the bind 
      variables remain same.

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

   SQLite uses **POSIX record locks**, which can not be used to sync multiple 
   threads.

## 2.2.5 Working with multiple databases
## 2.2.6 Working with transactions
## 2.2.7 Working with a catalog

1. The catalog table used by SQLite are named in `sqlite_*`, e.g., `sqlite_master`.

   Users are not allowedd to drop or modify it, or create index on it.

   ```shell
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

   ```shell
   :memory:> create table sqlite_xxxx (id int);
   object name reserved for internal use: sqlite_xxxx
   ```

## 2.2.8 Using the sqlite3 executable

# 2.3 Transaction Support

1. By default, SQLite runs at *autocommit* mode, i.e., a transaction will be 
   created for every SQL statement (without manual `BEGIN`), and every transaction
   will be automatically committed.

   Autocommit can be detrimental to performance, as for each write-transaction, SQLite
   requires to open, access and close the journal file. If it is used in multi-threaded
   context, then there is also the lock overhead.

   > For more information on the journal file, see the contens about rollback 
   > journal in Ch3.

2. The `BEGIN` command takes SQLite out of the *autocommit* state, into the manual
   commit mode, and thus starts a transaction.

   Command `COMMIT` or `ROLLBACK` ends the transaction, making SQLite back to
   the *autocommit* mode.

   If a `COMMIT` or `ROLLBACK` is not provided, then SQLite automatically rolls 
   back the transaction when the connection closes.

3. Nested transaction is NOT supported in SQLite, i.e.:

   ```shell
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

   ```shell
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

   Differnet DBMSs handle failure in a transaction differently.

## 2.3.1 Concurrency control

## 2.3.2 Database recovery

1. The rollback is supported via a separate journal file

   > Such a rollback is mostly for write-transactions, there is no need to rollback
   > a read-transaction.

   The jouranl file will always be created in the same directory where the database
   file resides and have the same name but with a `-journal` appended.

   ```shell
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

   ```shell
   # Shell A

   db> commit
   ```

   ```shell
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

   > There won't be multiple journal files as at most 1 write-transaction exists
   > at any time.

# 2.4 SQLite Catalog

1. In RDBMSs, catalogs themselves are tables (often called system tables).

   For example, the schema descriptions (SQL create statements) are stored as
   rows in catalogs.

   By default, `sqlite_master` is the ONLY catalog that is enabled in SQLite.

   Schema for the `sqlite_master` table:

   ```SQL
   create table sqlite_master (
       -- available types: table/index/view/trigger 
       type text,

       -- name of the object, for index that does not have a name, the name will 
       -- be `sqlite_autoindex_TABLE_N`, where `TABLE` is the name of the table,
       -- `N` is an integer indicating which index this is.
       name text,

       -- The name of the table if `type` is table or index. Or it is which table 
       -- this thing is associated with.
       tbl_name text,

       -- Which page is the root of this thing (B-Tree or B+Tree) on
       --
       -- One thing to note is that SQLite page index starts with 1, page 0 
       -- literally means it does not exist.
       rootpage integer,

       -- the `CREATE` SQL statement 
       sql text 
   )
   ```

   If you execute a `CREATE xxx` statement, then a new row will be inserted into
   this table.

   > Every object except for the `sqlite_master` itself will have an entry in 
   > `sqlite_master`.

   When SQLite opens a database file, it first scans the entire master table, 
   and processes the `sql` column in each row and produces many in-memory table,
   these in-memory table can collectively define a schema cache.


   ```shell
   $ sqlite
   sqlite> select * from sqlite_master;

   sqlite> create table student (id int primary key);

   sqlite> select * from sqlite_master;
   table|student|student|2|CREATE TABLE student (id int primary key)
   index|sqlite_autoindex_student_1|student|3|
   ```

2. There is another catalog `sqlite_temp_master` that is used to store the metadata
   of all the temporary objects.

   For each `library connection`, SQLite will create a temporary **databse** that 
   stores all temporary objects.

   > This means that for a library connection, at least 2 database connections will
   > be established.

   ```shell
   $ sqlite
   sqlite> select * from sqlite_temp_master;

   sqlite> create temp table student (id int primary key);

   sqlite> select * from sqlite_temp_master;
   table|student|student|2|CREATE TABLE student (id int primary key)
   index|sqlite_autoindex_student_1|student|3|
   ```

   When the library connection ended, the temporary database will be deleted.

3. `sqlite_sequence` for tables that have `integer primary key autoincrement` 
   column

   Such a autoincrement feature is implemented based on the following catalog
   table:

   ```SQL
   create table sqlite_sequence(
       -- name of the table
       name text,
       
       -- largest value so far issued for the autoincrement column
       seq integer 
   );
   ```

   > A table can have at most one autoincrement column, that is why
   > this catelog table does not store the column name.

   ```shell
   sqlite> create table student (id integer primary key autoincrement);

   sqlite> select * from sqlite_sequence;

   sqlite> insert into student values (0);
   sqlite> select * from sqlite_sequence;
   student|0

   sqlite> insert into student values (1);
   sqlite> select * from sqlite_sequence;
   student|1
   ```
   
   Note that the sequence catalog will be created ONLY when you attempt to insert
   a row to a table that has a autoincrement column.


3. Users are not allowed to modify the catalog table or create indexes on them.

   And SQLite does not create indexes on them either.

# 2.5 SQLite Limitations

1. The following is a list of shortcomings of SQLite:

   1. Limited SQL-92 support

      For SQL-92 features that are not supported by SQLite, see 
      http://www.sqlite.org/omitted.html

   2. No nesting transaction (subtransaction) support, i.e.:

      ```shell
      sqlite> begin;
      sqlite> begin;
      ``` 

   3. Low concurrency

   4. Application restriction

      Since SQLite aims to be simple, it is ONLY suitbale for small-size 
      applications

   5. NFS (Network File system) problems

      SQLite relies on file-level locks to handle concurrency, however, on most
      NFSs, locks contains bugs in their locking logic.

      Another thing is that NFS has high latency, so SQLite on a NFS is less 
      performant.

   6. Liminations on the number of objects in a table or index.

   7. Does not have `stored procedure`

      > Like a function in PL, store a group of SQL statements so that you can 
      > reuse them later.

# 2.6 SQLite Architecture

1. The architecuture of SQLite is quite modular, it consists of 7 major components,
   and are divided into 2 divisions:

   1. Frontend Parsing System

      Compiles the SQL statement

   2. Backend Engine

      Executes the compiled statement

   ![arch](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-17%2009-51-58.png)

## 2.6.1 Frontend

1. The frontend division is composed of three modules

   1. Tokenizer
      
      Splits an input SQL statement into tokens

   2. Parser
      
      1. Parse the tokens into an AST.
      2. Optimize the AST

         > This is kinda weird, never seen any optimization done in such a place.

   3. Code generator

      Generate an equivalent bytecode program 


   > The frontend implements the `sqlite3_prepare` API.

## 2.6.2 Backend

1. The backend is the engine that executes the bytecode generated from the
   frontend.


   It consists of four moduels:

   1. The `Virtual Machine`
      
      Executes the bytecode

   2. The tree
      
      > B+Tree for tables or B-Tree for indexes.

      The underlying data structure

   3. The pager

      More like the buffer pool manager  

   4. OS interface 

      Syscalls, locks...


## 2.6.3 The Interface

# 2.7 SQLite Source Organization

1. Directories and their usage

   * [art](https://github.com/sqlite/sqlite/tree/master/art): gif, logo 
   * [contrib](https://github.com/sqlite/sqlite/tree/master/contrib): TCL/TK console widget
   * [doc](https://github.com/sqlite/sqlite/tree/master/doc): documentations
   * [ext](https://github.com/sqlite/sqlite/tree/master/ext): extensions, like async I/O, rtree, full text search
   * [src](https://github.com/sqlite/sqlite/tree/master/src): source code
   * [test](https://github.com/sqlite/sqlite/tree/master/test): tests
   * [tools](https://github.com/sqlite/sqlite/tree/master/tools): sources for code generators

## 2.7.1 SQLite APIs
## 2.7.2 Tokenizer

> tokenize.c

1. In SQLite, the tokenizer involves parser, this is different from the parser
   that most people are familiar with, normally, it is the parser that should
   involves the tokenizer.

## 2.7.3 Parser

It uses Lemon parser under the hood, the source code for the Lemon parser 
(`lemon.c`) is not in the `src` directory but in `tool`.

The code that drives Lemon parser is in `parse.y`, the files generated from
Lemon are `parse.h` and `parse.c`

## 2.7.4 Code generator

1. Files involved by the code generartor module are:
   1. attach.c
   2. auth.c
   3. build.c
   4. delete.c
   5. expr.c
   6. insert.c
   7. pragma.c
   8. select.c
   9. trigger.c
   10. update.c
   11. vacuum.c
   12. where.c

   These are files where most of the SQLite arithmetic and logic reside.

## 2.7.5 Virtual Machine

1. A bytecode instruction is similar to the machine code, it consists of

   1. An opcode
   2. Up to five operands

2. Thes file for the VM are 
   1. `vdbe.c`
   2. `vdbe.h` 
   3. `vdbeInt.h.`
   4. `vdbeaux.c.`
   5. `vdbeapi.c.`
   6. `vdbeapi.c.`
   7. `vdbemem.c.`

   8. `func.c.`: most functions
   9. `date.c.`: date-related functions

   10. `util.c`: some utility functions
   11. `hash.c`: hash fuctions

   12. `utf.c`: conversion between UTF-8 and UTF-16

   13. `printf.c`: custom `printf` impl

   14. `random.c`: random number generator
   
   > VDBE: Virtual Database Engine

## 2.7.6 The Tree

> `btree.c` and `btree.h`

## 2.7.7 The Pager

> `pager.c`

## 2.7.8 OS interface

> `os.h`
> 
> `os_unix.c` for UNIX
> `os_win.c` for Windows

# 2.8 SQLite Build Process

1. The source code for the `sqlite3` binary is in `shell.c`
