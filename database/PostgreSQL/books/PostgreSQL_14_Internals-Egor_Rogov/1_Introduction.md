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

# 1.2 Processes and Memory
# 1.3 Clients and server protocol


