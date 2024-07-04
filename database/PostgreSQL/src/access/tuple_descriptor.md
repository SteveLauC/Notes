> This chapter is about:
>
> * Before I read it
>
>   * I thought these files contain something related to storage format that is 
>     general to heap tuples and index tuples.
>
> * After I read it
>
>   * Well, they are actually not! The tuple descriptor `struct TupleDescData` type
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
>   * How is the descriptor type cached (Postgres won't read the pg_attribute 
>     table every time it is needed)
>
>     Postgres has `relchache` and `typcache`, `TupleDescData` is referenced 
>     counted.

> What have you learned from it
>
> * The in-memory structure of relation metadata
>  
>   > QUES: when reading tables from disk, these structures will be constructed
>   > from system catalogs like `pg_attriute`, `pg_attrdef`, `pg_constraint`, 
>   > when modifying the metadata recorded in the `TupleDescData`, I guess Pg
>   > will update the cached `TupleDescData` as well as the memory pages of 
>   > those system catalogs.

# Navigation

* `src/include/access/tupdesc.h`tuple descriptor definitions
* `src/include/access/tupdesc_details.h`: tuple descriptor definitions we can't include everywhere
* `src/include/access/tupdesc.c`tuple descriptor support code

# `src/include/access/tupdesc.h`

1. Tuple descriptor

   ```c
   typedef struct TupleDescData
   {			
       /* number of attributes in the tuple, also the length of the last flexible array */
       int			natts;
       /* composite type ID for tuple type */
       //
       // Postgres creates a composite type for every table, so we have this field
       // QUES: I still do not understand the following 2 fields
       Oid			tdtypeid;		
       int32		tdtypmod;		/* typmod for tuple type */
       /* reference count, or -1 if not counting */
       // 
       // Since the instance of this type will be reference-counted in the cache 
       // this field records the reference count
       int			tdrefcount;		
       /* constraints, or NULL if none */
       //
       // Constraint of this relation, transient tuples generally don't have
       // constraints, this type is designed to be efficient in that case, by
       // using a pointer.
       TupleConstr *constr;		
       /* attrs[N] is the description of Attribute Number N+1 */
       FormData_pg_attribute attrs[FLEXIBLE_ARRAY_MEMBER];
   }			TupleDescData;
   ```

   1. The most important thing of this type is the last flexible array member 
      field, which is an array of tuples from system catalog `pg_attribute`.

      This `FromData_pg_attribute` type defines the `pg_attribute` type, see 
      [75.1 System Catalog Declaration Rules][link].
        
      [link]: ../../pg16_docs/Ch75_System_Catalog_Declarations_and_Initial_Contents/75.1_System_Catalog_Declaration_Rules.md

    2. The type only records **non-system** columns

    3. When reading from disk files, Postgres construct this type by reading the
       `pg_attribute/pg_attrdef/pg_constraint` system catalogs.

    4. QUES: I don't quite understand the `tdtyeid` and `tdtypmod` fields

2. Tuple/relation constraint

   ```c
   /* This structure contains constraints of a tuple */
   typedef struct TupleConstr
   {
       AttrDefault *defval;  		/* array */
       ConstrCheck *check;			/* array */
       struct AttrMissing *missing;	/* missing attributes values, NULL if none */
       uint16		num_defval;
       uint16		num_check;
       bool		has_not_null;
       bool		has_generated_stored;
   } TupleConstr;
   ```

   `defval` is an array (all the fields) containing the default values of all the
   fields, `AttrDefault` is defined as:

   > QUES: does every field have an entry in the `defval` and `check` array, or
   > only fields that have check constraint and default value have this value.
   >
   > Answer: Postgres only stores the fields that have default or check constraint,
   > to check if a field has default value, see their `pg_attribute.atthasdef`
   > value, to see the check constraint, see `pg_constraint.conkey`.

   ```c
   typedef struct AttrDefault
   {
       AttrNumber	adnum;
       char	   *adbin;			/* nodeToString representation of expr */
   } AttrDefault;
   ```

   As you can see that the `adbin` field is a string, it is the result of 
   `nodeToString(default expression)`.

   > QUES: for expressions like `1+1`, will Postgres evaluate it to `2` and then
   > store 2 there?

   When Postgres wants to use this field, it will call `stringToNode(str)` to get
   the ast back.

3. Check Constraints

   > Postgres doc: https://www.postgresql.org/docs/current/ddl-constraints.html#DDL-CONSTRAINTS-CHECK-CONSTRAINTS 

   ```c
   typedef struct ConstrCheck
   {
       char	   *ccname;
       char	   *ccbin;			/* nodeToString representation of expr */
       bool		ccvalid;
       bool		ccnoinherit;	/* this is a non-inheritable constraint */
   } ConstrCheck;
   ```

   This type stores the check constraint of a specific field, one can specify a
   constraint like this:

   ```sql
   CREATE TABLE products (
       product_no integer,
       name text,
       price numeric CHECK (price > 0)
   );
   ```

   We can give a check a name, then this user-defined name will be stored in 
   `ccname`, or the default value `{table}_{field}_check` will be used instead.

   > You can verify the default name of the above SQL by:
   >
   > ```sql
   > SELECT
   >	 *
   > FROM
   >     pg_constraint
   > WHERE
   >     conname LIKE '%price%';
   > ```

   ```sql
   CREATE TABLE products (
       product_no integer,
       name text,
       price numeric CONSTRAINT positive_price CHECK (price > 0)
   );
   ```

   The `ccbin` field stores the `nodeToString(constraint expr)`.

   `ccvalid` is a bool flag indicating if this check has been validated or not.
   (QUES: I am not sure about the meaning of this field).

   `ccnoinherit` is true if this constraint is inherited from its parent table.

4. Why these 2 macros use `do {} while(0)`?

   ```c
   #define PinTupleDesc(tupdesc) \
    do { \
		if ((tupdesc)->tdrefcount >= 0) \
			IncrTupleDescRefCount(tupdesc); \
    } while (0)

   #define ReleaseTupleDesc(tupdesc) \
	do { \
	    if ((tupdesc)->tdrefcount >= 0) \
			DecrTupleDescRefCount(tupdesc); \
    } while (0)
   ```

# `src/include/access/tupdesc_details.h`

```c
/*
 * Structure used to represent value to be used when the attribute is not
 * present at all in a tuple, i.e. when the column was created after the tuple
 */
typedef struct AttrMissing
{
	bool		am_present;		/* true if non-NULL missing value exists */
	Datum		am_value;		/* value when attribute is missing */
} AttrMissing;
```

QUES: does every field have one instance of `AttrMissing` in `TupleConstr`, there
is no field like `num_missing` in it.


# `src/include/access/tupdesc.c`

TODO: I am gonna skip these support functions because my current focus is the 
storage format, learning all the details of metadata won't help too much I guess.