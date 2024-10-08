# Bootstrap

Postgres stores its metadata in its `pg_catalog.pg_xxx` tables

> For more information, refer to [Chapter 53. System Catalogs](../Ch53_System_Catalogs).

, but we have seen that to decode a relation' data, one has to read the 
`pg_attribute` table, then how Postgres handles `pg_attribute` table before it
is ready for use? i.e., how does it bootstrap?

Postgres does this by bootstrap.

> Bootstrap typically means that:
> 1. The bootloader program
> 2. In programming language, a compiler achieves bootstrap by building the 
>    compiler in this language. (of course, you need to build it in other
>    language first)
>
> I think bootstrap has both meanings here.

The system catalog are initialized in the bootstrap stage, the structure and 
initial data of these system catalogs are predefined, so that Postgres can 
handle them without needing them.

The structure of these catalogs are defined in those `pg_xxx.h` files under 
the `src/include/catalog` directory. The initial data, is defined in `pg_xxx.dat`
file.

# BKI file

To create the catalog table files and load data into the file, commands stored
in the `postgres.bki` file need to be executed. This bki file is generated using
script `genbki.perl`, the structure and data for the `postgres.bki` come from
the `pg_xxx.h` and `pg_xxx.data` files.
