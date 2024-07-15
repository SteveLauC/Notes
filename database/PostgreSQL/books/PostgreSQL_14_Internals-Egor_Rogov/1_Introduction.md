> This chapter is about:
>
> * Before I read it
>   * A general introduction to various aspects of Postgres
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>
>   * I am not quite familiar with the free space map and visibility map, perhaps
>     this chapter can give me more insights
>     * their functionality
>     * general format
>
>   * Perhaps the format of TOAST
>
>   * Postgres process architecture

> What have you learned from it
>
> *
> *


> * 1.1 Data Organization
>   * Databases
>   * System Catalogs
>   * Schemas
>   * Tablespaces
>   * Relations 
>   * Files and Forks
>   * Pages
>   * TOAST
> * 1.2 Processes and memory
> * 1.3 Clients and server protocol

# 1.1 Data Organization

## Databases

1. A fresh Postgres installation comes with 3 databases:

   > [Template databases](https://www.postgresql.org/docs/current/manage-ag-templatedbs.html)

   1. `template0`

      Is used for:

      1. restoring data from logical backup
      2. creating a database with a different encoding  

   2. `template1`

      Serves as a template for all the databases users can create.

      Whenever you create a database, it is copied from `template1`.

   3. `postgres` 
       
      A regular database that users can use. 

## System Catalog

1. One can create tables with name prefix `pg_` because system catalogs are under
   schema `pg_catalog`.

2. Which database do these system catalog belong to?

   `pg_class` contains the tables/indexes of the current database, so every 
   database has it.

   `pg_database` and `pg_tablespace` are shared across all the databases 
   (cluster-level metadata), so system catalogs like this does not belong to 
   any specific database. A dummy database with OID 0 is used internally. 

3. All the system catalogs use an `OID` as the primary key. And `OID` with the same
   value can appear in different system catalogs, Postgres ensures that the OIDs
   used in a system catalog won't be duplicate by maintaining an unique index, if
   an OID already exists in the table, increment it by 1 and check again...

   The OID counter will be reset when it reaches it maximum value.

## Schema

1. To list all the schemas you have access to:

   ```sql
   steve=# \dnS+
                                                   List of schemas
           Name        |       Owner       |           Access privileges            |           Description            
   --------------------+-------------------+----------------------------------------+----------------------------------
   information_schema  | steve             | steve=UC/steve                        +| 
                       |                   | =U/steve                               | 
   pg_catalog          | steve             | steve=UC/steve                        +| system catalog schema
                       |                   | =U/steve                               | 
   pg_toast            | steve             |                                        | reserved schema for TOAST tables
   public              | pg_database_owner | pg_database_owner=UC/pg_database_owner+| standard public schema
                       |                   | =U/pg_database_owner                   | 
   (4 rows)

   steve=# select * from pg_namespace;
   oid  |      nspname       | nspowner |                            nspacl                             
   -----+--------------------+----------+---------------------------------------------------------------
     99 | pg_toast           |       10 | 
     11 | pg_catalog         |       10 | {steve=UC/steve,=U/steve}
   2200 | public             |     6171 | {pg_database_owner=UC/pg_database_owner,=U/pg_database_owner}
   14226| information_schema |       10 | {steve=UC/steve,=U/steve}
   (4 rows)
   ```

2. Predefined schemas

   * public: the default schema for users
   * pg_catalog: for system catalogs
   * information_schema: provides an alternative view to system catalogs in the SQL standard way
   * pg_toast: TOAST tables reside under this schema
   * pg_temp: create temporary stuff

     > QUES: As you can see, schema `pg_temp` does not appear in the above 
     > query result, I guess this is because that I haven't crate any temporary
     > stuff.
     >
     > Well, I created a temporary table and it still does not appear.

## Tablespaces

1. What is tablespace

   A tablespace is basically a directory in the file system, which defines the 
   data's physical layout.

2. A tablespace can be used by multiple databases, e.g., the default tablespace
   `pg_default` (if not changed), is used by database `template1`, and when 
   creating a new database, if not explicitly specified, it uses the tablespace
   used by `template1`, so you can say that most databases will use `pg_default`.

   And a database's data can be stored in multiple tablespaces, true, even though
   a database have only 1 tablespace attribute, we can specify tablespaces for its
   every table.

   > QUES(Solved): then what is the point of setting tablespace for database.
   >
   > ANSWER: the tablespace set for a database will be the default tablespace for
   > its objects. And when an object uses its database's tablespace, it 
   > `reltablespace` will be 0 in `pg_class`.

3. List all tablespaces

   ```sql
   steve=# select * from pg_tablespace;
    oid  |  spcname   | spcowner | spcacl | spcoptions 
    -----+------------+----------+--------+------------
    1663 | pg_default |       10 |        | 
    1664 | pg_global  |       10 |        | 
    (2 rows)

   steve=# \db
         List of tablespaces
       Name    | Owner | Location 
    -----------+-------+----------
    pg_default | steve | 
    pg_global  | steve | 
    (2 rows)
   ```

4. Predefined tablespaces

   * pg_default

     Located at `$PGDATA/base`, it is the default tablespace used for all the
     operations unless another tablespace is explicitly specified.

   * pg_global
     
     Located at `$PGDATA/global`, this is used for stuff that is shared across
     the cluster, e.g., the `pg_database/pg_tablespace` system catalog.

     ```sql
     SELECT
        oid,
        relname
     FROM
        pg_class
     WHERE
        reltablespace = (
           SELECT
              oid
           FROM
              pg_tablespace
           WHERE
              spcname = 'pg_global')
        ORDER BY
           oid;
     1213	pg_tablespace
     1214	pg_shdepend
     1232	pg_shdepend_depender_index
     1233	pg_shdepend_reference_index
     1260	pg_authid
     1261	pg_auth_members
     1262	pg_database
     2396	pg_shdescription
     2397	pg_shdescription_o_c_index
     2671	pg_database_datname_index
     2672	pg_database_oid_index
     2676	pg_authid_rolname_index
     2677	pg_authid_oid_index
     2694	pg_auth_members_role_member_index
     2695	pg_auth_members_member_role_index
     2697	pg_tablespace_oid_index
     2698	pg_tablespace_spcname_index
     2846	pg_toast_2396
     2847	pg_toast_2396_index
     2964	pg_db_role_setting
     2965	pg_db_role_setting_databaseid_rol_index
     2966	pg_toast_2964
     2967	pg_toast_2964_index
     3592	pg_shseclabel
     3593	pg_shseclabel_object_index
     4060	pg_toast_3592
     4061	pg_toast_3592_index
     4175	pg_toast_1260
     4176	pg_toast_1260_index
     4177	pg_toast_1262
     4178	pg_toast_1262_index
     4181	pg_toast_6000
     4182	pg_toast_6000_index
     4183	pg_toast_6100
     4184	pg_toast_6100_index
     4185	pg_toast_1213
     4186	pg_toast_1213_index
     6000	pg_replication_origin
     6001	pg_replication_origin_roiident_index
     6002	pg_replication_origin_roname_index
     6100	pg_subscription
     6114	pg_subscription_oid_index
     6115	pg_subscription_subname_index
     6243	pg_parameter_acl
     6244	pg_toast_6243
     6245	pg_toast_6243_index
     6246	pg_parameter_acl_parname_index
     6247	pg_parameter_acl_oid_index
     6302	pg_auth_members_grantor_index
     6303	pg_auth_members_oid_index
     ```

     > Question: the file layout under `$PGDATA/base` is that the first layer
     > of directories are databases:
     >
     > ```
     > steve=# SELECT
     >         oid,
     >         datname
     > FROM
     >         pg_database
     > ORDER BY
     >         oid;
     > oid  |  datname  
     > -------+-----------
     >     1 | template1
     >     4 | template0
     >     5 | postgres
     > 16384 | steve
     > (4 rows)
     > ```
     >
     > ```sh
     > $ l base
     > Permissions Links Size User  Group Date Modified Name
     > drwx------@     1    - steve steve  4 Jul 09:54  1
     > drwx------@     1    - steve steve  3 Jul 08:13  4
     > drwx------@     1    - steve steve  4 Jul 09:54  5
     > drwx------@     1    - steve steve  8 Jul 13:46  16384
     > ```
     >
     > Currently, there is no `database` directory that is under tablespace `pg_global` in
     > my environment:
     >
     > Future steve: true, all the tablespaces except for `pg_global` has database
     > dirs.
     >
     > ```sql
     > SELECT
     >     count(*)
     > FROM
     >     pg_class
     > WHERE
     >     reltablespace = (
     >         SELECT
     >             oid
     >         FROM
     >             pg_tablespace
     >         WHERE
     >             spcname = 'pg_global');
     >  count
     >  -------
     >  50
     >  (1 row)
     > ```
     > And there is exactly 50 main fork files under my `global` directory.

5. To create a tablespace, do this:

   ```sql
   CREATE TABLESPACE test_tablespace OWNER steve LOCATION '/home/steve/Desktop/pg_tablespace';
   ```

   > Location must be a absolute path.
   >
   > ```sql
   > steve=# CREATE TABLESPACE test_tablespace2 OWNER steve LOCATION 'foo';
   > ERROR:  tablespace location must be an absolute path
   > ```

   Postgres will create a file under the specified location:
   
   ```sh
   $ l /home/steve/Desktop/pg_tablespace
   Permissions Links Size User  Group Date Modified Name
   drwx------@     1    - steve steve  9 Jul 12:50  PG_17_202406171
   ```

   And, a symlink will be created under `$PGDATA/pg_tblspc`:

   ```sql
   steve=# select * from pg_tablespace;
   oid  |     spcname     | spcowner | spcacl | spcoptions 
   -----+-----------------+----------+--------+------------
   1663 | pg_default      |       10 |        | 
   1664 | pg_global       |       10 |        | 
   24680| test_tablespace |       10 |        | 
   (3 rows)
   ```

   ```sh
   $ l pg_tblspc                        
   Permissions Links Size User  Group Date Modified Name
   lrwxrwxrwx@     1   33 steve steve  9 Jul 12:50  24680 -> /home/steve/Desktop/pg_tablespace
   ```

## Relations

1. Postgres sees everything that has columnar structure as Relations:

   * table
   * index
   * materialized view
   * sequences (one row table)

   > This is affected by the original author Michael Stonebraker.

   All these things are stored in table `pg_class`, which was originally called
   `pg_relation`, but the column names still have prefix `rel`.

## Files and Forks

1. As we already know, a fork is a file, whose name is an `OID` followed by an
   optional suffix indicating the fork type:

   * None: main fork
   * Some(fsm): free space map fork
   * Some(vm): visibility map fork

   When the file exceed maximum size limit (1B by default, decided by `BLCKSZ`
   and `RELSEG_SIZE`), a new file (called segment) will be created, the segment
   number (starting from 1, number 0 will not be shown) will be added to the 
   end of the file name (x_fsm.1)

2. Various Postgres forks

   1. The main fork
   
      the actual data, this fork is available for every relations:

      * table
      * index
      * materialized view
      * sequences

      except for views, they contain no data.

   2. The initialization fork

      > With suffix `_init`

      It is a dummy/empty file that is used to "restore" unlogged tables after crash.

      ```sh
      $ l *_init
      Permissions Links Size User  Group Date Modified Name
      .rw-------@     1    0 steve steve 11 Jul 10:43  24692_init
      ```

      > QUES: I don't quite understand what is the point of keeping an empty file,
      > if we are clear that it is empty, why not just truncate the main fork after
      > crash.

   3. The free space map

      Used to record the free space availability for each page, available for
      table and index.

      It makes sense to maintain a fsm fork for table since Postgres uses heap
      tables. When inserting a new tuple, we can blindly put it in any page that
      has available space.

      But for index, B+Tree is sorted, you have to maintain the state, you cannot
      find a page, and insert the entry there. So Postgres only uses fsm to keep
      track of allocated (allocated file) pages that are totally empty, so that
      when adding new page, it can reuse the existing empty page rather than 
      allocating a new one.

      Free space map is created lazily (when needed), the easily way to create it
      is to invoke the `VACUUM` on the table:

      ```sh
      $ l 16389*
      Permissions Links Size User  Group Date Modified Name
      .rw-------@     1 8.2k steve steve  7 Jul 18:02  16389

      $ psql -c 'vacuum students'
      VACUUM

      $ l 16389*
      Permissions Links Size User  Group Date Modified Name
      .rw-------@     1 8.2k steve steve  7 Jul 18:02  16389
      .rw-------@     1  25k steve steve 11 Jul 11:15  16389_fsm
      .rw-------@     1 8.2k steve steve 11 Jul 11:15  16389_vm
      ```

   4. The visibility map

      This fork is used to speed up the `VACUUM` process, skip the pages that do
      not need `VACUUM` at all. Provided for table only.

      This map stores 2 bits for every page in the table main fork, the first bit
      is set if this page only contain up-to-date tuples. The second bit is set
      if all the tuples in this page are dead.

      If the first bit of a page is set, the `VACUUM` does not need to clean it.

## Pages

## Toast

1. If a table has columns that can potentially use TOAST, then a TOAST table will
   be created for it. No matter if it is needed or not.
   
   > QUES: Is this true?
   >
   > Future steve: yes
   
   ```sql
   steve=# CREATE TABLE could_need_toast (
           bin text
   )
   
   steve=# WITH table_oid AS (
           SELECT
                   'pg_toast_' || cast(oid AS text) || '%' AS condition
           FROM
                   pg_class
           WHERE
                   relname = 'could_need_toast'
   )
   SELECT
           relname
   FROM
           pg_class,
           table_oid
   WHERE
           relname LIKE condition
           AND relnamespace = (
                   SELECT
                           oid
                   FROM
                           pg_namespace
                   WHERE
                           nspname = 'pg_toast');
          relname        
   ----------------------
    pg_toast_24695
    pg_toast_24695_index
   (2 rows)
   ```

2. TOAST for index only supports compression.

3. The TOAST strategy used for a column depends on its type, use `\d+ <table>`
   to see the details. 

   Or you can take a look at the `attstorage` field from the `pg_attribute` table:

   ```sql
   steve=# SELECT
         attname,
         attstorage
   FROM
         pg_attribute
   WHERE
         attrelid = (
                  SELECT
                           oid
                  FROM
                           pg_class
                  WHERE
                           relname = 'students')
                  AND attnum > 0;
   attname | attstorage 
   --------+------------
   it      | p
   name    | x
   (2 rows)

   steve=# \d+ students;
                                                   Table "public.students"
   Column |         Type          | Collation | Nullable | Default | Storage  | Compression | Stats target | Description 
   -------+-----------------------+-----------+----------+---------+----------+-------------+--------------+-------------
   it     | integer               |           |          |         | plain    |             |              | 
   name   | character varying(10) |           |          |         | extended |             |              | 
   Access method: heap
   ```

   TOAST strategies listed below:

   > QUES: How Postgres decides the strategy?

   * plain 
   
     means that TOAST is not used (this strategy is applied to data types that 
     are known to be “short,” such as the integer type).

   * extended 
   
     allows both compressing attributes and storing them in a separate TOAST 
     table. 
   
   * external 
   
     implies that long attributes are stored in the TOAST table in an uncompressed
     state.

   * main 
      
     Requires that long attributes to be compressed first, they will be moved to
     TOAST table if compression does not help.

4. Postgres TOAST algorithm

   > source code: `src/backend/access/heap/heaptoast.c` `heap_toast_insert_or_update()`

   > How postgres decides if an attribute should be compressed and moved to a 
   > separate TOAST table.

   > The threshold that triggers this algorithm is that a row's length exceeds
   > `toast_tuple_target`, which by default is 2000 bytes.
   >
   > Postgres wants to have at least 4 tuples in a page (default 819)

   If a row's length is bigger than `toast_tuple_target`, do the following steps,
   every step is a while loop, whose condition is to check if this row is smaller
   than `toast_tuple_target`, after running a loop procedure, check if the loop 
   condition is satisfied, if yes, stop. So this algorithm stops as soon as possible.

   1. Take a look at attributes whose strategy is `extended` or `external`, starting
      from the largest var-len one, if its strategy is `extended`, compress it, 
      if its strategy is `external`, do nothing. Then check this attribute's size,
      if itself (the attribute rather than the row) is bigger than `toast_tuple_target`, 
      move it to TOAST table.
      
      > 先抓大头儿
      
      This loop exits either because:
      
      1. There is no extended or external attributes to handle
      2. Row size is smaller than `toast_tuple_target`
      
   2. If row size is still bigger than  `toast_tuple_target`, try step 2.
   
      Iterate over the var-len attributes (they should all be smaller than 
      `toast_tuple_target`), blindly move it to TOAST table.
      
      > 既然 extended 和 external 的大头儿抓完没用，那就全抓!
      
      This loop exits either because:
      
      1. Extened and external attribute have all been removed from the row
      2. Row size is smaller than `toast_tuple_target`
      
   3. If row size is still bigger than  `toast_tuple_target`, try step 3.
   
      > 变长字段里，extended 和 external 都杀光了，接下来搞 main 的，先压缩它们 
      
      Iterate over the attributes with the main strategy, compress them.
      
      This loop exits either because:
      
      1. All the main attributes have been compressed
      2. Row size is smaller than `toast_tuple_target`
      
   4. If row size is still bigger than  `toast_tuple_target`, try step 4.
   
      > 压缩不够了，杀！
   
      Iterate over the attributes with the main strategy, move them to TOAST 
      table.
      
      This loop exits either because:
      
      1. All the main attributes have been moved
      2. Row size is smaller than `toast_tuple_target`
      
   After running all the 4 steps, it is still possible that the row size is bigger
   than `toast_tuple_target`, if so, accept it.
   
5. The TOAST threshold `toast_tuple_target` is 2000, which can be changed at the
   table level.
   
   Get it:
   
   ```sql
   steve=# SELECT
           pg_options_to_table(reloptions)
   FROM
           pg_class
   WHERE
           relname = 'students';
    pg_options_to_table 
   ---------------------
   (0 rows)
   ```
   
   0 rows because all the options are using the default value.
   
6. The storage strategy for an attribute can be changed via `alter table alter column`,
   e.g., when you don't want an attribute to be compressed, you should change the
   storage strategy to `external`:
   
   ```sql
   steve=# ALTER TABLE students ALTER COLUMN name SET STORAGE EXTERNAL;
   
   steve=# SELECT
           attname,
           attstorage
   FROM
           pg_attribute
   WHERE
           attrelid = (
                   SELECT
                           oid
                   FROM
                           pg_class
                   WHERE
                           relname = 'students')
                   AND attnum > 0;
    attname | attstorage 
   ---------+------------
    it      | p
    name    | e
   (2 rows)
   ```
   
7. TOAST tables are under schema/namespace `pg_toast`, they are named under 
   `pg_toast_{tableoid}` or `pg_toast_{tableoid}_index`

   ```sql
   steve=# SELECT
           relname
   FROM
           pg_class
   WHERE
           relnamespace = (
                   SELECT
                           oid
                   FROM
                           pg_namespace
                   WHERE
                           nspname = 'pg_toast')
           LIMIT 10;
          relname       
   ---------------------
    pg_toast_1255
    pg_toast_1255_index
    pg_toast_1247
    pg_toast_1247_index
    pg_toast_2604
    pg_toast_2604_index
    pg_toast_2606
    pg_toast_2606_index
    pg_toast_2612
    pg_toast_2612_index
   (10 rows)
   ```
   
   Schema `pg_toast` is not included in search path, so TOAST tables are usually 
   hidden:
   
   ```sql
   steve=# SELECT
           *       
   FROM
           pg_toast_1255;
   ERROR:  relation "pg_toast_1255" does not exist
   LINE 4:  pg_toast_1255;
            ^
   steve=# SELECT
           COUNT(*)
   FROM
           pg_toast.pg_toast_1255;
    count 
   -------
        3
   (1 row)
   ```
   
   

# 1.2 Processes and Memory
# 1.3 Clients and server protocol


