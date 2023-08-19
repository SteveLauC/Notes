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

2. Limit on the page size

   The page size must be:

   1. A power of 2
   2. in range [512(2^9), 65536(2^16)]

   The lower bound is for performance reason, the upper bound exists because 
   SQLite stores page size in a `u16`.

3. Maximux database file size

   A database file can have at most 2^31 - 1 pages(i32), this pretty big, so
   actually, the limit will be imposed by the underlying file system.

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
   2. A B+Tree internal page storing the root page of `sqlite_master` and 
      `sqlite_temp_master`

   3. Reserved space

2. The first page, and tables `sqlite_master` and `sqlite_temp_master` are the 
   metadata that SQLite uses.

3. Format of the file header

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

   * 
