> Today's agenda:
>
> * Introduction
> * Locks vs Latches
> * Buffer pool
>   * Buffer pool metadata
>   * Memory allocation policies
> * Buffer pool optimization
>   * Multiple Buffer Pools
>   * Pre-fetching 
>   * Scan Sharing (Synchronized Scans)
>   * Buffer Pool Bypass
> * OS Page Cache
> * Buffer Replacement Policies
> * Other memory pools

# Introduction
1. The DBMS is responsible for managing its memory and moving pages back-and-forth
   from disk as most DBMSes are disk-based.

# Locks vs Latches
1. We need to make a distinction between locks and latches when discussing how 
   the DBMS protects its internal elements. 

   * Locks: A lock is a higher-level, **logical** primitive that protects the 
     contents of a database (e.g., tuples, tables, databases) from other transactions.
     Transactions will hold a lock for its entire duration. Database systems can 
     expose to the user which locks are being held as queries are run. 

     Locks need to be able to rollback changes. 

     > High level, transaction level

   * Latches: A latch is a low-level protection primitive that the DBMS uses 
     for the critical sections in its internal data structures (e.g., hash 
     tables, regions of memory). Latches are held for only the duration of 
     the operation being made. 

     Latches do not need to be able to rollback changes.

     > Low level, source code level, `Mutex<T>`, `RwLock<T>`.

# Buffer Pool

1. The internal memory is called `Buffer Pool`, which are composed of an array
   of fixed length page, each page is called a `frame`.When the DBMS needs a 
   page, it will first search the buffer pool to see if it is already in the
   memory. If not found, then an exact copy will be placed into a specific 
   `frame`

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2014-43-36.png)

2. Buffer Pool Metadata - `Page Table`

   The buffer pool must maintain certain meta-data in order to be used efficiently
   and correctly. 

   `Page Table` keeps track of :
      
   * Record which pages are in the buffer pool
   * Mapping from `frame` to `PageID`
   * Dirty sign for each page
   * Pin/Reference counter for each page: prevents the Buffer Pool Manager from
     evicting it.

3. Memory allocation policies
   
   There are generally two policies:

   1. `Global Policies`

      Deal with decisions that the DBMS should make to benefit the entire 
      workload that is being executed. It considers all active transactions 
      to find an optimal decision for allocating memory.

   2. `local policies`

      Make decisions that will make a single query or transaction run faster,
      even if it isn’t good for the entire workload. Local policies allocate 
      frames to a specific transactions without considering the behavior of 
      concurrent transactions. 

   > Most systems use a combination of both global and local views.

# Buffer Pool Optimization

There are a number of ways to optimize the Buffer Pool:

1. Multiple buffer pools
   
   The DBMS does not always have a single buffer pool:

   > Basically partition the memory for different components.

   * per-database buffer pool
   * per-page *type* buffer pool(data/index/metadata)

   > Helps reduce latch contention (lock granularity) and improve locality.

   When we have multiple buffer pools, how can we know which buffer pool a 
   page is in? Two approaches:

   1. Object-ids
      
      Extend the internal ID with a field `ObjectID`:

      ```
      // Before

      (ObjectID, PageID, SlotID)
      ```

      ```
      // After 

      (ObjectID, PageID, SlotID)
      ```

      And we store a mapping from `ObjectID` to `Buffer Pool` so that we can 
      find where this tuple goes using its `ObjectID`.

   2. Hash the `PageID` to select which buffer pool to go


2. Pre-fetching

   The DBMS can prefetch pages based on a query.

   per-fetching categories:

   1. Sequential scans

      For example, if our query needs `page 2`, then we can also copy `page 3`
      to the buffer pool. When DBMS need `page 3`, it is already in the buffer
      pool.

      > This is quite simple, the OS can also do this for us. For example, if 
      > you ask a page a disk file using `mmap`, then the OS will prefetch the
      > following pages for us. So by the time of asking subsequent pages, they
      > are already in the memory.

   2. Index scans
    
      > This is what the OS can NOT do, cause it doesn't know the semantics
      > of our query.
      >
      > This should be used to index files whereas the OS does not know if a 
      > file is an index file or not, it is just chunks of bytes.

      For example, we have a query

      ```sql
      SELECT * FROM A
      WHERE val BETWEEN 100 AND 250;
      ```

      And we have a `index` for table `A`

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-29-14.png)

      To iterate over our `index`, we have to read the root node of that index
      (B+Tree) into memory.
     
      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-30-27.png)

      And I know the `val` I need is greater than 100, which is on the left side
      of the tree, so I am gonna read `index-page-1` into memory.
      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-33-56.png)
     
      At this time, the OS thinks "Oh, you have read page1, maybe you also need
      page 2 and 3". But we know that's not the case, according to our index,
      the pages we actually need are page 3 and 5.
     
      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-37-06.png)


3. Scan sharing

   > This is called `synchronized scans` in PostgreSQL's term.

   If a query starts a scan and there is already one doing this, then the DBMS 
   will attach the new task to the existing query's cursor.

   For example:

   There is a Q1(`SELECT SUM(a) FROM A`), and Q1 is scanning the disk file Page 3

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-41-28.png)

   Then another query Q2(`SELECT AVG(a) FROM A`) comes, and the DBMS knows Q2
   and Q1 need to scan the same disk file(Page 0-5). Normally(without optimization),
   Q2 should scan from Page 0, but the DBMS knows Q2 and Q1 should scan the 
   same page so it will simply attach the cursor of Q2 to the cursor 
   of Q1, so that they all scan from Page 3. When Page 5 is scanned, Q1 is 
   done, Q2 still needs to scan Page0-2 to finish its job.

   ![without_optimization](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-47-49.png)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-48-24.png)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-48-36.png)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-48-52.png)

4. Buffer pool bypass

   Bringing a page into the buffer pool has its overhead(Page Table Metadata 
   Management), if the page content is only used ONCE, there is no need to 
   store it in the buffer pool(if so, we have to replace it later)

   Called `light scans` in `informix DBMS`

   For example, the result of join may not be used anymore. So we just 
   calculate it and give it to the user.

# OS Page Cache

1. We read/write using OS api, the OS itself will cache the disk page but we 
   don't need it so it is redundant. Use `open(file_path, O_DIRECT)` to 
   bypass it(Direct I/O)

   > PostgreSQL is the only system relies on the OS cache.

   Why don't most DBMSs use this feature:
   1. It is redundant in terms of memory usage
   2. Most DBMSs support different OSs, and the OS cache performance may vary
      between them, to make their product consistent in performance crossing 
      different OSs, they should close this feature.

# Buffer Replacement Policies

> When the buffer pool is full, we need to free up a `frame` to make room for a
> new page, it must decide which page to `evict/replace`. A replacement policy is
> an algorithm that the DBMS implements that makes a decision on which pages to 
> evict from buffer pool when it needs space.

1. How to evict pages

   1. Least Recently Used(LRU)

      Maintain a timestamp of when each page is last accessed, evict the page 
      with the oldest timestamp

   2. Clock

      > The mechanism behind it is: if a page has not been accessed **for a 
      > while**(i.e., since the last visit of the clock hand), it may not 
      > be used in the near future, so just evict it.

      In this policy, image all the pages are put in a circular buffer 
      (clock), and there is a clock hand regularly sweeps the buffers (just like
      a real clock hand). Each page is given a `reference bit` indicating whether
      it is accessed by the query or not, when it is accessed, set the bit to 1.

      When the clock hand visits a buffer, check its reference bit, if it is
      1, set it to 0(alright, this page survives this round); if it is 0, 
      which means there is no access since clock hand's last visit, it is out.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-00.png)

      Find a reference bit of 1, set it to 0

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-19.png)

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-31.png)

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-45.png)

      find a reference bit of 0, evict this frame.
   
   > One problem of `LRU` and `Clock` is that they are susceptible to `sequential
   > flooding`: A query that sequentially scan all the pages, this will pollute
   > the buffer pool with pages that are ONLY used once.
   >
   > In this case, the most recently used page is the most **unwanted** page.

   3. LRU-K

      > This is not affected by `sequential flooding`.

      For each page, we keep track of the timestamps of the last `k` accesses,
      and we compute the interval between accesses and use it estimate the 
      next access.

2. How to handle dirty pages (when replacing frames)
   
   * Fast way: Ignore dirty frames, simply drop frames that are not dirty.

   * Slow way: Write the dirty frame back to the disk and drop it

   The slow way is expensive. And there is a method which is: the DBMS 
   **periodically** walks through the `page table` and writes the dirty frame back 
   to disk in the background. When the modification is sent back to the disk, 
   the DBMS can drop it or simply unset its dirty flag. 

   NEED TO BE CAREFUL THAT **THE LOGS SHOULD BE WRITTEN BEFORE THE MODIFICATION**.

# Other kind of Memory Pools
1. The DBMS needs memory for things other than just tuples and indexes. These 
   other memory pools may not always backed by disk depending on implementation.

   * Sorting + Join Buffers
   * Query Caches
   * Maintenance Buffers
   * Log Buffers
   * Dictionary Caches
