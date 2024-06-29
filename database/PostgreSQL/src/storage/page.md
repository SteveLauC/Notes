> This chapter is about:
>
> * Before I read it
> * After I read it
>
> Buffer page management.

> What do you expect to learn from it
>
> * If you are new to it, ...
> * If you already knew something about it, ... (tip: think more, read less)
>
>   1. How is the page structure defined
>
>      * Page header (see `struct PageHeaderData`)
>      * line pointer array
>      * free space
>      * special space
>
>      Special space is specific to access methods, e.g., for an index page, this
>      space stores the pointers to the sibling pages.
>
>      The structure of heap tuple and index tuple are not covered here.
>
>   2. What does an empty page looks 
>      
>      An empty page only has the header initialized 
>
>   3. How Postgres add new item to the database, then to the page
>
>      Postgres first looks for a page that has enough free space for the tuple 
>      using the free space map fork, then it calls `PageAddItemExtended()`. 
>
>      For heap pages, the transaction ID of the transaction that inserts the 
>      data will be set in the Item.
>
>   4. How Postgres removes an item
>
>      Postgres does delete to heap and index pages differently:
>
>      * heap pages: QUES: I just take a guess here. Postgres will set the 
>        pointer to `LP_UNUSED`, then mark the tuple dead by setting the `t_xmax`
>        field in the heap tuple header to the transaction ID of the transaction
>        that removes this tuple.
>
>        Later, when appropriate, Postgres will compact the page.
>       
>      * index pages
>
>         ```
>         void PageIndexTupleDelete(Page page, OffsetNumber offnum);
>         void PageIndexMultiDelete(Page page, OffsetNumber *itemnos, int nitems);
>         void PageIndexTupleDeleteNoCompact(Page page, OffsetNumber offnum);
>         ```
>
>   5. How Postgres updates an item
>
>      Remove the existing one, then add a new tuple.
>
>   6. Concurrency and locks

# Navigation

* `src/include/storage/bufpage.h` for the related types and structure specification
  of buffer pages and disk blocks (they use the same structure)

* `src/backend/storage/page/bufpage.c`: implementations of the function defined in 
  `src/include/storage/bufpage.h`

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
      **line pointers** before `pd_lower`, it is just a hint flag, may not be 
      accurate.
      
      > This will be checked when calling 
      > `PageAddItemExtended(Page, Item, Size, InvalidOffsetNumber, flags)`
      > though it is simply a hint, that function will still scan the line pointer
      > array to check available slots.
      >
      > After scan, if no available slot is found, this hint will be cleared.

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

# `src/backend/storage/page/bufpage.c`

```
extern void PageTruncateLinePointerArray(Page page);
extern Size PageGetFreeSpace(Page page);
extern Size PageGetFreeSpaceForMultipleTuples(Page page, int ntups);
extern Size PageGetExactFreeSpace(Page page);
extern Size PageGetHeapFreeSpace(Page page);
extern void PageIndexTupleDelete(Page page, OffsetNumber offnum);
extern void PageIndexMultiDelete(Page page, OffsetNumber *itemnos, int nitems);
extern void PageIndexTupleDeleteNoCompact(Page page, OffsetNumber offnum);
extern bool PageIndexTupleOverwrite(Page page, OffsetNumber offnum,
									Item newtup, Size newsize);
extern char *PageSetChecksumCopy(Page page, BlockNumber blkno);
extern void PageSetChecksumInplace(Page page, BlockNumber blkno);
```

1. `void PageInit(Page page, Size pageSize, Size specialSize)`

   Initialize the page by:

   1. `MemSet()` the whole page to 0

      > This `MemSet()` is a macro rather than a function to avoid function runtime
      > overhead 

   2. setting the header fields. `pd_lsn` and `pd_checksum` won't be set in this
      function, they will be set when this page is written.

      ```c
      p->pd_flags = 0;
      p->pd_lower = SizeOfPageHeaderData;
      p->pd_upper = pageSize - specialSize;
      p->pd_special = pageSize - specialSize;
      PageSetPageSizeAndVersion(page, pageSize, PG_PAGE_LAYOUT_VERSION);
      /* p->pd_prune_xid = InvalidTransactionId;		done by above MemSet */
      ```

      > Love the last line of commented code!

   Note that the `specialSize` will be aligned to 8:

   ```c
   specialSize = MAXALIGN(specialSize);
   ```

   ```c
   /* Define as the maximum alignment requirement of any C data type. */
   #define MAXIMUM_ALIGNOF 8
   ```

   
2. `bool PageIsVerifiedExtended(Page page, BlockNumber blkno, int flags)`

   Cheaply check if page header and checksum are valid, this will be invoked
   when a page is loaded from disk, if check passed, then it will be added to
   buffer pool. Return `true` if checksum is correct (if checksum is enabled)
   and the basic header is valid.

   Postgres adds the block number when computing checksum, so this function needs
   a `blkno` argument. `flags` can be used to configure 

3. `OffsetNumber PageAddItemExtended(Page page, Item item, Size size, OffsetNumber offsetNumber, int flags)`

   > There is no lock in this function.

   Put an item in the page by:

   1. Setting a line pointer
   2. Put the data in the free space

   `Item` is simply a pointer, so we need another `size` argument to provide
   the tuple length, **this `size` argument will also be max aligned** so that
   we can adjust `pd_upper` accordingly, but the tuple size stored in `ItemIdData`.
   is not aligned.

   The `OffsetNumber` argument has to be:

   1. `InvalidOffsetNumber` (None) to ask this function to find a available pointer.
      If page hint `PD_HAS_FREE_LINES` is set, then we search the existing line
      pointers to find one that is unused and does not have storage. If not found,
      append a line pointer.
      
      Otherwise, append a line pointer.
      
   2. a number between `[FirstOffsetNumber, number_of_line_ptr_in_page + 1]`
      
      In this case, if `PAI_OVERWRITE` is set in `flags`, then we just put the 
      item in the specified `OffsetNumber`. (If the `OffsetNumber` is 
      `number_of_line_ptr_in_page + 1`, then you won't overwrite anything)
      
      Otherwise, if `OffsetNumber` is not `number_of_line_ptr_in_page + 1`, you
      move all the line pointers whose offset is greater than or equal to `OffsetNumber`
      by `sizeof(ItemIdData)`
      
      ```text
      | 1 | 2 | 3 |
      
      | 1 |   | 2 | 3 |
      ```
      
    To summarize, this function will eiter:
    
    1. Successfully add an item to the page
       1. In an unused existing slot that does not have storage, overwrite or move 
          the previous one.
       2. Append a new slot
    2. Fail
    
4. `Page PageGetTempPage(Page page)`

   Allocates an **uninitialized** temporary page that has the same size as `page`.
   
5. `Page PageGetTempPageCopy(Page page)`

   Similar to `PageGetTempPage()`, but will copy the contents of `Page` to this new
   temporary page.
   
6. `Page PageGetTempPageCopySpecial(Page page)`

   1. Create a temporary page
   2. Initialize it with the `page`'s size and special space size
   3. Copy the special space of `page` to new page
   4. return
   
7. `void PageRestoreTempPage(Page tempPage, Page oldPage)`

   Processing has been done on `tempPage`, copy the contents of `tempPage` to
   `oldPage`.
   
8. `void PageRepairFragmentation(Page page)` and 
   `static void compactify_tuples(itemIdCompact itemidbase, int nitems, Page page, bool presorted)`
   
   > `PageRepairFragmentation()` is for heap pages only, `compactify_tuples()`
   > is used in both heap and index pages.
   
   `compacitfy_tuples()` is a helper function that will be invoked by 
   `PageRepairFragmentation()`, `PageRepairFragmentation()` sequentially scans
   the line pointer to find the tuples that **are still in use**, which will be
   stored in an array of `itemIdCompact (struct itemIdCompactData)`

   ```c
   /*
    * Tuple defrag support for PageRepairFragmentation and PageIndexMultiDelete
    *
    * Steve's NOTE: this type represents a tuple that is still in use.
    */
   typedef struct itemIdCompactData
   {
       uint16		offsetindex;	/* linp array index */
       int16		itemoff;		  /* page offset of item data */
  	   uint16		alignedlen;		/* aligned length! MAXALIGN(item data len) */
   } itemIdCompactData;
   ```
   
   Then this array will be passed to `compactify_tuples()`, `compactify_tuples()`
   remove the data of dead tuples and **sort** the tuples so that they are in the
   reverse order of their line pointers. If the `presorted` parameter is `true`,
   then the tuples are alreay sorted, they just have gaps, then Postgres only moves
   the tuples.
   
   > QUES: what is the benefit of sorting the tuples?
   >
   > Answer: If the tuples are ordered, then the next time we compact the page,
   > then the `itemidbase` is highly to be ordered, then we can simply scan 
   > the the `itemidbase`, the first line pointer should point to the last tuple,
   > `memmove()` the tuple to `pd_upper - aligned_len_of_tuple`, and repeat until
   > all the line pointers have been handled. 
   >
   > This is a fairly new change, done in 2020 [commit][link]
   >
   > [link]: https://github.com/postgres/postgres/commit/19c60ad69a91f346edf66996b2cf726f594d3d2b
   >
   > Otherwise, using the approach described above can overwrite exising tuples.
   > we have to create a  temporary buffer to store all the tuples, then `memcpy()`
   > it back to the page.
   
   
   > How can tuples be unordered?
   >
   > Remove a tuple that uses a line pointer in the middle of the line pointer 
   > array, making this unused line pointer the first unused one, then insert
   > another tuple, this line pointer will be reused, you will notice that this
   > line pointer is still in the middle of array, but the data is put at the 
   > end of the free space.


9. `void PageTruncateLinePointerArray(Page page)`

   For **heap pages only**, truncate the tailing *continuous* unused line pointers
   (`LP_UNUSED`). If all the line pointers are all unused, then Postgres will 
   leave the first one there.
   
   QUES: why?
   
   ```c
   * We avoid truncating the line pointer array to 0 items, if necessary by
   * leaving behind a single remaining LP_UNUSED item.  This is a little
   * arbitrary, but it seems like a good idea to avoid leaving a PageIsEmpty()
   * page behind.
   
	 /*
	  * This is an unused line pointer that we won't be truncating
	  * away -- so there is at least one.  Set hint on page.
	  */
   ```
   
   This function's comment mentions lock:
   
   > Caller can have either an exclusive lock or a full cleanup lock on page's
   > buffer.  The page's PD_HAS_FREE_LINES hint bit will be set or unset based
   > on whether or not we leave behind any remaining LP_UNUSED items.
   
10. `Size PageGetFreeSpace(Page page)`
    
    If the free space calculated through `size = pd_upper - pd_lower` is bigger than
    `sizeof(ItemIdData)`, then return `size - sizeof(ItemIdData)`. Otherwise, 0 is
    returned.
    
11. `Size PageGetFreeSpaceForMultipleTuples(Page page, int ntups)`
    
    Similar to `PageGetFreeSpace()`, but for multiple tuples. 
    
    If the free space calculated through `size = pd_upper - pd_lower` is bigger than
    `ntups * sizeof(ItemIdData)`, then return `size - ntups * sizeof(ItemIdData)`. 
    Otherwise, 0 is returned.

12. `Size PageGetExactFreeSpace(Page page)`

    If the free space calculated through `pd_upper - pd_lower` is greater than
    0, return it.
    
    > All the above 3 functions convert `pd_upper` and `pd_lower` to `int` before
    > doing the subtraction because `pd_lower` can be greater than `pd_upper`, 
    > using `uint16` will give a number that is pretty big.
    
13. `Size PageGetHeapFreeSpace(Page page)`

14. `void PageIndexTupleDelete(Page page, OffsetNumber offnum)`

15. `void PageIndexMultiDelete(Page page, OffsetNumber *itemnos, int nitems)`

16. `void PageIndexTupleDeleteNoCompact(Page page, OffsetNumber offnum)`

17. `bool PageIndexTupleOverwrite(Page page, OffsetNumber offnum,`

18. `char * PageSetChecksumCopy(Page page, BlockNumber blkno)`

19. `void PageSetChecksumInplace(Page page, BlockNumber blkno)`

# `src/include/storage/off.h`

1. `OffsetNumber` starts from 1, 0 is considered invalid:

   ```c
   #define InvalidOffsetNumber		((OffsetNumber) 0)
   #define FirstOffsetNumber		((OffsetNumber) 1)
   #define MaxOffsetNumber			((OffsetNumber) (BLCKSZ / sizeof(ItemIdData)))
   ```

2. Checking if a `OffsetNumber` is invalid is simple:

   ```c
   /*
   * OffsetNumberIsValid
   *		True iff the offset number is valid.
   */
   #define OffsetNumberIsValid(offsetNumber) \
      ((bool) ((offsetNumber != InvalidOffsetNumber) && \
            (offsetNumber <= MaxOffsetNumber)))
   ```

   