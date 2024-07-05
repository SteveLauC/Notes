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
>     1. Without TOAST
>     2. With TOAST
>   * How NULL is represented
>   * Alignment requirements for different types
>   * Is padding added before data or after data
>   * Postgres will access the metadata first, then read the data, I guess it 
>     will read the `TupleDesc`

> What have you learned from it
>
> *
> *

# Navigation

* `src/include/access/htup_details.h` heap tuple header definition (includes `htup.h`)
* `src/include/access/htup.h` heap tuple definition
* `src/include/access/heapam.h`: defines heap access methods (CRUD)
* `src/backend/access/heapam.c`: implements heap access methods (CRUD)

# `src/include/access/htup_details.h`   

1. Postgres heap tuple format

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-07-04%20at%201.40.50%20PM.png)

   * Header
     * t_xmin: the ID of the transaction that inserts the tuple
     * t_xmax: the ID of the transaction that deletes (explicit delete by `DELETE`
       or inexplicit delete by `UPDATE`)
     * t_cid
     * t_ctid
     * t_infomask2
     * t_infomask
     * t_hoff
   * NULL bitmap
   * Padding
   * User data

1. Postgres heap tuple header is defined as follows

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

   ```


