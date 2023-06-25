> This chapter is mainly about how the data in a relational database fit into
> the block-based disk files, and how we manage the memory buffer such that we
> can minimize the transfers between the memory and disk.

> * 13.1 Database Storage Architecture
> * 13.2 File Organization 
>   * 13.2.1 Fixed-Length Records
>   * 13.2.2 Variable-Length Records
>   * 13.2.3 Storing Large Objects
> * 13.3 Organization of Records in Files
> * 13.4 Data-Dictionary Storage (system catalog)
> * 13.5 Database Bufffer
>   * 13.5.1 Buffer Manager
>     * 13.5.1.1 Buffer replacement strategy
>     * 13.5.1.2 Pinned blocks
>     * 13.5.1.3 Shared and Exclusive locks on Buffer
>     * 13.5.1.4 output of blocks
>     * 13.5.1.5 Forced output of blocks
>   * 13.5.2 Buffer-Replacement Strategies
>   * 13.5.3 Reordering of Writes and Recovery
> * 13.6 Column-Oriented Storage
> * 13.7 Storage Organization in Main-Memory Databases
> * 13.8 Summary

# 13.1 Database Storage Architecture

1. Most databases use operating system files as an intermediate layer for storing 
   records, which abstract away some details of the underlying blocks. However,
   to ensure efficient access, as well as to support recovery from failures,
   databases must continue to be aware of blocks.

# 13.2 File Organization 

## 13.2.1 Fixed-Length Records
1. In a relational database, tuples of distinct relations are generally of 
   different sizes. One approach to mapping the database to files is to use
   several files to store records of only one fixed length in any given file.

   Or we can design some structures to maintain var-len records in the same
   file.

   > The former is easier to implement

2. For a fixed-length record, the easiest way to store it in a file is simply
   put it there, one by one.

   ```
   # A file containing multiple fixed-length records
   [Record][Record][Record][...][Record]
   ```

   Some problems in this method:
   1. Unless the block size is a multiple of the length of the record, some 
      record will cross block boundaries. To read this special record, you 
      need to read two blocks.

      > Though we can only allocate as many records to a block as would fit
      > entirely in the block. The remaining bytes in that block won't be 
      > used.

   2. To delete a record, the space occupied by this record has to be filled
      by other records, or we have to have a way of marking the deleted records,
      like tombstone in LSM, when inserting a new record, we can write the new
      record here.

      > Another workaound is, we can `swap_remove()` it, like replacing that record
      > with the last one and truncate the file.(as records are unordered)

   3. Every search will result in a whole file scan, which is expensive.

3. Free list

   Free list is a link list maintaining the records that have been deleted. To
   implement this, we add a header to the file, which simply stores the address
   (offset) of the first deleted record, and in the space of the first deleted 
   record, we store the address of the next deleted record, and so on.

   > Since we don't de-allocate the space of a deleted record, file size always
   > gets bigger, even with deletion operations.

## 13.2.2 Variable-Length Records

> Obviously, not all data types in SQL have fixed-size. And records with different
> schemas can be stored in the same file.

1. There are different techniques for implementing var-len records, but two main
   problems have to be resolved by any such technique.

   1. How to represent a single record in such a way that individual attributes
      can be extracted easily, even if they are var-len(i.e., var-len types like
      varchar(x)).
   2. How to store var-len records within a block, such that records in a block 
      can be extracted easily.

   > If records have fixed-len, these problems can be easily addressed by seeking
   > the disk file.

2. A typical way for storing var-len record:

   For the space occupied by this record, it can be divided into two parts:
   1. Some kind of Metadata 

      > not totally correct, as it does contain data for fixed-len attributes

      Fixed length for all records in the same relation.

   2. Data for var-len attributes

   Let me give an example:

   ```SQL
   CREATE TABLE Person (
       name VARCHAR(10),
       age INT
   );

   INSERT INTO Person VALUES
   ('steve', 18);
   ```

   For the above `Person` tuple `steve`, it is stored like this:

   ```
   [2B  2B 4B ][    1B     ][  5B   ]
   [(8, 5)(18)][null bitmap]['steve']
   ```
   
   For a var-len attribute, we store its (offset, len) in the "metadata", and 
   store its actual data in the data part. For a fixed-len attribute, it is 
   simply stored in the "metadata" part.

   > `offset` and `len` are stored in 2 bytes, you can alter it if 2 bytes is
   > not sufficient.

   The `null bitmap` in the middle indicates which attributes is `null`, here
   we use 1 byte for it, as we only have 2 attributes. If an attribute is null,
   the corresponding bit in the `null bitmap` will be set to 1, for these null
   attributes, it is ok to not to allocate space at the cost of extra work to
   extract attributes from this record.

3. How var-len records are stored in a block: slotted-page structure
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/slotted_page_structure.png)

   This structure divides a block into three parts:
   * Header (at the beginning)
     
     It contains teh following information:

     * The number of tuples in this block
     * The end of the free space
     * An array of slots whose contents contain the location and size of each tuple

   * Free space (in the middle)
   * Tuples (at the end)

     Tuples are stored continuously, starting from the end of the block. 

   > With this structure, the length of the file is the multiple of block size.

4. CRUD on slotted-page structure
   
   * Insertion
     
     When inserting new tuple, space is allocated for it at the end of 
     the free space, and an entry containing its size and location is 
     added to the header.

   * Deletion
     If a tuple is deleted, the space it occupies is freed, and its slot is set
     to `deleted`(e.g., set the size to -1). The tuples in the block before the
     deleted tuple are moved, so that the free space created by deletion gets 
     occupied, and all free space is again between the final slot in the header
     array and the first tuple.

   * Update
     Similar to deletion, if the size of that tuple gets updated, we adjust the 
     locations of the tuples before the updated tuple.

   > In the process of deletion and update, tuples may need to be moved, the cost
   > is not too high since the size of a block is limited.

## 13.2.3 Storing Large Objects

1. Many databases choose to store very large objectss, such as video data, 
   outside of the database, in file system. In such cases, the database
   may store the filename(usually a path in the file system) as an attribute
   of a tuple in the database.

   > The corresponding file may be deleted and results in a form of foreign-key
   > constraint violation. And since the actual data is stored outside of the
   > database, database authorization controls are not appliable to the data
   > stored in the file system.
   >
   > Some databases support file system integration with the database, to ensure
   > that containts are satisfied.

# 13.3 Organization of Records in Files

> I am still unclear about this secion.

1. How to organize tuples in files 
   * Heap file organization
     Everying is unordered

   * Sequential file organization
     Keep the tuples sorted by the `search key`, and they are chained by link 
     list.

     > A `search key` is some attributes used to sort tuples.

   * Multitable clustering file organization
   * B+TREE file organization
     SQLite uses this.
   * Hashing file organization
2. Free space map

   This map is an array of `fraction`s to keep track of the capacity of each
   block, for example:

   ```
   [4, 2, 1, 4, 7]
   ```

   Here is an array of length 5, which means we have 5 blocks, each `fraction`
   takes 3 bits, so the max value is `8`, `7` means that 7/8 space is unoccupied.


# 13.4 Data-Dictionary Storage (system catalog)
1. Data dictionary is used to store the metadata(relation schema)
   * Names of the relations
   * Names of the attributes of each relation
   * Domains and lengths of attributes
   * Names of views defined on the database, and definition of those views
   * Integrity contrints (e.g., key constraints)
   * Names of users, the default schema of the users, and password or other 
     information to authenticate uers
   * Information about authorizations for each user.

   > The above metadata is actual a miniature database, so some DBMSs store
   > them as a relation.

# 13.5 Database Bufffer

> Database still mainly reside on the disk.

1. Goal of the buffer manager
   * Minimize the number of block transfers between the disk and memory.
     > Why not laod all blocks into the memory? Impossible
   * Maxmize the chance that when a block is accessed, it is already in the 
     memory, and, thus, no disk access is required.

## 13.5.1 Buffer Manager
> In SQLite, this layer is called pager.

### 13.5.1.1 Buffer replacement strategy
### 13.5.1.2 Pinned blocks
1. A buffer block should not be evicted while being accessing, to do this, we `pin`
   the block, and the buffer manager will never evict pinned blocks.

   A simple way to implement this is to keep a `pin` count for each block, every
   time it gets pinned, the count + 1, only blocks with pin count 0 can be evicted.

### 13.5.1.3 Shared and Exclusive locks on Buffer
> RwLock
### 13.5.1.4 output of blocks
### 13.5.1.5 Forced output of blocks
1. forced ouput: Evict a block forcefully

   > will be discussed in Ch19

## 13.5.2 Buffer-Replacement Strategies
1. Operating system uses LRU as its replacement strategy because OS is a general
   purpose software. If database maintains its own buffer, then it can use some
   more intelligent algorithms to manage its buffer as it has the context, and,
   thus, can predict the future block useage more accurately.

2. `toss-immediate` strategy

    After using a block, evict it as soon as possible because it won't be used
    in the near future.

    ```sql
    SELECT * FROM instructor, department;
    ```

    For the above query, assume these two relations are stored in two separate
    files, and the query will be executed with the following pseudo-code.

    ```
    for tuple1 in instructor:
        for tuple2 in department:
            include <tuple1, tuple2> as port of the result
    ```

    We can see that once a tuple in `instructor` has been processed, it is no
    longer needed again, even though it has been used recently. And for the 
    block that contains tuples of `instructor`, the buffer manager should evict
    it as soon as the final tuple in that block has been processed.

2. Most recently used (MRU) strategy

## 13.5.3 Reordering of Writes and Recovery
1. Database buffers allows writes to be performed in-memory and output to disk 
   at the later time, possibiy in an order different from the oreder in which
   the writes were performed.

   However, such reordering can lead to inconsistent data on disk in the event
   of a system crash. So the ancient file system has to perform a `file system
   consistency check` on start, if it is not consistent, extra steps have to be
   takes to restore to the consistency.

   Modern file systems assign a disk for storing a log of the writes in the order
   that they were performed, such a disk is called `log disk`.

   > 什么 WAL

   File systems that support log disks are called `journaling file system`, and
   it can be implemented without using a separate disk for log, log can be reocrded
   in the same disk.

# 13.6 Column-Oriented Storage
1. Procs of columnar storage
   * Reduced I/O 
   * Improved CPU cache performance:
     Under analysis workloads, all values of the same attribute are often accessed
     consecutively, which can fully utilize the modern CPU cache.
   * Improved compression
     Values with the same type are stored together, which means there will be
     multiple repeated values, they can be compressed efficiently. 
   * Vector processing
     Vector processing allows a CPU oeration to be applied in parallel on a number
     of elements. Query like `SELECT` usually computes with a constant, vector
     processing can do such a compution for multiple values at the same time.

2. Cons of columnar storage
   
   > The analysis of cons is mainly related to the worklaod, for transaction
   > processing(OLTP), they have the following drawbacks.

   * Cost of tuple reconstruction
     Attributes are sepearated and stored, to turn them back, we need to 
     reconstruct them.

   * Cost of tuple deletion and update
     In transaction processing, deletion and update are quite frequent, this
     is hard to implement in columnar storage.

     > In analysic processing(OLAP), data is seldomly being deleted or updated,
     > columnar storage is pretty suitable.

   * Cost of decompression
     Multiple column values are compressed as a unit, which means that, to retrive
     one of these values, you have to de-compress them all.

     In analysis secenaio, this is fine as multiple values need to be fetched.

     In transaction processing, only a few records need to be accessed, but to
     be able to de-compress and retrieve the needed data, we have to access some
     irrelevant column values.


# 13.7 Storage Organization in Main-Memory Databases
# 13.8 Summary
