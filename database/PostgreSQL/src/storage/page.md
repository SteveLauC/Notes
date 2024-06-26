> Buffer page management.

# Navigation

* `src/include/storage/bufpage.h` for the related types and structure specification
  of buffer pages and disk blocks (they use the same structure)

* `src/include/storage/itemid.h` for slot (slotted-page structure) definition 
  `ItemIdData`

* `src/include/storage/itemptr.h` contains a type that uniquely identifies a tuple
  within the whole database file (`struct ItemPointerData`, aka, ctid), block 
  number and tuple number within the block/page.

* Checksum impl

  * `src/include/storage/checksum_impl.h` actual implementation
  * `src/include/storage/checksum.h`: expose the function needed by Postgres
  * `src/backend/storage/page/checksum.c`: includes above 2 files
  
  > The implementation is put in a C file so that users can include it.

* `src/include/storage/off.h` defines `OffsetNumber`, the index type for `ItemIdData`,
  note that this index starts with 1

# `src/include/storage/bufpage.h`

1. `ItemIdData` is called line pointer in Postgres's term (pointer to a specific
   tuple within a page).

2. `struct ItemPointerData` is a pointer that contain
   
   1. A **physical** block number 
   2. A **logical** ItemIdData index

   To access the actual tuple, you have to read the `ItemIdData` specified by the
   logical ItemIdData index, this indirection enables the tuples to be able to 
   move within its page. 

3. What is AM

   > AM-generic per-page information is kept in PageHeaderData.
   >
   > AM-specific per-page data (if any) is kept in the area marked "special
   > space"; each AM has an "opaque" structure defined somewhere that is
   > stored as the page trailer.  an access method should always
   > initialize its pages with PageInit and then set its own opaque
   > fields.

   AM means access methods here, access method defines how pages are stored and 
   retrieve, which is basically a type of page, all the access methods in Postgres
   are stored in system catalog `pg_am`:

   ```sql
   steve> select * from pg_am;
   +------+--------+----------------------+--------+
   | oid  | amname | amhandler            | amtype |
   |------+--------+----------------------+--------|
   | 2    | heap   | heap_tableam_handler | t      |
   | 403  | btree  | bthandler            | i      |
   | 405  | hash   | hashhandler          | i      |
   | 783  | gist   | gisthandler          | i      |
   | 2742 | gin    | ginhandler           | i      |
   | 4000 | spgist | spghandler           | i      |
   | 3580 | brin   | brinhandler          | i      |
   +------+--------+----------------------+--------+
   ```

   As you can see from the `amtype` field, they are either a table or an index.

   When something is described as AM-generic, it means that it can be used with
   all the access methods, for buffer page, it means that this is same no matter
   this page is a index page or a table page.

4. Postgres defines `Page` as a raw pointer

   ```c
   typedef char *Pointer;
   typedef Pointer Page;
   ```

   This makes sense, as a page is a piece of heap memory.

5. Postgres use `u16` as the `LocationIndex` type, which is the index of a byte 
   within a page, but it should be `u15`, which is limited by `ItemIdData.lp_off(15bits)`.

   This 15-bit field also limits the maximum page size Postgers can have, `2^15`.

6. `PageXLogRecPtr`

   This type is represented using 2 32-bit numbers, I don't quite understand the 
   reason, the only reason that I can image is that there was no `u64` before this
   type appeared.

   ```c
   /*
    * For historical reasons, the 64-bit LSN value is stored as two 32-bit
    * values.
    */
   typedef struct
   {
       uint32		xlogid;			/* high bits */
       uint32		xrecoff;		/* low bits */
   } PageXLogRecPtr;

   /*
    * Pointer to a location in the XLOG.  These pointers are 64 bits wide,
    * because we don't want them ever to overflow.
    */
   typedef uint64 XLogRecPtr;

   static inline XLogRecPtr PageXLogRecPtrGet(PageXLogRecPtr val)
   {
       return (uint64) val.xlogid << 32 | val.xrecoff;
   }

   #define PageXLogRecPtrSet(ptr, lsn) \
       ((ptr).xlogid = (uint32) ((lsn) >> 32), (ptr).xrecoff = (uint32) (lsn))
   ```

