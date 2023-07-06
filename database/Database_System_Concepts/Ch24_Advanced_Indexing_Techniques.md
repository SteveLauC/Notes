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

   This memtable is implemented using B+Tree

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

   2. Overflow Bucket

      > Open Hashing (Closed Addressing)

3. `linear probing`

   This is one way to handle collisions with `Closed Hashing`, if a bucket is 
   full, we use the next available bucket (cylic order).

## 24.5.2 Dynamic Hashing
### 24.5.2.1 Data Structure
### 24.5.2.2 Queries and Updates
### 24.5.2.3 Static Hashing vs. Dynamic Hashing
## 24.5.3 Comparsion of Ordered Indexing and Hashing
