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

   `pg_database` is shared across all the databases, so system catalogs like this 
   does not belong to any specific database. A dummy database with OID 0 is used
   internally. 

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

2. A tablespace can be used by multiple databases, and a database's data can be
   stored in multiple tablespaces.

3. List all tablespaces

   ```sql
   steve=# select * from pg_tablespace;
    oid  |  spcname   | spcowner | spcacl | spcoptions 
    -----+------------+----------+--------+------------
    1663 | pg_default |       10 |        | 
    1664 | pg_global  |       10 |        | 
    (2 rows)
   ```

4. Predefined tablespaces

   * pg_default
   * pg_global

# 1.2 Processes and Memory
# 1.3 Clients and server protocol


