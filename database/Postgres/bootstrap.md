# How Postgres bootstraps

1. Compile the header files of the bootstrap system catalogs and generate a 
   "postgres.bki" script file

   > bootstrap catalogs are catalogs that are marked `BKI_BOOTSTRAP`
   >
   > BKI stands for "backend interface"

   * pg_type
   * pg_attribute
   * pg_proc
   * pg_class
   
   The "postgres.bki" script contians dummy SQL scripts that can be recognized by
   the postgres process (`$ postgres --boot`)
   
   
2. `initdb` will fork a `postgres` process and run it in "bootstrap" mode to
   create "template1", then it runs `postgres` in single-user mode to create
   databases "template0" and "postgres".
   