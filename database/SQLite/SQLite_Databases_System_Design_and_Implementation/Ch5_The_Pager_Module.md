> This chapter is about:
>
> * what a page cache is, why it is needed, and who uses it
>  
>   > The tree module uses it
> 
> * generic cache management techniques
> * the normal transaction processing and recovery processing steps that are 
>   carried out by SQLite

> * 5.1 The Pager Module
> * 5.2 Pager Interface
>   * 5.2.1 Pager-client interaction protocol
>   * 5.2.2 The pager interface structure
>   * 5.2.3 The pager interface functions
> * 5.3 Page Cache
>   * 5.3.1 Cache state
>   * 5.3.2 Cache organizatioin
>   * 5.3.3 Cache read
>   * 5.3.4 Cache update
>   * 5.3.5 Cache fetch policy
>   * 5.3.6 Cache management
>     * 5.3.6.1 Cache replacement
>     * 5.3.6.2 LRU cache replacement schema
>     * 5.3.6.3 SQLite's cache replacement schema
> * 5.4 Transaction Management
>   * 5.4.1 Normal processing
>     * 5.4.1.1 Read operation
>     * 5.4.1.2 Write operation
>     * 5.4.1.3 Cache flush
>     * 5.4.1.4 Commit operation
>     * 5.4.1.5 Statement operations
>     * 5.4.1.6 Setting up savepoints
>     * 5.4.1.7 Releasing savepoints
>   * 5.4.2 Recovery processing
>     * 5.4.2.1 Transaction abort
>     * 5.4.2.2 Statement subtransaction abort
>     * 5.4.2.3 Reverting to savepoints
>     * 5.4.2.4 Recovery from failure
>   * 5.4.3 Other management issues
>     * 5.4.3.1 Checkpoint
>     * 5.4.3.2 Space constraint


# 5.1 The Pager Module

1. SQLite is a disk-oriented databse system that stores data on the disk, but
   it cannot manipulate the data on the disk efficiently, it has to load the
   data to the memory, and do changes there.

   That memory is called database cache or data buffer, in SQLite's term, it is
   called the *page cache*.

   The pager module is the manager of that page cache.

2. SQLite maintains a separate page cache for each open database file (aka, 
   database connection)

3. The pager module of SQLite is responsible for:

   1. Managing the page cache
      1. Load pages
      2. Flush pages
   2. Managing transactions
      1. Manage locks
      2. Manage log record

   > It is kinda weird to see that transactions are managed by the pager module.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-10-31%2015-52-20.png)

   > The book says the pager module is also a log manager, it is talking about
   > the journal file/log record.

# 5.2 Pager Interface
## 5.2.1 Pager-client interaction protocol

1. Everything above the pager module is isolated from the underlying low-level
   lock and log management mechanisms.

   The tree module sees things in term of transactions, and is not concerned
   with how the transactional ACID properties that are implemented by the pager
   module.

2. Modules that use the Pager module will request a page by giving a page number,
   then the pager module will return a pointer to the page cache of the requested
   page.

   > QUES: How does SQLite guarantee that the returned pointer will never be a
   > dangling pointer
   >
   > By manually recording the pages that are in use? Pin them?
   >
   > Future Steve: Yes, SQLite will pin that page.

3. Before modifying a page, the tree module will inform the pager so that the pager
   module can save sufficient information (in a journal file) for possible future 
   use, and can acquire appropriate locks on the database file.

   > In the last chaper, we talked about tha before SQLite modifies its pages,
   > it will first write the log (journal file), then ensure the log is persisted
   > to the disk.
   >
   > This is consistent with that.

   > The pager module is the lock manager

   The tree module eventually notifies the pager when it (the tree module) has
   finished using a page.

   > To release the guard?

## 5.2.2 The pager interface structure

1. Every open database file will be assigned a `Pager` object
 
   ```c
   struct Pager {
     sqlite3_vfs *pVfs;          /* OS functions to use for IO */
     u8 exclusiveMode;           /* Boolean. True if locking_mode==EXCLUSIVE */
     u8 journalMode;             /* One of the PAGER_JOURNALMODE_* values */
     u8 useJournal;              /* Use a rollback journal on this file */
     u8 noSync;                  /* Do not sync the journal if true */
     u8 fullSync;                /* Do extra syncs of the journal for robustness */
     u8 extraSync;               /* sync directory after journal delete */
     u8 syncFlags;               /* SYNC_NORMAL or SYNC_FULL otherwise */
     u8 walSyncFlags;            /* See description above */
     u8 tempFile;                /* zFilename is a temporary or immutable file */
     u8 noLock;                  /* Do not lock (except in WAL mode) */
     u8 readOnly;                /* True for a read-only database */
     u8 memDb;                   /* True to inhibit all file I/O */
     u8 memVfs;                  /* VFS-implemented memory database */

     /**************************************************************************
     ** The following block contains those class members that change during
     ** routine operation.  Class members not in this block are either fixed
     ** when the pager is first created or else only change when there is a
     ** significant mode change (such as changing the page_size, locking_mode,
     ** or the journal_mode).  From another view, these class members describe
     ** the "state" of the pager, while other class members describe the
     ** "configuration" of the pager.
     */
     u8 eState;                  /* Pager state (OPEN, READER, WRITER_LOCKED..) */
     u8 eLock;                   /* Current lock held on database file */
     u8 changeCountDone;         /* Set after incrementing the change-counter */
     u8 setSuper;                /* Super-jrnl name is written into jrnl */
     u8 doNotSpill;              /* Do not spill the cache when non-zero */
     u8 subjInMemory;            /* True to use in-memory sub-journals */
     u8 bUseFetch;               /* True to use xFetch() */
     u8 hasHeldSharedLock;       /* True if a shared lock has ever been held */
     Pgno dbSize;                /* Number of pages in the database */
     Pgno dbOrigSize;            /* dbSize before the current transaction */
     Pgno dbFileSize;            /* Number of pages in the database file */
     Pgno dbHintSize;            /* Value passed to FCNTL_SIZE_HINT call */
     int errCode;                /* One of several kinds of errors */
     int nRec;                   /* Pages journalled since last j-header written */
     u32 cksumInit;              /* Quasi-random value added to every checksum */
     u32 nSubRec;                /* Number of records written to sub-journal */
     Bitvec *pInJournal;         /* One bit for each page in the database file */
     sqlite3_file *fd;           /* File descriptor for database */
     sqlite3_file *jfd;          /* File descriptor for main journal */
     sqlite3_file *sjfd;         /* File descriptor for sub-journal */
     i64 journalOff;             /* Current write offset in the journal file */
     i64 journalHdr;             /* Byte offset to previous journal header */
     sqlite3_backup *pBackup;    /* Pointer to list of ongoing backup processes */
     PagerSavepoint *aSavepoint; /* Array of active savepoints */
     int nSavepoint;             /* Number of elements in aSavepoint[] */
     u32 iDataVersion;           /* Changes whenever database content changes */
     char dbFileVers[16];        /* Changes whenever database file changes */

     int nMmapOut;               /* Number of mmap pages currently outstanding */
     sqlite3_int64 szMmap;       /* Desired maximum mmap size */
     PgHdr *pMmapFreelist;       /* List of free mmap page headers (pDirty) */
     /*
     ** End of the routinely-changing class members
     ***************************************************************************/

     u16 nExtra;                 /* Add this many bytes to each in-memory page */
     i16 nReserve;               /* Number of unused bytes at end of each page */
     u32 vfsFlags;               /* Flags for sqlite3_vfs.xOpen() */
     u32 sectorSize;             /* Assumed sector size during rollback */
     Pgno mxPgno;                /* Maximum allowed size of the database */
     Pgno lckPgno;               /* Page number for the locking page */
     i64 pageSize;               /* Number of bytes in a page */
     i64 journalSizeLimit;       /* Size limit for persistent journal files */
     char *zFilename;            /* Name of the database file */
     char *zJournal;             /* Name of the journal file */
     int (*xBusyHandler)(void*); /* Function to call when busy */
     void *pBusyHandlerArg;      /* Context argument for xBusyHandler */
     int aStat[4];               /* Total cache hits, misses, writes, spills */
   #ifdef SQLITE_TEST
     int nRead;                  /* Database pages read */
   #endif
     void (*xReiniter)(DbPage*); /* Call this routine when reloading pages */
     int (*xGet)(Pager*,Pgno,DbPage**,int); /* Routine to fetch a patch */
     char *pTmpSpace;            /* Pager.pageSize bytes of space for tmp use */
     PCache *pPCache;            /* Pointer to page cache object */
   #ifndef SQLITE_OMIT_WAL
     Wal *pWal;                  /* Write-ahead log used by "journal_mode=wal" */
     char *zWal;                 /* File name for write-ahead log */
   #endif
   };
   ```

2. A process can have more than one pagers for a single database file, one for
   each connection, these pagers are isolated.

   However, in shard cache mode of operation, there will be ONLY one pager object
   even though there are multiple connections to a database file.

## 5.2.3 The pager interface functions

1. The pager module implements a set of interface functions, some important
   functions are briefly discussed below:

   > All these functions are defined in the `pager.c` file

   1. `sqlite3PagerOpen()`

      ```c
      int sqlite3PagerOpen(
        sqlite3_vfs *pVfs,       /* The virtual file system to use */
        Pager **ppPager,         /* OUT: Return the Pager structure here */
        const char *zFilename,   /* Name of the database file to open */
        int nExtra,              /* Extra bytes append to each in-memory page */
        int flags,               /* flags controlling this file */
        int vfsFlags,            /* flags passed through to sqlite3_vfs.xOpen() */
        void (*xReinit)(DbPage*) /* Function to reinitialize pages */
      )
      ```

      Creates a new `Pager` object

   2. `sqlite3PagerClose()`

      ```c
      // Every database connection will have an instance of `sqlite3`
      int sqlite3PagerClose(Pager *pPager, sqlite3 *db)
      ```

      Destroy a `Pager` and remove it from the associated connection. If the 
      associated file is a temporary file, it will also be deleted from the
      file system.

      > Well, you can close a pager when a transaction is running on it, this
      > will make the transaction abort immediately and aborts all the changes.

   3. `sqlite3PagerGet()`

      Ask a page, return a pointer to the starting address of that page.

      If the requested page exists and is not in the memory, then it will be loaded
      from the disk. If the page does not actually exist on the disk, then an empty
      page will be created in the page cache.

      ```c
      /* Dispatch all page fetch requests to the appropriate getter method.
      */
      int sqlite3PagerGet(
        Pager *pPager,      /* The pager open on the database file */
        Pgno pgno,          /* Page number to fetch */
        DbPage **ppPage,    /* Write a pointer to the page here */
        int flags           /* PAGER_GET_XXX flags */
      )
      ```

      For lock management, this function will make the Pager module obtain a
      shared lock on the database file.

   4. `sqlite3PagerWrite()`

      When the caller call `sqlite3PagerGet()`, it already gets a pointer to the
      in-memory page, which means that the caller "can" write to it.

      But the caller has to call `sqlitePagerWrite()` first so that the Pager 
      module can be aware of this. (Mark the page dirty)

      This function will try to acquire a `reserved lock` on the database file 
      and create a journal file, contents of the original page will also be
      written to the journal file (a log record)

      ```c
      // The argument of this function is that page pointer rather than a page 
      // number
      int sqlite3PagerWrite(PgHdr *pPg)
      ```

   5. `sqlite3PagerLookup()`

      Request a page and return a pointer to the in-memory copy.

      Different from `sqlite3PagerGet()`, it will return `NULL` when the requested
      page is not in the page cache.

      ```c
      DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno)
      ```

   6. `sqlite3PagerRef()`

      Every page in the page cahche has a header:

      ```c
      /*
      ** Every page in the cache is controlled by an instance of the following
      ** structure.
      */
      struct PgHdr {
        sqlite3_pcache_page *pPage;    /* Pcache object page handle */
        void *pData;                   /* Page data */
        void *pExtra;                  /* Extra content */
        PCache *pCache;                /* PRIVATE: Cache that owns this page */
        PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
        Pager *pPager;                 /* The pager this page is part of */
        Pgno pgno;                     /* Page number for this page */
      #ifdef SQLITE_CHECK_PAGES
        u32 pageHash;                  /* Hash of page content */
      #endif
        u16 flags;                     /* PGHDR flags defined below */

        /**********************************************************************
        ** Elements above, except pCache, are public.  All that follow are 
        ** private to pcache.c and should not be accessed by other modules.
        ** pCache is grouped with the public elements for efficiency.
        */
        i64 nRef;                      /* Number of users of this page */
        PgHdr *pDirtyNext;             /* Next element in list of dirty pages */
        PgHdr *pDirtyPrev;             /* Previous element in list of dirty pages */
                                /* NB: pDirtyNext and pDirtyPrev are undefined if the
                                ** PgHdr object is not dirty */
      };
      ```

      You can see that the `nRef` field tracks how many users a page has, this 
      function will increase this field by one (in other words, pin this page),
      if this page is in the free list, then remove it from that list.

      ```c
      void sqlite3PagerRef(DbPage *pPg)
      ```

      > About the `pin` in SQLite, you may notice that it does not differentiate
      > read and write pin, because that has already handled by the database file
      > level lock so that you don't need to worry about it at the page level.

   7. `sqlitePagerUnref()`

      Decrease the `nRef` field by one, when the `nRef` becomes 0, this page
      is unpined and free.

      When all pages have been unpinned (trakced by `PgHdr.pCache.nRefSum`), 
      the lock on the database file will be released.

      ```c
      void sqlite3PagerUnref(DbPage *pPg)
      ```
      

# 5.3 Page Cache
## 5.3.1 Cache state
## 5.3.2 Cache organizatioin
## 5.3.3 Cache read
## 5.3.4 Cache update
## 5.3.5 Cache fetch policy
## 5.3.6 Cache management
### 5.3.6.1 Cache replacement
### 5.3.6.2 LRU cache replacement schema
### 5.3.6.3 SQLite's cache replacement schema
# 5.4 Transaction Management
## 5.4.1 Normal processing
### 5.4.1.1 Read operation
### 5.4.1.2 Write operation
### 5.4.1.3 Cache flush
### 5.4.1.4 Commit operation                                                    
### 5.4.1.5 Statement operations
### 5.4.1.6 Setting up savepoints
### 5.4.1.7 Releasing savepoints
## 5.4.2 Recovery processing
###  5.4.2.1 Transaction abort
### 5.4.2.2 Statement subtransaction abort
### 5.4.2.3 Reverting to savepoints
### 5.4.2.4 Recovery from failure
## 5.4.3 Other management issues
### 5.4.3.1 Checkpoint
### 5.4.3.2 Space constraint
