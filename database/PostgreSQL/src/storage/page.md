> Buffer page management.

# Navigation

* `src/include/storage/bufpage.h` for the related types and structure specification
  of buffer pages and disk blocks (they use the same structure)

* `src/include/storage/itemid.h` for slot (slotted-page structure) definition 
  `ItemIdData` and its getters/setters.
   
  This slot is also called `ItemId` or line pointer 

* `src/include/storage/item.h` defines `Item` (just a raw pointer) since the actual
  item can be different things depending on the page type

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
   retrieved, which is basically a type of page, all the access methods in Postgres
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

   ~~This 15-bit field also limits the maximum page size Postgres can have, `2^15`.~~

   Update: this type does not limit the page size, page size is limited by the
   `page_size_version` field in the page header, it has to be a multiple of 256,
   so that the max value is `256 * 128 = 32768 = 2^15`.

6. `PageXLogRecPtr`

   This type is represented using 2 32-bit numbers, I don't quite understand the 
   reason, the only reason that I can image is that there was no `u64` before this
   type appeared.

   TODO: ask this in the mailing list `pgsql-general`.

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

7. I don't quite understand this sentence

   > A dirty buffer cannot be dumped to disk until xlog has been flushed at 
   > least as far as the page's LSN.

   Looks like Postgres won't `fsync(2)` every WAL append.

8. `pd_prune_xid` field in the page header records the transaction ID of the
   last (image that there is a sequence of transactions that need to be cleaned
   up) transaction that hasn't been cleaned up.

   When this last is relatively new, then it means the last cleanup was done
   in the near time, and we don't need to clean it up.

   If the transaction ID is pretty old, then we need to clean up the page.

   This is just a **hint field**, i.e., it is used as a hint.

9. `pd_pagesize_version` in the page header uses an `u16` to store 2 values, which
   is due to historical reasons, there was no `version` value in this field back
   in the day, then they want to add one, so they use the lower byte to store
   the version, higher byte for page size.

   To ensure the bits of page size won't occupy the lower byte, it has to be a 
   multiple of 256, and the max multiple of 256 that can be expressed in `u16`
   is `2^15 = 32768`.

   If these 2 bytes have the following value (assume little endian machines)

   ```sh
   [0x04, 0x20]
   ```

   ```rs
   fn main() {
       let version = u8::from_ne_bytes([0x04]);
       println!("version: {}", version);

       // set the first byte to 0s, as it is the value of version
       let pagesize = u16::from_ne_bytes([0x00, 0x20]);
       println!("page size: {}", pagesize);
   }
   ```

   See the `PageGetPageSize()` function:

   ```c
   static inline Size PageGetPageSize(Page page)
   {
      return (Size) (((PageHeader) page)->pd_pagesize_version & (uint16) 0xFF00);
   }
   ```

10. `pd_flags` in the page header

    This field is a bitflag:

    * `PD_HAS_FREE_LINES` (0x001): A hint flag is set if there are available 
      line pointers before `pd_lower`, it is just a hint flag, may not be 
      accurate.

    * `PD_PAGE_FULL` (0x002): If an `UPDATE` operation does not find enough free
      space for the new version of tuple, then this flag will be set.

      > QUES: will `INSERT` operation set this flag? Or it simply uses the free
      > space map fork to choose the page.

    * `PD_ALL_VISIBLE` (0x004): all tuples on pages are available to everyone

    * `PD_VALID_FLAGS_BITS`: OR of all the above valid bits
    
11. Postgres page version history

    | Pg version | Page version | reason for bumping page version |
    |------------|--------------|---------------------------------|
    | before 7.3 |    0         |    -                            |
    | 7.3 - 7.4  |    1         |    new `HeapTupleHeader` layout |
    | 8.0        |    2         |    new `HeapTupleHeader` layout |
    | 8.1 - 8.2  |    3         |new `HeapTupleHeader` `infomask` |
    | 8.3 - now  |    4         |new `HeapTupleHeader` layout, heap header is also changed |

    > There was a `pd_tli` field in the page header in the location of the 
    > `pd_checksum` field in page version 3/4, but it no longer exists in 
    > the Postgres 9.3, replaced by `pd_checksum`.
    >
    > The only occurrence I found is: https://github.com/postgres/postgres/blob/5d6c64d290978dab76c00460ba809156874be035/src/include/storage/bufpage.h#L197
    >
    > [Commit of this change][link]
    >
    > [link]: https://git.postgresql.org/gitweb/?p=postgresql.git;a=blobdiff;f=src/include/storage/bufpage.h;h=42f8f2fa496176336dbee04f42b9aa3d018a6e40;hp=8c887cab735ed70c07da2972ef27915b078e50f2;hb=bb7cc2623f242ffafae404f8ebbb331b9a7f2b68;hpb=4c855750fc0ba9bd30fa397eafbfee354908bbca

12. The definition of `PageHeaderData` contains a flexible array member field
    `pd_linp`, which actually is not part of the page header.

    ```c
    typedef struct PageHeaderData
    {
       /* XXX LSN is member of *any* block, not only page-organized ones */
       PageXLogRecPtr pd_lsn;		/* LSN: next byte after last byte of xlog
                           * record for last change to this page */
       uint16		pd_checksum;	/* checksum */
       uint16		pd_flags;		/* flag bits, see below */
       LocationIndex pd_lower;		/* offset to start of free space */
       LocationIndex pd_upper;		/* offset to end of free space */
       LocationIndex pd_special;	/* offset to start of special space */
       uint16		pd_pagesize_version;
       TransactionId pd_prune_xid; /* oldest prunable XID, or zero if none */
       ItemIdData	pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* line pointer array */
    } PageHeaderData;
    ```

    The page size, is calculated as the offset of this field:

    ```c
    #define SizeOfPageHeaderData (offsetof(PageHeaderData, pd_linp))
    ```

13. A page will be initialized by function `PageInit()` 
    (defined in `src/backend/storage/page/bufpage.c`), which will set the `pd_upper`
    field to the `page size - special space size`.

    Postgres checks if a page is initialized by checking if this field is 0.

    So it cannot be a non-0 value before if uninitialized, i.e., the corresponding
    memory has to be zero-initialized.

14. Function `PageGetItemId()` takes a page, and the index (starting from 1) of the
    Item you want, returns the pointer to that `Item`

    ```c
    /*
    * PageGetItemId
    *		Returns an item identifier of a page.
    */
    static inline ItemId
    PageGetItemId(Page page, OffsetNumber offsetNumber)
    {
       return &((PageHeader) page)->pd_linp[offsetNumber - 1];
    }
    ```

15. Alignment

    Postgres defines the alignment of size (minimal multiple of `ALIGVAL`) by

    ```c
    #define TYPEALIGN(ALIGNVAL,LEN)  \
      (((uintptr_t) (LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t) ((ALIGNVAL) - 1)))
    ```

    `ALIGNVAL` is the align value, it **must be a power of 2** (or this won't 
    work) due to how computer transfers data.

    To find the minimal multiple of `ALIGNVAL` that is bigger than `LEN`, simply
    add `ALIGNVAL-1` to len and set all the bits represent by `ALIGNVAL-1` to 0.

    > By setting of bits of `2^n-1` to 0, we can ensure the remaining value is
    > a multiple of `2^n`. 
    >
    > Why:
    > The minimal value that has this restriction is `2^n` (by setting the lowest
    > bit to 1), this is a multiple of `2^n`. If we set the next higher bit, it
    > will be `2 * 2^n`, then you should understand, setting any bit, the value
    > will be `n * 2^n`.

    > The page size should be a multiple of `256`, so that the lower byte of field
    > `pd_page_version` is 0.

16. 