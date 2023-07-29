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
   full, we use the next available bucket (cyclic order).

## 24.5.2 Dynamic Hashing

1. We need to increase or shrink the structure **gradually** (incrementally) so 
   that the database won't needs to be stopped at runtime.

   > minimize the cost of rehashing.

### 24.5.2.1 Data Structure

> In this section, we will introduce 
> [`Extendable Hashing`](https://en.wikipedia.org/wiki/Extendible_hashing)
>
> Implementation [here](https://github.com/SteveLauC/ExtendableHashMap)

1. Overview of an Extendable Hash Structure

   ![diagram](https://github.com/SteveLauC/pic/blob/main/IMG_1722.jpg)

   The table on left is called `bucket address table` or `directory`, it has 
   pointers pointing to the buckets on the right.

2. Pros of Extendable Hashing
   
   1. It copes with changes in database size by **splitting and coalescing [bucket]s**

   2. For split, every time, ONLY one bucket will be splitted and rehashed (i.e., 
      the overflow one), other buckets are not affected

      > Rehashing is incremental, which is lightweight.

3. Properties of an extendable hash structure
   
   1. A bucket may have more than one pointer pointing to it if the local depth
      is smaller than the global depth.

      Specifically, every bucket should have `2^(global_depth - local_depth)`
      pointers.

      > local depth should always be smaller than or equal to global depth.

   2. When a bucket becomes full(buckets have limited capacity), we split it 
      and increase the local depth by 1, global depth will also be increased 
      if the local depth equals to the global depth.

   3. Buckets are created **on demand**, say the global depth is `i`, we don't
      necessarily have `2^i` buckets.

      > A new bucket is created when a bucket overflow happens.
      
   4. Several directory entries that have the same prefix (or suffix, depends 
      on your implementation) will point to the same bucket, the local depth 
      of that bucket is the length of that common prefix (or suffix).

### 24.5.2.2 Queries and Updates

1. Locate the bucket
   
   hash(T), and use its first-i (i is the global depth) bits, look at the 
   corresponding directory entry, then follow the bucket pointer.

2. Look-up

   1. Locate the bucket
   2. Sequentially scan the items in the bucket.
    
      > To make the look-up fast, every bucket should have few entries.

3. Split the Bucket `Bj`

   1. If the local depth (call it j) is smaller than the global depth

      > There are more than one directory entry pointing to this bucket.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-07-09%2018-29-35.png)

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-07-09%2018-53-02.png)

      1. Increase the local depth (`j`) by 1
      2. `tmp_bits` = `Bj`'s bits
      3. Update `Bj`'s bits to `tmp_bits` + `0`
      2. Create a the bucket (`Bz`) with 
         1. local depth set to `j`
         2. `bits` set to `tmp_bits` + `1`
      3. Leaves the first half directory entries pointing to `Bj`, make the other
         half directory entries point to the newly created bucket (`Bz`) 

         > These entries are **consecutive**.
         >
         > Taking index of `Bj`'s value, for example, global depth is 3, `Bj`'s
         > `bits` is `[1]`, index will be in range `[4, 7]`.
         >
         > Directory entries that are in range `[4, 5]` should point to `Bj` 
         > (remain untouched), the other half should point to the newly allocated
         > `Bz`.

      5. Rehash the items in bucket `Bj`

         These items, either they go into the bucket `Bj`, or they go into the 
         newly created bucket `Bz`.

   2. If the local depth equals to the global depth

      > ONLY one entry in the directory entry points to bucket `Bj`

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-07-09%2017-09-15.png)

      1. Increase the global depth by 1.
      2. Increate the local depth `j` by 1.
      3. `tmp_bits` = `bits` of `Bj`
      4. Update `bits` of `Bj` with `tmp_bits` + `0`
      5. Allocate a new bucket, with `bits` set to `tmp_bits` + `1`
      6. Double the directory entries.

         You can see that we are prepending `0` and `1` to the old entries.

         ```
         # numeric value: (0, 1) -> (0, 1, 2, 3)
         [0, 1] -> [00, 01, 10, 11] 

         # numeric value: (0, 1, 2, 3) -> (0, 1, 2, 3, 4, 5, 6, 7)
         [00, 01, 10, 11] -> [000, 001, 010, 011, 100, 101, 110, 111] 
         ```

         > Benefits of doing so:
         >
         > 1. This enables us do appending instead of inserting.
         > 2. The numeric values are continuous, and we don't need to manipulate
         >    bits string, number-calculation is fast.
         >
         > Drawback
         >
         > 1. Entries for `Bj` and `Bz` are not adjacent.
         > 2. You need to

      7. Update bucket pinters in the bucket directory
        
        1. Move (clone) buckets to a temporary place
        2. Iterating over the buckets, calculate their value
        3. Use theirs values as indexes, update directory entries' bucket pointers.

      8. Rehashing the entries in the bucket `Bj`

         These items, either they go into the bucket `Bj`, or they go into the 
         newly created bucket `Bz`.
   
   3. Try locating the bucket for `key` and inserting it again
    
      > This `key` should go to either bucket `Bj` or `Bz`.

      If the bucket is still full, call split again (recursion).

3. Insertion(key, value)

   1. Locate the bucket, call it `Bj`.
   2. If the bucket is not full, insert to it and return.
   3. If the bucket is full, do split.

      > split is a recursive process, the split function will be called until 
      > `key` is inserted into the map.
      >
      > What a bad hash algorithm.
      
4. Deletion

   1. Locate the bucket, call it `Bj` 
   2. Remove the `(key, value)` from `Bj`
   3. Let's see if we can remove this bucket and coalesce it with its sibling:

      If the following conditions are satisfied:

      1. This bucket's local depth is greater than or equal to 2
      2. This bucket's sibling bucket exists

         > Sibling buckets have the same local depth but differ in their last 
         > bit, and the directory entries for siblings are always consecutive.

      3. The items of this bucket and its sibling can fit into one bucket

         > In real life, we won't merge two sibling buckets unless they are
         > less than full, or they will probably get a split in the next 
         > insertion.

      Then we can coalesce it and its sibling by:

      1. Append the `(key, value)` of the bucket at the back to the bucket at 
         the front, remove **the bucket at the back**.

         > In my impl, they are in an array, removing the last one is more 
         > lightweight. 
         >
         > And directory entries store the index of its bucket,
         > remove a bucket would invalidate these indexes. Using real pointers
         > won't suffer from this.

      2. Update the directory entry pointing to the back bucket to the front one.
      3. Decrease the bucket at the front's local depth by 1
      4. Update the invalid directory entries 

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-07-27%2010-47-43.png)

   4. Cut the directory entry in half

      > This operation is expensible!!! You may only want to do this when a big 
      > amount of items have been removed from the map.

      After the coalescence, if all the bucket has the local depth that is 
      `global depth - 1`, then every two directory entries should point
      to the same bucket.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-07-27%2011-09-28.png)
      
### 24.5.2.3 Static Hashing vs. Dynamic Hashing

1. Pros and Cons of Extendable Hashing

   * Pros

     1. It supports dynamic bucket allocation and thus suitblae for time-sensitive
        applications like file system and database.

   * Cons

     1. It adds another indirection layer, you have to access the directory 
        entry beforing accessing the bucket.
         
        > The traditional HashMap is basically something like:
        > 
        > ```rust
        > struct HashMap<K, V> {
        >     buckets: Vec<Vec<(K, V)>>,
        > }
        > ```
        >
        > To access the bucket, you `hash()` the key, and calculate the bucket
        > index by moding the `self.bucket.len()`, then you get the bucket.

     2. Directory entry also has some storage overhead.

     3. Doubling the directory entry is also costly
     
## 24.5.3 Comparsion of Ordered Indexing and Hashing

For disk-based index, most DBMS prefer B+Tree over Hash Index because:

1. Support range query
2. When inserting nodes, the relation size increases gracefully. 

   Extendable hash index solves the issue of rehashing, but doubling the directory
   entry is still condiered heavy.

For memeory index where range query is not common, hash index can still be used.


  
