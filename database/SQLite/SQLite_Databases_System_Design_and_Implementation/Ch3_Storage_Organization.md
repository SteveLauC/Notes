> This chapter is mainly about:
>
> * The organization of a single SQLite database
> * The formats of the database and various journal files
> * The concept of page in the SQLite context and the purpose of various pages
> * How a database is made platform independent
>
> Basically, we have:
> 1. The format of the database file
> 2. The format of the journal file
> 3. The format of the WAL file

> * 3.1 Database Naming Conventions
> * 3.2 Database File Structure
>   * 3.2.1 Page abstraction
>   * 3.2.2 Page size
>   * 3.2.3 Page types
>   * 3.2.4 Database metadata
>   * 3.2.5 Structure of freelist
> * 3.3 Journal File Structure
>   * 3.3.1 Rollback Journal 
>     * 3.3.1.1 Segment header structure
>     * 3.3.1.2 Log record structure
>   * 3.3.2 Statement journal
>   * 3.3.3 Multi-database transaction journal, the master journal(super journal)

> Questions I have
>
> 1. Seems like SQLite does not store the type of a page in the page itself, why?


# 3.1 Database Naming Conventions

1. If we create multiple connections to `:memory:`, then multiple in-memory
   database would be created.

2. Naming conventions of temporary files used by SQLite

   `etilqs_` followed by 16 random alphanumeric characters, without any extension.

   The prefix `etilqs` can be changed via the `SQLITE_TEMP_FILE_PREFIX` macro.

3. Temporary directories used by SQLite

   1. `/var/tmp`
   2. `/usr/tmp`
   3. `/tmp`


   > On my system, `/usr/tmp` is a symlink to `/var/tmp`.

   The temporary directory can be overwritten by the `TMPDIR` environment variable.


4. the database opened by SQLite (no matter it is in stored on disk or in memory),
   is named as `main` by SQLite:

   ```
   $ litecli -D steve_db
   Version: 1.9.0
   Mail: https://groups.google.com/forum/#!forum/litecli-users
   GitHub: https://github.com/dbcli/litecli

   steve_db> select * from steve_db.student;
   no such table: steve_db.student

   steve_db> select * from main.student;
   +----+
   | id |
   +----+
   | 1  |
   +----+
   1 row in set
   Time: 0.010s
   ``` 

   And for each connection, SQLite also maintains a temporary database, it is 
   named as `temp`. This temp database is created for every connection, and is
   not visible to other connections. Even if we have two connections connecting
   to the same database file, we get two differnt temporary databases.

5. You can `attach` a database file to its unique name by the following command:

   > This is how you open another database that is not named `main` in SQLite.

   ```shell
   SQLite> attach path_to_file <db>
   ```

   You can detach it via:

   ```
   SQLite> detach <db>
   ```

   > `detach` cannot be used for the `main` and `temp` databases

# 3.2 Database File Structure

## 3.2.1 Page abstraction

1. SQLite divides a database file into multiple fixed-size database pages. 

   SQLite pages start from `page 1`, not `page 0`

   > Page 0 liternally means a page that does not exist.(NULL)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-18%2010-07-17.png)

## 3.2.2 Page size


1. [Since SQLite 3.12, the default size for a page is 4096 bytes.](https://www.sqlite.org/pgszchng2016.html)

   ```shell
   steve_db> pragma page_size;
   +-----------+
   | page_size |
   +-----------+
   | 4096      |
   +-----------+
   1 row in set
   Time: 0.006s
   ```


   Also, you can query how many pages the current database takes via:

   ```
   steve_db> pragma page_count;
   +------------+
   | page_count |
   +------------+
   | 2          |
   +------------+
   1 row in set
   Time: 0.007s
   ```

2. Limitations on the page size

   The page size must be:

   1. A power of 2
   2. in range [512(2^9), 65536(2^16)]

   The lower bound is for performance reason, the upper bound exists because 
   SQLite stores page size in a `u16`.

3. Maximux database file size

   A database file can have at most 2^31 - 1 pages(i32), this pretty big, so
   actually, the limitation will be imposed by the underlying file system.

4. Change the page size

   When you create a new database, beforing creating the first table(or Page), 
   you can update the page size via:

   ```
   :memory:> pragma page_size;
   +-----------+
   | page_size |
   +-----------+
   | 4096      |
   +-----------+
   1 row in set
   Time: 0.010s
   :memory:> pragma page_size=512;
   Query OK, 0 rows affected
   Time: 0.000s
   :memory:> pragma page_size;
   +-----------+
   | page_size |
   +-----------+
   | 512       |
   +-----------+
   1 row in set
   Time: 0.006s
   :memory:>
   ```

## 3.2.3 Page types

1. SQLite has the following 4 page types

   1. free
      
      1. Trunk Page
      2. Leaf Page

   2. tree

      1. leaf
      2. internal
      3. overflow

   3. pointer-map
      
      > used by autovacuum and incremental vacuum features

   4. lock-byte

## 3.2.4 Database metadata

1. Any page in SQLite can be of any type, except for the Page 1

   Page 1 will always contain the following stuff:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-18%2011-09-36.png)

   1. A 100 bytes **file** header
   2. A B+Tree internal page storing the page number of the root pages of 
      `sqlite_master` and `sqlite_temp_master`
   3. Reserved space

2. The first page, and tables `sqlite_master` and `sqlite_temp_master` are the 
   metadata that SQLite uses.

3. What is vacuum in SQLite

   The `VACUUM` command rebuilds the database file (defragement the file), 
   repacking it into a minimal amount of disk space.

   > Compact the database file.

   For the reason why we need vacuumn in SQLite, see 
   [doc](https://www.sqlite.org/lang_vacuum.html).

4. Different vacuum modes supported by SQLite

   1. Autovacuum

      > Vacuum after every transaction.

      1. Incremental Vacuum

         This mode will do vacuum incrementally, i.e., a partial vacuum.

   2. Manual vacuum

      Involve it via the `vacuum` command. 

5. Format of the file header of the Page 1

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-18%2011-15-37.png)

   * Header String: "SQLite format 3" (including the tailing NUL) in the 
     UTF-8 format
     
     > This string can be changed via macro: `SQLITE_FILE_HEADER`

   * Page size in bytes (u16)

   * File format: write version and read version

     > Shouldn't this be only applied to a journaling file?

     These version number can either be 1, which is the old legacy rollback 
     journaling, or it is 2 for the new WAL journaling.

     Numbers other than 1 and 2 indicate that this database file cannot be
     read or write.

     If the SQLite team introduces a new format in the furture, then number
     3 will be taken, and so forth.

   * Reserved Space: SQLite may reserve a small fixed amount of space (<=255 bytes)
     at the end of each page for its own purpose.

     By default, the reserved space size is 0, it will be non-zero if the
     encryption feature is enabled for this database.

   * Max/Min Embedded Payload:

     Embedded Payload is the space that is consumed by a **reocrd**(seems to be 
     wrong here?) in a tree page, max embedded payload is the upper bound, min 
     embedded payload is the lower bound.

     The max embedded payload is 64 bytes, so the fraction is 25. The min embedded
     payload is 32 bytes, so the fraction is 12.5.

     > If the acutal payload is bigger than the Max embedded payload, then SQLite
     > move the extra bytes to a (or multiple) overflow pages.
     >
     > Once SQLite allocates an overflow page, it moves as many bytes as possible
     > into the overflow page as long as the embedded payload does not drop under 
     > the min embedded payload.

   * Max/Min Leaf Payload:

     Leaf Payload is similar to Embeded Payload, but only applies to B+Tree leaf
     pages.

     | Max Leaf Payload(fraction) | Min |
     |----------------------------|-----|
     |255(100)                    |32(12.5)|


     > Why does SQLite store the fraction here? It makes more sense to directly
     > store the limination in bytes so that you don't need to bother with floating
     > points.

   * File Change Counter

     Counter for write-transactions (only write-transaction will change the file)

     Initialized with 0, and will be incremented by 1 when a write-transaction is
     successfully performed on this file (database)

   * Database Size

     Size of the database in pages.

   * Freelist: A link list of free pages.

   * Number of freelist

   * Database Schema Cookie: A 4-byte integer number stored at offset 40; 
     initialized to 0. The value is incremented by one whenever the database 
     schema changes and it is used by prepared statements for their own 
     validity testing.

   * Other Meta variables: 14 4-byte integers

     1. Schema format number (at 44)
     2. Suggested Page cache size (at 48)
     3. The autovacuum related information (at 52)
     4. Text encoding (at 56)
        * 1 means UTF-8
        * 2 means UTF-16 LE
        * 3 means UTF-16 BE
     5. User version number (at 60)
        > Not used by SQLite, but by users
     6. Incremental vacuum mode (at 64)
        0 for no vacuum
     7. Version numbers 
     8. Other remaining bytes are reserved for future use

6. SQLite stores all multibyte values in Big-endian

7. SQLite file format is backward compatible back to the version 3.0.0, which means
   that the latest SQLite can still understand a database file created by SQLite 
   3.0.0.

## 3.2.5 Structure of freelist

1. Free page can be subtyped into 2 types:

   1. Trunk page

      SQLite separates free pages into trunks, and allocate a Trunk Page for each Trunk.

      > Trunk Page is used to store the metadata of the leaf page.

      A trunk page consists of three parts

      | Page number of the next trunk Page | # of leaf pages | Page num of Leaf Page | 
      |------------------------------------|-----------------|-----------------------|
      | 4B                                 | 4B              | Multiple 4B           |

   2. Leaf page

      This is the actual `free page`.

2. Structure of the freelist

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-21%2011-32-51.png)

# 3.3 Journal File Structure

1. For the **legacy** journal files, SQLite has

   1. Rollback journal

      Rollback journal is used to implement atomic transaction(`commit` and 
      `rollback`)

   2. Statement journal

      Used to undo **partial results** of **a single statement**. For example, 
      suppose an `UPDATE` statement will attempt to modify 100 rows in the 
      database. But after modifying the first 50 rows, the `UPDATE` hits a 
      constraint violation which should block the entire statement. The statement
      journal is used to undo the first 50 row changes so that the database is 
      restored to the state it was in at the start of the statement..

   3. Master journal (Super-journal)

      > Seems that master journal has been renamed to super journal

      The super-journal file is used as part of the atomic commit process when a 
      **single transaction** makes changes to **multiple databases** that have 
      been added to a single database connection using the `ATTACH` statement.

   > SQLite has introduced the WAL journaling scheme in 3.7.0.
   >
   > The structure of the WAL journal file will be talked about in Section 10.17.


## 3.3.1 Rollback Journal 

1. For every database, SQLite maintains a single rollback journal file.

   > In-memory databases do not use journal files, they store journal info
   > in the memory. 

2. The rollback journal file will always have the same name as the database
   file with `-journal` appended.

3. By default(journal mode: `DELETE`), a rollback journal file will be deleted
   when the write-transaction ends.

   You can query what the current journal mode is via: `pragam journal_mode`:

   ```
   $ litecli -D db
   Version: 1.9.0
   Mail: https://groups.google.com/forum/#!forum/litecli-users
   GitHub: https://github.com/dbcli/litecli
   db> pragma journal_mode;
   +--------------+
   | journal_mode |
   +--------------+
   | delete       |
   +--------------+
   1 row in set
   Time: 0.010s
   ```

   And you can change it via: `pragma journal_mode=XXX`

   ```
   db> pragma journal_mode=persist;
   +--------------+
   | journal_mode |
   +--------------+
   | persist      |
   +--------------+
   1 row in set
   Time: 0.006s
   db> pragma journal_mode;
   +--------------+
   | journal_mode |
   +--------------+
   | persist      |
   +--------------+
   1 row in set
   Time: 0.005s
   ```

4. Format of rollback journal file

   Each journal file is devided into varibale-sized log segments, every log segment
   starts with a header followed by one or more log records.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-23%2011-15-23.png)

### 3.3.1.1 Segment header structure

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-23%2011-49-24.png)

1. A rollback journal file is considered to be valid if it exists and have a valid
   header.

   > Header is used to check if the rollback journal file is valid.

2. Rollback journal header contains several components:

   * Magic Number (8B)

     A 8 bytes string: 0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7

   * Number of reocrds (in this segment) (4B)

     > This is stored as a signed integer

     For synchronous transactions, this field is initialized with 0.

     For asynchronous transactions, it will be initialized with -1 and **will 
     always be -1**.

   * Random number (4B)

     Used to compute the 'checksums' for individual log reocrd

   * Initial database page count(4B)

     How many pages were there in the original database file before this write-transaction.

   * Sector Size
     
     Sector size of the underlying file system.  SQLite will query the file system for this
     value, if not available, the 512 will be used as the default value.

     > The Segment header occupies a complete sector size.

   * Page Size

     SQLite page size

   * Unused spcae

3. What are asynchronous transactions?

   * Much faster than synchronous transactions
   * Does not flush the database file and the journal file
   * `number of records` will always be -1
   * Not recommended to use, mainly for test

4. Normally, a rollback journal file contains a single log segment.

5. In multi-segment journal file, the number of records field will never be -1.

   > No idea why

### 3.3.1.2 Log record structure

> A log segment consists of:
>
> 1. A segment header
> 2. One or multiple log records

1. Within a write-transaction, every non-SELECT statement creates one or more log 
   record.

   > A log record stores one page, so if a statement accesses multiple pages, then
   > multiple log records will be created.

2. Think about it, what is the easiest way to implement the rollback functionality,
   storing the original pages.

   Indeed, this is how SQLite handles it

   > 惊不惊喜，意不意外

   Every log record has the following structure:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-24%2010-27-58.png)

   The page number and the page contens, with a checksum on them.

   > The checksum is computed using the random number in the segment header.

## 3.3.2 Statement journal

> Don't quite understand the contents this sectioin tries to convey

1. The statement journal is for the latest

   * INSERT
   * UPDATE
   * DELETE

   SQL statenents that could

   1. Modify muliple rows
   2. Result in a constraint violation
   3. Raise an exception within a trigger

2. It is a separate, ordinary, temporary rollback journal file that resides in 
   the standard temporary directories.

   > For what the standard temporary directories are, see the section Naming 
   > Conventions.

   > And a temporary file is named as `etilqs_xx`

3. Statement journal file is not required for crash recovery operation, it is 
   **only needed for statement aborts**.

4. The books said it is an ordinary rollback journal file, and it does not
   have a header, which makes me so confused...
   

## 3.3.3 Multi-database transaction journal, the master journal (super journal)

1. We can open multiple database files within a library connection by using the
   `ATTACH` command, and a transaction involving multiple databases is allowed
   in SQLite.

   A rollback journal file will be created for a write-transaction, but this is
   ONLY for a single database, when dealing with multiple databases, multiple
   rollback journal files will be created.

   But these rollback journal files are indepedent are not aware of each other,
   transaction on a databsae can be atomically committed or rolled back, but
   you cannot do this for all the database files **together**.

   Super journal solves this problem.

2. The format of super journal is quite simple: A UTF-8 encoded string containing
   all the full paths of the rollback files, and are separated by a null character.

3. Super journal resides in the same direcotry as **the main database** file does,
   and has the stirng "-mj" appended, followed by eight randomly chosen 4-bit hex
   numbers.

4. It is a transient file, created when the transaction **attempts to commit**, and
   deleted when the commit procedure is done.

5. When a transaction involves multiple database files, the format of the rollback
   journal file will also be slightly updated:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-24%2012-13-30.png)

   
   As you can see, a "Master journal record" is appended to the end of the 
   rollback journal file.
