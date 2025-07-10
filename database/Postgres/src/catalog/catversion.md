file: `src/include/catalog/catversion.h`

Postgres system catalogs has versions, which is expressed using a version 
number: `CATALOG_VERSION_NO`. Every time when an incompatible change is
made to system catalogs, this number gets updated.

This is mainly used to prevent running a Postgres binary (stored in `catversion.h`) 
in a data directory that is not compatible with it (stored in `$PGDATA/gloabl/pg_control`).