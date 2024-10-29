> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Storage engines (B-Tree vs LSM-Tree)
>   * Indexes (Hash, maybe inverted index)
>   * Basic intro to OLAP
>   * How columnar formats do compression and writes
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   * What is the on-disk format for a hash index 
>     > I don't think this will be covered)
>   * OLAP histrory and key products
>   * How write works with columnar storage 
>     > I know a bit about parquet, but let's see what we will cover

> What have you learned from it
>
> *
> *

> TOC
>
> * Data Structures That Power Your Database
>   * Hash Indexes
>   * SSTable and LSM-Trees
>   * B-Trees
>   * Comparing B-Trees and LSM-Trees
>   * Other Indexing Structures
> * Transaction Processing or Analytics
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

## B-Trees
## Comparing B-Trees and LSM-Trees
## Other Indexing Structures
# Transaction Processing or Analytics
## Data Warehousing
## Stars and Snowflakes: Schemas for Analytics
# Column-Oriented Storage
## Column Compression
## * Sort order in Column Storage
## Writing to Column-Oriented Storage
## Aggregation: Data Cubes and Materialized Views
# Summary