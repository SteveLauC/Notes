1. difference between `Locks` and `Latches`

* Locks: A lock is a higher-level, logical primitive that protects the contents 
  of a database (e.g., tuples, tables, databases) from other transactions. 
  Transactions will hold a lock for its entire duration. Database systems can 
  expose to the user which locks are being held as queries are run. Locks need 
  to be able to rollback changes.

* Latches(like Mutex): A latch is a low-level protection primitive that the DBMS uses for 
  the critical sections in its internal data structures (e.g., hash tables, 
  regions of memory). Latches are held for only the duration of the operation 
  being made. Latches do not need to be able to rollback changes.

2. about log
  
   we have to make sure log is written back to disk before the data.

### Today's Agenda

#### Buffer Pool Manager
1. `frame`

   The buffer pool is divided into an array of fixed-size pages, each page is
   called a `frame`. When the DBMS needs a page, it will first search the buffer
   pool to see if it is already in the memory. If not found, then an exact copy 
   will be placed into a specific `frame`

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2014-43-36.png)

2. metadata of buffer pool: Page Table(hashtable)(Do not confuse with the OS page table 
   or the page directory)

   > The buffer pool must maintain certain meta-data in order to be used efficiently
   and correctly.

   A mapping between `frame` locations and `disk page ID`. In addition to this 
   mapping, it also records some extra metadata about each page:

   1. dirty flag: if this frame is modified since read from disk, it's dirty and this flag is set.
   This indicates to the storage engine that this frame or page must be written
   back to the disk.

   2. pin/reference counter: how many threads are currently accessing(read or write)
   this frame. Each thread must increment it before accessing it. 

   If the counter is greater than 0, then the storage engine is not allowed to 
   evict this page from memory. This is why it is named `Pin` counter, we are
   actually pinning(locking) it in the memory

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-45-15.png)

3. memory allocation policies

   1. global policies: make decisions for all the active(concurrent) transactions
   to benefit all the workloads

   2. local policies: allocate frames for a specific transction to make it run 
   faster without considering the entire workload

   > Most systems use a combination of them.

4. optimization

   1. multiple buffer pools
      
      The DBMS does not always have a single buffer pool:

      * per-database buffer pool
      * per-page *type* buffer pool(data/index/metadata)

      > Helps reduce latch contention and improve locality.

      When we have multiple buffer pools, how can we know which buffer pool a 
      page is in? Two approaches:

      1. object-ids
      2. Hash the page_id to select which buffer pool to go


   2. pre-fetching

      The DBMS can prefetch pages based on a query.

      per-fetching categories:

      1. sequential scans

         For example, if our query needs `page 2`, then we can also copy `page 3`
         to the buffer pool. When DBMS need `page 3`, it is already in the buffer
	 pool

	 > This is quite simple, the OS can also do this for us. For example, if 
	 you ask a page a disk file using `mmap`, then the OS will prefetch the
	 following pages for us. So by the time of asking subsequent pages, they
	 are alreay in the memory.

      2. index scans
       
         > This is what the OS can NOT do, cause it doesn't know the semantics
	 of our query.

	 For example, we have a query
	 ```sql
	 SELECT * FROM A
	 WHERE val BETWEEN 100 AND 250;
	 ```

	 And we have a `index` for table `A`

	 ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-29-14.png)

	 To iterate over our `index`, we have to read the root node of that index
	 into memory.
         
	 ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-30-27.png)

	 And I know the `val` I need is greater than 100, which is on the left side
	 of the tree, so I am gonna read `index-page-1` into memory.
	 ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-33-56.png)

	 At this time, the OS thinks "Oh, you have read page1, maybe you also need
	 page 2 and 3". But we know that's not the case, according to our index,
	 the pages we actually need are page 3 and 5.

	 ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2016-37-06.png)



   3. scan sharing

      if a query starts a scan and if there one already doing this, then the DBMS w
      ill attach to the second query's cursor

      For example:

      There is a Q1(select sum(a) from A), and Q1 is scanning the disk file Page 3

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-41-28.png)

      Then another query Q2(select avg(a) from A) comes, and the DBMS knows Q2
      and Q1 need to scan the same disk file(Page 0-5). Normally(with optimization),
      Q2 should scan from Page 0, but the DBMS knows Q2 and Q1 should scan the 
      same page so it will simply attach the cursor of Q2 to the cursor 
      of Q1, so that they all scan from Page 3. When Page 5 is scanned, Q1 is 
      done. But Q2 still needs to scan Page0-2 to finish its job.

      ![without_optimization](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-47-49.png)
      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-48-24.png)
      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-48-36.png)
      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2015-48-52.png)

   4. buffer pool bypass

      If the page content is only used once, there is no need to store it in
      the buffer pool(if so, we have to replace it later)

      Called `light scans` in `informix DBMS`

      For example, the result of join may not be used anymore. So we just 
      calculate it and give it to the user.

   4. OS cache bypass 

      We read/write using OS api, the OS itself will cache the disk page but we
      don't need it so it is redundant. Use `open(file_path, O_DIRECT)` to bypass it.

      > PostgreSQL is the only system relies on the OS cache.

      Why don't most DBMSs use this feature:
      1. It is redundant in terms of memory usage
      2. Most DBMSs support different OSs, and the OS cache performance may vary
      between them, to make their product consistent in performance crossing 
      different OSs, they should close this feature.


#### Frame Replacement Policies

> When the buffer pool is full, we need to free up a frame to make room for a
> new page, it must decide which page to `evict/replace`. A replacement policy is
> an algorithm that the DBMS implements that makes a decision on which pages to 
> evict from buffer pool when it needs space.

1. Least Recently Used(LRU)

   Maintain a stimestamp of when each page is last accessed, evict the page with
   oldest timestamp

2. clock

   > The mechanism behind it is: if a page is not accessed in a while, it may not
   be used in the near future, so just evict it.

   In the clock policy, each page is given a `reference bit` indicating whether
   it was accessed since `clock hand`'s last visit, when a page is accessed by
   a query, set it to 1.

   Place all the frames in a circular shape with a `clock hand`. When the `clock
   hand` is sweeping, if it finds a frame's `reference bit` is 1, set it to 0.
   If it is 0, which means in the duration from the time of its last visit no 
   query needs this, evict it.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-00.png)
   Find a reference bit of 1, set it to 0
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-19.png)
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-31.png)
   find a reference bit of 0, evict this frame.
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2015-19-45.png)

3. how to handle dirty pages(when replacing frames)
   
   Fast way: simply drop frames that are not dirty

   Slow way: write a dity page back to disk and drop it

   The slow way is expensive. And there is a method which is: the DBMS periodically 
   walks through the page table and writes the diry page back to disk in the 
   background. When the modification is sent back to the disk, the DBMS can drop
   it or simply unset its dirty flag. NEED TO BE CAREFUL THAT THE LOGS SHOULD 
   BE WRITTE BEFORE THE MODIFICATION.

### Other Memory Pools
