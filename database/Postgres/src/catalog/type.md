# Navigation

* `src/include/catalog/pg_type.h`: This files defines the schema of `pg_type` 
  system catalog, for example, `#define Anum_pg_type_oid 1` defines that field
  `oid` is the first field.

  And also the OID of various built-in types `#define BOOLOID 16`.

* `src/include/catalog/pg_type.dat`
* `src/backend/catalog/pg_type.c`