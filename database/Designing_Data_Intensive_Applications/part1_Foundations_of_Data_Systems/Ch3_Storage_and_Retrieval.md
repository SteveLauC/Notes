> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Storage engines (B-Tree vs LSM-Tree)
>   * Indexes (Hash, maybe inverted index)
>   * Basic intro to OLAP
>   * How columnar formats do compression and writes
> * After I read it
>   * Introduction to 2 major storage engines (B-Tree/LSM)
>     > The description of B+Tree here also applies to B-Tree
>   * Basic intro to various index structure
>   * Intro to in-memory DBs
>   * Intro to AP and column storage

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   * What is the on-disk format for a hash index 
>     > I don't think this will be covered (Yeah, it is not covered)
>   * OLAP histrory and key products (C-store is cool)
>   * How write works with columnar storage (C-store is cool)
>     > I know a bit about parquet, but let's see what we will cover

> What have you learned from it
>
> * log file + hash index => bitcast, this is really cool
> * Why compaction in LSM could potentially harm performance
> * C-store (Vertica)'s replicas have different sort order, interesting
> * The way C-store (Vertica) writes data into column-based format becomes pretty
>   much the standard for column-based databases
> * Data cube (meterialized cube) is new to me

> TOC
>
> * Data Structures That Power Your Database
>   * Hash Indexes
>   * SSTable and LSM-Trees
>   * B-Trees (It is a B+Tree actually)
>   * Comparing B-Trees and LSM-Trees
>   * Other Indexing Structures
> * Transaction Processing or Analytics (TP or AP)
>   * Data Warehousing
>   * Stars and Snowflakes: Schemas for Analytics
> * Column-Oriented Storage
>   * Column Compression
>   * Sort order in Column Storage
>   * Writing to Column-Oriented Storage
>   * Aggregation: Data Cubes and Materialized Views
> * Summary

# Data Structures That Power Your Database

1. The general idea behind index is to keep some additional metadata on the side,
   which acts as a signpost and helps you to locate the data you want.
   
   * B+Tree
   * Hash
   * GIN
   * IVFFlat

## Hash Indexes

1. A very interesting key-value storage engine can be built by simply appending 
   to a single file, appending to a file is efficient so the write performance
   should be good.
   
   For point query, we scan the file from end to start, to locate the target key.
   Range query can be done by in a similar way. The read performance should be
   bad as we are basically scanning the whole file. This problem can be resolved
   by introducing a hash index, for each key, record its start offset in a hashmap.
   
   Now we have a key-value storage that is both good at read and write. The idea
   looks toy but it is indeed viable.
   
   > This is what [bitcask](https://github.com/basho/bitcask) does.
   
   If you want to optimize write by reducing the number of `write(2)`(batching
   data in memory), you can add an in-memory buffer. To mitigate the space
   amplification issue, you can implemnent compaction (GC). If you want to 
   support efficient range query, then you need to have sorted logs. With these 
   3 optimizations, you are moving torwards LSM.
   
2. A data structure that requires many seeks won't be suitable for disk, e.g., 
   B+Tree typically few times of seek, making it very suitable for disk.
   
## SSTable (sorted string table) and LSM-Trees

1. LSM in-memory buffer is ordered, but the write order of different keys won't
   be maintained, the write order of the same key will be retained.
   
   ```
   write order:        (b, 2), (b, 1), (a, 0)
   buffer store order: (a, 0), (b, 2), (b, 1)
   ```
   
2. SSTable requires that

   1. All the keys stored in a file are ordered
   2. Each key only appears once 
   
   > memtable would satisfy both requirements in the first place.
   
3. Advantages of SSTable over the simple log file (introduced in the prev section)

   1. Merging segments becomes much efficient because segments are now ordered.
      We can do a N-way merge.
   2. Since data is ordered, sparse index can be used. With the simple log file,
      you must use a dense index.
      
4. Lucene stores its term dictionary in a structure similar to LSM.

5. LSM has 2 compaction strategies:

   * size-tiered
   
     used by HBase
     
   * leveled compaction
     
     Used by LevelDB (hence the name of LevelDB) and RocksDB
     
   
   Cassandra supports both strategies.  

## B-Trees (It is actually a B+Tree)

1. Similarities between LSM and B-Tree: they both keep data sorted by key.

2. WAL is simple, it only consists few interfaces:

   * read
   * write
   * clear
   * fsync
   * replay (can be implemented using read and storing a checkpoint)
   
   But it can be complex depending on your data model and storage engine
   
   * If your storage engine
   
     > B-Tree, B+Tree, heap all suffer from this.
   
     * For the on-disk structure, write-in-place
       
       * B-Tree, B+Tree, heap, they all write-in-place.
       * LSM is not write-in-place, existing SSTable won't be modified, new 
         SSTable resides in new files (i.e., new disk pages)
       * ~~Bitcask, if dost not have a memtable, every write would append the disk
         file, is also writes-in-place, every disk write has to write complete
         disk pages, appending a new entry, if is a torn write, could ruin the 
         previous written entries. WAIT, if you don't have an in-memory structure,
         then why the fxxk you need a WAL????~~
         
         Future steve: it is actually not, you are appending new bytes, existing
         bytes will be written again, with the same contents, so this is not
         write-in-place. And the file system should give you a guarantee that
         the file length will only be longer but not shorter (even with torn write),
         so the existing bytes are safe.
         
         If it is write-in-place, then appending a new record to WAL would harm
         previous records, which is not acceptable.
       
     * disk write is not atomic, can have data corrupted during crash
     
       > This is called torn write
     
     * The logs in your WAL only describes incremental changes
     
       Say you have an array, "append 1 to it" describes an incremental change. 
       For page-oriented databses, inserting a complete tuple to the page is 
       also an incremental change. For a page-oriented, B-Tree based key-value
       database, writing the whole key-value pair in the WAL is an incremental
       change.
       
       AS LONG AS you are not writing the whole disk write unit to your WAL, 
       your logs describe incremental changes.
       
     Then after every checkpoint, the first time you modify your disk write unit, 
     you should store the whole contents of this unit in the WAL because without
     it, you cannot restore the state back with your incremental changes.
     
     In the following cases, this issue does not exist:
     
     1. Don't write-in-place, example, LSM-tree
     2. Get a disk/fs/OS that supports atomic write
        
        * If the write fails, you get the previous state, i.e., no data overwriting,
          then you replay the logs to recover the state.
        * If the write succeeds, congratulations, nothing to do!
  
   * Do you need to `fsync(2)` the WAL after every write to ensure your data
     safety:
     
     * For LSM, yes
     * For Elasticsearch (or anything without transaction support), yes
     * For Postgres or any DBs that support transaction, you can `fsync(2)` wal
       when a transaction commits, so the answer is no. Specifically, Postgres 
       batches WAL in a memory buffer.
       
   * Write-ahead log, will it be written before your data write? 
   
     * For Elasticsearch, no, it writes Lucene first, then the WAL
     * For Postgres, no, according to a [post] from EDB, Postgres writes the 
       buffer pool first: 
       
       [post]: https://www.enterprisedb.com/postgres-tutorials/postgresql-replication-and-automatic-failover-tutorial#section-9
       
       QUES: is the procedure described in the post right?
       
     * For LSM, yes

## Comparing B-Trees(B+Tree) and LSM-Trees

1. As a rule of thumb, LSM is good at write, whereas B+Tree (or B-Tree) is good
   at read.
   
2. B+Tree write amplification

   * WAL
   * Repeated writes induced by dirty page flush
   * Extra writes to ensure page write atomicity
   
     For example, MySQL/InnoDB double write.
     
3. LSM write amplification

   * WAL
   * Repeated write induced by compaction
   
     > this reason.
     >
     > WiscKey: Separating Keys from Values in SSD-Conscious Storage
     >
     > LSM is SSD-Conscious
     
4. The compaction process of LSM can harm the performance because it takes disk 
   bandwidth.
   
   The book says that "The impact on throught and average response time is usually 
   small, but at higher percentiles the response time of queries to log-structured
   storage engine can sometimes be quite high", so the tail latency will be affected
   by compaction. 🤔
   
   > QUES: what will happen if you reaches disk bandwidth, requests will be pending
   > because they have to wait?
   
   And if your write throughput is high, it is possible that the compaction cannot
   keep up with your write, thus, you will have more segment files on the disk,
   your read performance will also be affected due to more segment file to check.
   
5. B+Tree is more suitable for implementing transaction isolation because transaction
   isolation uses locks on range of keys, and with B+Tree, these locks can be directly
   attached to the tree.
   
   > QUES: Perhaps I need to implement one to understand this.

## Other Indexing Structures

1. The posting list in full-text index can be considered an index that is for
   non-unique keys
   
   ```
   term -> [doc_id1, doc_id2, doc_id3]
   ```
   
2. MySQL/InnoDB stores the physical tuple in its primary index. They call this
   clustered index. Read performance will be better compared to storing 
   references in the index.
   
   QUES: I seriously think databases terminologies are amibiguous, clustered
   index is just an alias to primary index in the book Database System Concepts.
   
3. The book says:

   The performance advantage of in-memory databases is not due to the fact that
   they don't need to read from disk, rather, it is because they don't need to
   encode the data in a form that can be written to disk.

# Transaction Processing or Analytics (TP or AP)

1. In early days of business data processing, a write to the database typically
   corresponds to a commercial transaction. As databases expanded into areas
   that didn't involve money, the term "transaction" nevertheless stuck.
   
   Even though databases begin to be used for many different kinds of data, the
   pattern remains unchanged, because these applications are interactive, this
   pattern is called "oneline transaction processing", "online" means requests
   will be handled interactively/in realtime, in contrast to "batch processing".
   
   A transaction does not necessarily need to be ACID, it just means provides
   low latency (means response time) read and write, as a contrast to batch
   processing.
   
2. Databases also started being used for data analytics, whose access pattern is
   very different from OLTP. Usually, an AP query only involves few columns from
   the table, but needs to scan a huge number of records.
   
   > The meaning of "Online" in "OLAP" is **undefined**

3. Compares OLTP and OLAP **workloads**

   ![illustration](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-11-02%20at%208.34.28%E2%80%AFAM.png)
   
   For the write of OLAP, it is usually done by ETL, so it would be bulk write 
   in most cases if your ETL works periodly. The data in OLAP databases can be 
   history data, but there is  a trend to make OLAP purely realtime.
   
4. Compares OLTP and OLAP from database's perspective

   | x        |  OLTP                | OLAP                        |
   |----------|----------------------|-----------------------------|
   |storage   | row-based            |column based                 |
   |cache     | row-based            |column based                 |
   |execution | volcano/pull-based   |volcano/vectorized/push-based|
   |index     | hash/b+tree/...      |min-max/statictics           |
   

4. In the 1980s, there was a trend to stop using OLTP databases for OLAP workload,
   dedicated databases are used instead, these databaes are called "Data Warehouse".

## Data Warehousing

1. If you run OLAP workload and OLTP workload using the same database, then the
   TP workload won't perform well because the AP worklaod will eats all the CPU
   and disk throughput.
   
2. The index structures we have seen in this chapter:

   * hash
   * B+Tree
   
   They are suitable for OLTP workloads, they won't answer OLAP queries well.

## Stars and Snowflakes: Schemas for Analytics

1. What is star schema

   When you ETL your data into a data warehouse, your data is typically represented
   in this (logical) schema. I think the reason why people chose this is that it makes analytics
   easier?
   
   > It is just a logical scheam, how you store your data in multiple tables, 
   > it has nothing to do with database systems itself.
   
   > See this for an example:
   > 
   > https://github.com/SteveLauC/Notes/blob/b41a1ca9388a2f63a693a615c5f820b03d8a4816/database/Database_System_Concepts/Ch11_Data_Analytics.md#1122-multidimensional-and-warehouse-schemas
   
   Snowflake schema is a more normalized form of star schema, dimension tables are
   broken into sub-dimension tables.
   

# Column-Oriented Storage

1. For a columnar format, row entries in every column should have the same order
   so that when you answer queries like "SELECT *", you can reassemble the row.

## Column Compression

https://github.com/SteveLauC/Notes/blob/b41a1ca9388a2f63a693a615c5f820b03d8a4816/database/CMU15-445-2022-fall/Lecture5_Storage_Models_and_Compression.md

## Sort order in Column Storage

1. DBA can sort the data based on the usual queries.

2. Sorting can help compression.

3. For a distributed database, replication is a must-do, why not store them in 
   different orders, then a query can use the replica that fits the query best.
   
   This idea was introduced in C-Store and Vertica.
   
   > Let's extend this! Given that you have replicas, why not store one in a 
   > row-based format, another one in a column-based format! Lol
   

## Writing to Column-Oriented Storage

Column-oriented format, compression, maintaining sort order, they are all helpful
for AP read-only queries, they have the downside of making write (typically, update
in place) difficult.

So column-based databases typically adopt a write mode similar to LSM, accumulate
data in memtable, then write them to the disk in batch. Vertica does the write in 
this way, and probably C-store is the first one that didn't it.

## Aggregation: Data Cubes and Materialized Views

1. Data cubes is just a speial case of materialized view. 

# Summary