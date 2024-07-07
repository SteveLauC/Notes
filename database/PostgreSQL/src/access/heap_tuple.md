> This chapter is about:
>
> * Before I read it
>
>   * How Postgres stores **heap** tuples (data)
>
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>
>   * Heap tuple format
>   * How var-len fields are stored
>
>     1. Without TOAST
>     2. With TOAST
>
>   * How NULL is represented
>
>     Postgres uses a NULL bitmap, 1 bit for a field, unless the table does not
>     have null fields.
>
>   * Alignment requirements for different types
>   * Is padding added before data or after data
>
>     before data
>
>   * Postgres will access the metadata first, then read the data, I guess it 
>     will read the `TupleDesc`

> What have you learned from it
>
> *
> *

# Navigation

* `src/include/access/htup_details.h` heap tuple header definition (includes `htup.h`)
   Header for `src/backend/access/common/heaptuple.c`
* `src/include/access/htup.h` heap tuple definition
* `src/include/access/heapam.h`: defines heap access methods (CRUD)
* `src/backend/access/heap/heapam.c`: implements heap access methods (CRUD)
* `src/backend/access/common/heaptuple.c`: This file contains heap tuple accessor
  and mutator routines, as well as various tuple utilities.
  The prototypes of these functions are defined in `src/include/access/htup_details.h`

# `src/include/access/htup_details.h`   

1. Postgres heap tuple format

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-07-04%20at%201.40.50%20PM.png)

   * Header (23 bytes)
     * t_xmin: the ID of the transaction that inserts the tuple
     * t_xmax: the ID of the transaction that deletes (explicit delete by `DELETE`
       or inexplicit delete by `UPDATE`) or locks the tuple
     * t_cid: this field can be either `cmin` (the ID of the command that 
       inserted the tuple) or `cmax` (the ID of the command that deletes 
       the tuple). If a tuple is inserted and deleted in the same transaction,
       then this field stores a "combo" command ID that can be mapped to real
       `cmin` and `cmax`.
     * t_ctid: ctid of the tuple, or the ctid of its replacement if there is an 
       update
     * t_infomask2: flags
     * t_infomask: flags
     * t_hoff: length of (header+bitmap+padding), also the start offset of user data
   * NULL bitmap: 1 bit for each column
   * Padding
   * User data

2. Postgres heap tuple header is defined as follows

   ```c
   struct HeapTupleHeaderData
   {
       union
       {
           HeapTupleFields t_heap;
           DatumTupleFields t_datum;
       }			t_choice;

       ItemPointerData t_ctid;		/* current TID of this or newer tuple (or a
                                   * speculative insertion token) */

       /* Fields below here must match MinimalTupleData! */

   #define FIELDNO_HEAPTUPLEHEADERDATA_INFOMASK2 2
       uint16		t_infomask2;	/* number of attributes + various flags */

   #define FIELDNO_HEAPTUPLEHEADERDATA_INFOMASK 3
        uint16		t_infomask;		/* various flag bits, see below */

   #define FIELDNO_HEAPTUPLEHEADERDATA_HOFF 4
       uint8		t_hoff;			/* sizeof header incl. bitmap, padding */

       /* ^ - 23 bytes - ^ */

   #define FIELDNO_HEAPTUPLEHEADERDATA_BITS 5
       bits8		t_bits[FLEXIBLE_ARRAY_MEMBER];	/* bitmap of NULLs */

        /* MORE DATA FOLLOWS AT END OF STRUCT */
   };
   ```

   For heap tuples, the first union field will be a `HeapTupleFields`, which is
   defined as:

   ```c
   typedef struct HeapTupleFields
   {
       TransactionId t_xmin;		/* inserting xact ID */
       TransactionId t_xmax;		/* deleting or locking xact ID */

       union
       {
           CommandId	t_cid;		/* inserting or deleting command ID, or both */
           TransactionId t_xvac;	/* old-style VACUUM FULL xact ID */
       }			t_field3;
   } HeapTupleFields;
   ```

   The other value of the first union, is for [composite types][link] (also called)
   row type, 
   
   ```c
   typedef struct DatumTupleFields
   {
       int32		datum_len_;		/* varlena header (do not touch directly!) */

       int32		datum_typmod;	/* -1, or identifier of a record type */

       Oid			datum_typeid;	/* composite type OID, or RECORDOID */

       /*
       * datum_typeid cannot be a domain over composite, only plain composite,
       * even if the datum is meant as a value of a domain-over-composite type.
       * This is in line with the general principle that CoerceToDomain does not
       * change the physical representation of the base type value.
       *
       * Note: field ordering is chosen with thought that Oid might someday
       * widen to 64 bits.
       */
   } DatumTupleFields;
   ```
   
   tuples with Datum fields like this are called Datum tuple. **Every in-memory 
   tuple will be initialized to a Datum tuple.**, when a tuple is about to be 
   inserted into the page, the transaction fields will be filled instead.

   [link]: https://www.postgresql.org/docs/current/rowtypes.html

3. For tuples that are updated, its `t_ctid` field will be updated to the ctid
   of its replacement tuple.

   If this table is a partitioned table, and this update moves this tuple from
   one relation to another one, then `t_ctid` field no longer works because it
   is an identifier within a specific database file, different partitions have
   different main fork files, in this case, its `t_ctid` will be set to a special
   value to indicate that this tuple has been moved to another partition.

   > QUES: if I update a tuple `A` twice, `A -> B -> C`, will its `t_ctid` points 
   > to the latest `C`, or `B`.
   >
   > My guess is that it will be updated to points to `B`
   >
   > Future steve: yes, it won't point to `C`, to access the latest version of this
   > tuple, you need to "dereference" the ctid until it points to itself if it is
   > not moved to a different partition.
   >
   > NOTE: `VACUUM` may remove `B` before `A`, so the `t_ctid` stored in `A` is
   > dangling, be sure to check if it still exists before "dereferencing it", one
   > can do this by checking the tuple `t_xmin` and `t_xmax`, tuple `B`'s `t_xmin`
   > should have the same value as `A`'s `t_xmax`.

   Thus, a tuple is in its latest version if and only if either:

   1. its `t_tmax` is **INvalid**
   2. its `t_ctid` points to itself

4. On insertions, `t_ctid` **may** be set to an speculative insert token and will be 
   replaced by its real tid after insertion. This only happens with insertion, you 
   won't see this with updates.

   > When will this speculative token be set?

5. NULL bitmap can be not stored if there is no NULLs in the table. This can be
   checked through `bitmap_is_empty = !(t_infomask & HEAP_HASNULL)`.

6. Optional `OID` fields

   Table can be created with `WITH OIDS`, by default, this is disabled. When 
   enabled, this field is stored after at the offset specified by `t_hoff`,
   before user data, i.e., move the user data backwards and put an `OID` there.


7. Flags that can be set in `t_infomask`:

   ```c
   /*
    * information stored in t_infomask:
    */
   #define HEAP_HASNULL			0x0001	/* has null attribute(s) */
   #define HEAP_HASVARWIDTH		0x0002	/* has variable-width attribute(s) */
   #define HEAP_HASEXTERNAL		0x0004	/* has external stored attribute(s) TOAST */
   #define HEAP_HASOID_OLD			0x0008	/* has an object-id field */
   /* t_cid is a combo CID  */
   // will be set if this tuple has been inserted and removed within the same
   // transaction.
   #define HEAP_COMBOCID			0x0020	

   #define HEAP_XMAX_KEYSHR_LOCK	0x0010	/* xmax is a key-shared locker */
   #define HEAP_XMAX_EXCL_LOCK		0x0040	/* xmax is exclusive locker */

   #define HEAP_XMAX_LOCK_ONLY		0x0080	/* xmax, if valid, is only a locker */

   /* xmax is a shared locker */
   #define HEAP_XMAX_SHR_LOCK	(HEAP_XMAX_EXCL_LOCK | HEAP_XMAX_KEYSHR_LOCK)

   // QUES: `HEAP_XMAX_SHR_LOCK` already includes `HEAP_XMAX_EXCL_LOCK` and 
   // `HEAP_XMAX_KEYSHR_LOCK`, why they are both includeded in `HEAP_LOCK_MASK`?
   #define HEAP_LOCK_MASK	(HEAP_XMAX_SHR_LOCK | HEAP_XMAX_EXCL_LOCK | \
                           HEAP_XMAX_KEYSHR_LOCK)

   #define HEAP_XMIN_COMMITTED		0x0100	/* t_xmin committed */
   #define HEAP_XMIN_INVALID		0x0200	/* t_xmin invalid/aborted */
   #define HEAP_XMIN_FROZEN		(HEAP_XMIN_COMMITTED|HEAP_XMIN_INVALID)
   #define HEAP_XMAX_COMMITTED		0x0400	/* t_xmax committed */
   #define HEAP_XMAX_INVALID		0x0800	/* t_xmax invalid/aborted */
   #define HEAP_XMAX_IS_MULTI		0x1000	/* t_xmax is a MultiXactId */
   #define HEAP_UPDATED			0x2000	/* this is UPDATEd version of row */

   // QUES: are the following 3 flags still in use?
   #define HEAP_MOVED_OFF			0x4000	/* moved to another place by pre-9.0
                                           * VACUUM FULL; kept for binary
                                           * upgrade support */
   #define HEAP_MOVED_IN			0x8000	/* moved from another place by pre-9.0
                                           * VACUUM FULL; kept for binary
                                           * upgrade support */
   #define HEAP_MOVED (HEAP_MOVED_OFF | HEAP_MOVED_IN)

   #define HEAP_XACT_MASK			0xFFF0	/* visibility-related bits */
   ```

8. Flags that can be set in `t_infomask2` (uint16)

   ```c
   /*
    * information stored in t_infomask2 (uint16 0xFFFF):
    */
   /* 11 bits for number of attributes */
   // NOTE that this is a mask: 000011111111111
   // 
   // And this is actually stored in `TupleDescData.natts`, so this field should only
   // serve as a cross check.
   #define HEAP_NATTS_MASK			0x07FF	
   /* bits 0x1800 are available */
   #define HEAP_KEYS_UPDATED		0x2000	/* tuple was updated and key cols
                                             * modified, or tuple deleted */
   #define HEAP_HOT_UPDATED		0x4000	/* tuple was HOT-updated */
   #define HEAP_ONLY_TUPLE			0x8000	/* this is heap-only tuple */

   /* visibility-related bits */
   // NOTE this is also a mask for the above 3 flags
   #define HEAP2_XACT_MASK			0xE000
   ```

# `src/include/access/htup.h`

1. type `struct HeapTupleData` and `typedef HeapTupleData* HeapTuple`

   `HeapTupleData` serves as a pointer to a heap tuple, and actually, it contains
   a pointer to a heap tuple header, see the `t_data` field.
  
   ```c
   typedef struct HeapTupleData
   {
       uint32		t_len;			/* length of *t_data */
       ItemPointerData t_self;		/* SelfItemPointer */
       Oid			t_tableOid;		/* table the tuple came from */
   #define FIELDNO_HEAPTUPLEDATA_DATA 3
       HeapTupleHeader t_data;		/* -> tuple header and data */
   } HeapTupleData;
   ```
   

# `src/backend/access/heaptuple.c`

1. `varlenas` means var-len attributes

2. Before Postgres 8.3, var-len attribute's header is always 4 bytes. After 
   that, short var-len attribute (0-126 bytes) can have short 1 byte header.

   > For the first 2 bits, when the highest-order or lowest-order bit is set, 
   > the value has only a single-byte header instead of the normal four-byte 
   > header, and the remaining bits of that byte give the total datum size 
   > (including length byte) in bytes. This alternative supports space-efficient
   > storage of values shorter than 127 bytes,

   A tuple with short header is TOASTed, for the functions that do no want to
   care about the header difference (4B vs 1B), `pg_detoast_datum()` can be used
   to expand the header.

3. Insert fields to tuple

   > Postgres does not have real updates/deletes so insertion is all you need.

   * `fill_val()`
   * `heap_fill_tuple()`