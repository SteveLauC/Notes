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


