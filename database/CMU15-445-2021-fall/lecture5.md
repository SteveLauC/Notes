### Today's Agenda

#### Buffer Pool Manager
1. `frame`
   The buffer pool is divided into an array of fixed-size pages, each page is
   called `frame`. When the DBMS needs a page, it will be copied from disk to
   a specific `frame`

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-23%2014-43-36.png)

2. metadata of buffer pool: Page Table(Do not confuse with the OS page table)

   A mapping between `frame` and `disk page ID`. In addition to this mapping, it
   also records some extra metadata about each page:

   1. dirty flag: if this frame is modified, it's dirty and this flag is set.
   2. pin/reference counter: how many threads are accessing(read or write) this
   frame. Each thread must increment it before accessing it.

3. allocation policies
   1. global policies: make decisions for all the active(concurrent) transactions
   2. local policies: allocate frames for a specific transction without considering
   the behivior of concurrent transactions

4. optimization

   1. mupltiple buffer pools
      
      The DBMS does not always have a single buffer pool:

      * per-database buffer pool
      * per-page type buffer pool

      > Helps reduce latch contention and improve locality.


   2. pre-fetching

      The DBMS can prefetch pages based on a query.

      per-fetching categories:

      1. sequential scans

         For example, if our query needs `page 2`, then we can also copy `page 3`
         to the buffer pool. When DBMS need `page 3`, it is already in the buffer
	 pool

      2. index scans
         
	 ???


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


#### Frame Replacement Policies
> When the buffer pool is full, we need to free up a frame to make room for a
new page, it must decide which page to `evict/replace`

1. Least Recently Used(LRU)
   Maintain a stimestamp of when each page is last accessed, evict the page with
   oldest timestamp

#### Other Memory Pools
