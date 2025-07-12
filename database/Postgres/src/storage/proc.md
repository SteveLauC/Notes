> * src/include/storage/proc.h: per-process shared memory data structures

1. `struct PGPROC` is a per-backend process in-memory structure to track the its 
   status, particularly, it has a `databaseId` field that is the database this
   process currently connects to:

   ```c
   Oid			databaseId;		/* OID of database this backend is using */
   ```

   A Postgres backend process, once connected to a database, cannot switch to another
   database, so this field should always have the same value during that backend 
   process's lifetime.
