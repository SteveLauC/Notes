1. Every table AM has a handler function that:

   * takes a dummy parameter

     This dummy parameter is used to prevent the handler function from being 
     called directly in SQL commands.

   * returns a pointer to `struct TableAmRoutine`

     This type basically contains everything needed by the Postgres core to use
     the AM. This pointer needs to have a `'static` lifetime, i.e., be valid
     during the lifetime of the Postgres process, so the usual way to do this
     is to define a `static` variable to store the `struct TableAmRouting` and
     returns a pointer to it.

2. If the AM wants to support Update/Index, then every tuple should have a tuple
   identifier, i.e., the `ctid` column, it it not required to have the same 
   semantic as the column in the heap AM.