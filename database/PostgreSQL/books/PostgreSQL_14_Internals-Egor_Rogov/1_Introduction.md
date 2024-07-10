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
   * pg_toast: related to TOAST
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

2. 

# 1.2 Processes and Memory
# 1.3 Clients and server protocol


