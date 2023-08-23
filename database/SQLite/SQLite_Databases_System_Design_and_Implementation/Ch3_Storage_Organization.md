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
>   * 3.3.3 Multi-database transaction journal, the master journal

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

   > Page 0 liternally means a page that does not exist.

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

   When you create a new database, beforing creating the first table, you can
   update the page size via:


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

   4. lock-type

   And these pages are sub-typed into:

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

   The VACUUM command rebuilds the database file, repacking it into a minimal 
   amount of disk space.

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

5. Format of the file header

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-18%2011-15-37.png)

   * Header String: "SQLite format 3" (including the tailing NUL) in the 
     UTF-8 format
     
     > This string can be changed via macro: `SQLITE_FILE_HEADER`

   * Page size

   * File format: write version and read version

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

     Embedded Payload is the space that is consumed by a reocrd, max embedded
     payload is the upper bound, min embedded payload is the lower bound.

     The max embedded payload is 64 bytes, so the fraction is 25. The min embedded
     payload is 32 bytes, so the fraction is 12.5.

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

     Counter for write-transactions.

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
   2. Statement journal
   3. Master journal

   > SQLite has introduced the WAL journaling scheme.
   >
   > The structure of the WAL journal file will be talked about in Section 10.17.


## 3.3.1 Rollback Journal 

2. Rollback journal

3. Statement journal
4. Master journal



