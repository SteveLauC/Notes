> This chapter is about:
>
> * Before I read it
>
>   * I thought these files contain something related to storage format that is 
>     general to heap tuples and index tuples.
>
> * After I read it
>
>   * Well, it is actually not! The tuple descriptor `struct TupleDescData` type
>     defined in `src/include/access/tupdesc.h` is NOT data, but metadata of a
>     tuple, AND, I would say it is more like relation metadata (`RelationData.rd_att`).
>     The reason why it is called `TupleDescData` is probably it will be used
>     when processing tuples,

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>
>   * tuple format (This is not covered here!)
>   * Why does post
>   * How is the descriptor type cached (Postgres won't read the pg_attribute 
>     table every time it is needed)

# Navigation

* `src/include/access/tupdesc.h`tuple descriptor definitions
* `src/include/access/tupdesc.c`tuple descriptor definitions
* `src/include/access/tupdesc_details.h`: tuple descriptor definitions we can't include everywhere

# `src/include/access/tupdesc.h`

1. Tuple descriptor

   ```c
   typedef struct TupleDescData
   {
       int			natts;			/* number of attributes in the tuple */
       Oid			tdtypeid;		/* composite type ID for tuple type */
       int32		tdtypmod;		/* typmod for tuple type */
       int			tdrefcount;		/* reference count, or -1 if not counting */
       TupleConstr *constr;		/* constraints, or NULL if none */
       /* attrs[N] is the description of Attribute Number N+1 */
       FormData_pg_attribute attrs[FLEXIBLE_ARRAY_MEMBER];
   }			TupleDescData;
   ```

   The most important thing of this type is the last flexible array member field,
   which is an array of tuples from system catalog `pg_attribute`.

   This `FromData_pg_attribute` type defines the `pg_attribute` type, see 
   [75.1 System Catalog Declaration Rules][link].
   
   [link]: ../../pg16_docs/Ch75_System_Catalog_Declarations_and_Initial_Contents/75.1_System_Catalog_Declaration_Rules.md