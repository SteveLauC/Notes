> * 24.1 Bloom Filter
> * 24.2 Log-Structured Merge Tree and Variants
> * 24.3 Bitmap Indices
> * 24.4 Indexing of Spatial Data
> * 24.5 Hash Indices
>   * 24.5.1 Static Hashing
>     * 24.5.1.1 Hash Functions
>     * 24.5.1.2 Handling of Bucket Overflows
>   * 24.5.2 Dynamic Hashing
>     * 24.5.2.1 Data Structure
>     * 24.5.2.2 Queries and Updates
>     * 24.5.2.3 Static Hashing vs. Dynamic Hashing
>   * 24.5.3 Comparsion of Ordered Indexing and Hashing

# 24.1 Bloom Filter
# 24.2 Log-Structured Merge Tree and Variants
1. B+Tree is not suitable for workload where there are a very number of writes.

2. The key idea of LSM is to replace random I/O in CRUD with a smaller number
   of sequential I/O.

3. MemTable

   This memtable is implemented using B+Tree?

# 24.3 Bitmap Indices
# 24.4 Indexing of Spatial Data
# 24.5 Hash Indices
## 24.5.1 Static Hashing
### 24.5.1.1 Hash Functions

1. An ideal hash function distributes the stored keys uniformly across all the
   buckets.

### 24.5.1.2 Handling of Bucket Overflows

1. `Bucket Overflow` can happen for the following reasons:

   1. Insufficient buckets

   2. Skew
      
      Some buckets are assigned more records than are others. It can occur for
      2 reasons:
      
      1. Multiple records have the same search-key
      2. Bad hash function

2. To handle `Bucket Overflow`

   1. Allocate engouth `bucket`s

      > More spcae wasted, but the probability of `Bucket Overflow` is reduced.

   2. Use Overflow Bucket

3. `linear probing`

   > Visualization: https://www.cs.usfca.edu/~galles/visualization/ClosedHash.html

   This is one way to handle collisions with `Closed Hashing`, if a bucket is 
   full, we use the next available bucket (cylic order).

## 24.5.2 Dynamic Hashing

1. We need to increase or shink the structure **gradually** (incrementally) so 
   that the database won't needs to be stopped at runtime.

   > minimize the cost of rehashing.

### 24.5.2.1 Data Structure

> In this section, we will introduce 
> [`Extendable Hashing`](https://en.wikipedia.org/wiki/Extendible_hashing)

1. Overview of an Extendable Hash Structure

   ![diagram](https://github.com/SteveLauC/pic/blob/main/IMG_1722.jpg)

2. Pros of Extendable Hashing
   
   1. It copes with changes in database size by **splitting and coalescing [bucket]s**

   2. For split, every time, ONLY one bucket will be splitted (i.e., the overflow
      one), other buckets are not affected.

      Rehashing is incremental, which is lightweight.

3. Properties of an extendable hashset
   
   1. A bucket may have more than one pointer pointing to it if the local depth
      is smaller than the global depth.

      Specifically, every bucket should have `2^(global_depth - local_depth)`
      pointers.

   2. When a bucket becomes full, we split it and increase the local depth by 1,
      and the global depth will also be incremented.

   3. Buckets are created **on demand**, say the global depth is `i`, we don't
      necessarily have `2^i` buckets.
      
   4. Several directory entries that have the same prefix (or suffix) will point
      to the same bucket, the local depth of that bucket is the length of that 
      common prefix (or suffix).

### 24.5.2.2 Queries and Updates
### 24.5.2.3 Static Hashing vs. Dynamic Hashing
## 24.5.3 Comparsion of Ordered Indexing and Hashing
