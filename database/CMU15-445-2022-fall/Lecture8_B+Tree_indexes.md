> Today's agenda
>
> * B+Tree Overview
> * Use in DBMS
> * Design Choices
> * Optimizations

> Read 
> [DBSC: Ch14 note](https://github.com/SteveLauC/Notes/blob/main/database/Database_System_Concepts/Ch14_Indexing.md)
> for more information about B+Tree.

# B+Tree Overview & Use in DBMS

0. Family of B-Tree

   * B-Tree (1971)
   * B+Tree (1973)
   * B*Tree (1977?)
   * B link-Tree (1981)

   > Nowadays, B+Tree borrows things from other tree structures.

1. Properties of a B+Tree

   1. It is perfectly balanced (All the leaf nodes are in the same level)
   2. Every node other than root is at least half full (i.e., amount of keys range: `[M/2, M-1]`)
   3. Every internal node with `k` keys has `k+1` non-null children
   4. When used as an index:
      1. Every node is an array of key-value pairs.
      2. The keys are derived from the attributes that the index is based on(search key)
      3. The values will differ based on whehter it is a internal node or leaf node.
      4. The array is (usually) kept in order (for binary search)

2. What should be stored in the leaf node:

   1. Pointer (Record ID)

      A pointer to the location of the tuple to which the index entry corresponds. 

      > Then you have to do a second look-up to fetch the actual data.

      > Implementations that use this approach:
      > * PostgreSQL
      > * SQL Server

   2. Actual data

      > B+Tree file organization 

      We store the actual data in leaf nodes

      > Implementations that use this approach:
      > * SQLite
      > * MySQL 

3. Advantage of using B+TREE

   In B-TREE, each key ONLY appears once in the tree, which is obviously more
   space-efficient. But when traversing the tree, you need to jump up and down,
   accessing different pages and thus causing more random access.

   In B+TREE, all the data are stored in the leaf nodes, above problems naturally
   disappears.

4. Insertion in a B+TREE

   > Visualization of the operations on a B+TREE: 
   > https://dichchankinh.com/~galles/visualization/BPlusTree.html

   1. Find the leaf node `L`.
   2. Try to insert entry to `L` in a sorted order.
      1. If `L` has enough space, we are done.
      2. Insert this entry is inserted to `L`, then split this node by
         this entry into two nodes, then insert the entry to the parent node, make 
         the two newly-split nodes be the left and right children of the node that 
         is just pushed to the parent node.

         > If the parent node exceeds it capacity after the insertion of that 
         > entry, do the same thing with the parent node.

    > B+TREE grows by spliting the leaf nodes and inserting nodes to the parent 
    > node. In other words, this tree grows from bottom up. (The depth of a B+TREE
    > gets increased when the root node is split)

5. Deletion in a B+TREE
   1. Find the leaf node `L` that contains this entry
   2. Remove the entry
      1. If `L` is at least half full, done. 
      2. If `L` has ONLY `M/2-1` entries (we have to reblance it)
         1. Try to borrow from sibling (adjacent node that has the same parent 
            with `L`) if this won't make sibling un-balanced (less than half full)
         2. If borrow fails, merge `L` and sibling, and delete the entry (pointing
            to `L` or its sibling) from the parent node. 

6. For an ordered index, a query can be speed up if the attributes selected
   is a subset of the search key. For example, `index on (a, b, c)`, the
   following queries can utilize this to accelerate the query:
   * (a = 1 AND b = 2 AND c = 3)
   * (a = 1 AND b = 2)
   * (c = 3)

   But for a hash index, ONLY queries that have ALL the attributes in search 
   key can be boosted, i.e., a query that exactly selects attributes `a, b, c`.
   

7. How to handle duplicate keys in B+TREE

   1. Append RecordID

      Add the tuple's unique RecordID as part of the search key to ensure that 
      the keys are unique.

      > The DBMS can still use partial key to find the tuple.
      > 
      > Then if the key for which we are building index includes RecordID, then
      > the key will never be duplicate? Yes.

   2. Overflow Leaf Nodes

      Allow leaf nodes to spill into overflow nodes that contain the duplicate
      keys.

      > This is more complex to maintain and modify

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-06-25%2011-00-10.png)

      > This is kinda similar to how we handle collision in Open Hashing HashMap.

8. What is `Clustered Index`

   A clustered index is a special kind of index that reorders the way that
   records are phycially stored on the disk. And ONLY one clustered index 
   can exist for a table.


9. Sequential iterting over entries using a non-clustering index can be 
   inefficient

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-06-25%2011-42-33.png)

   As we may access one page more than once. We can collect the pageIDs first, and
   sort the pageIDs, then access them.

   > This is a simple query optimization

   > Wait, we sort them so we no longer access entries in the order of that 
   > non-clustering index...


> The following stuff is actually in the next video, but I just put it here.

# Design Choices

1. Node Size

   General idea should be, the slower the device is, the larger the node should 
   be as we want more sequential I/O.

   * HDD: 1MB
   * SSD: 10KB
   * Memory: 512B

   > In some enterprise system, one can have different page sizes for different
   > components.

2. Merge Threshold

   In B+Tree, a node that is less than half-full should be merged with other 
   nodes or borrows some entry from the other node. But sometimes you may want
   to delay such operations so that it won't be split in the near future.

3. Var-len keys

   1. Pointers

      Store a pointer to the key instead of the key itself, you don't want to 
      do this in a disk system as everytime you access a node, you have to
      access the address pointed by that pointer, which is a random access.

   2. Var-len node

      Also a bad idea as these will be fragments.

   3. Padding

      Always pad the key to its type's max length.

      Say I build an index on varchar(32), always store 32 bytes for the key no
      matter what the size of the key it.

      > MySQL does this.

   4. key map / indirection (Most common one)

      Store a HashMap in the node mapping a fix-sized integer to the key, then
      store the key in the node.

      > We have seen this in the compression class.


4. Intra-Node Search (Search within the node)

   1. Linear search

      > Slow but can be sped up by SIMD
      >
      > In memory, if your array is short, then linear can be fast for the cache.

   2. Binary Search 

   3. Interpolation
      
      Approximate location of desired key based on known distribution of keys.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-03%2008-54-39.png)

      Find 8 in the above node.

      > This is the fastest one if you can implement it, Andy said he hasn't 
      > seen any non-academic databases that have this implemented.


# Optimizations

1. Pointer swizzing

   On disk, a node's pointers are alwasy `PageID`s, say I have two pages in the 
   memory buffer pool, one is the parent of the other one, when accessing the 
   child node, we follow the `PageID` stored in the parent node, access the 
   `PageTable` (If you have no idea what the `PageTable` is, read the note of the
   lecture 6, it is the metadata for the buffer pool), find the corresponding
   memory frame.

   If a page is **pined** in the memory, we can store raw addresses instead 
   of `PageID`s, to avoid `PageTablel` accesses.

   > The PageTable possibly is behind a lock, this optimization could avoid 
   > a access to the lock.

2. Bulk Insertion (Bulk Loading)

   The fastest way to construct a B+Tree is to:
   1. Sort the items
   2. Insert them

   With doing this, we will always access a page for ONLY one time. This advantage
   will be magnified if the relation we are building index against is huge that
   a page will be evicted after loading into memory and accessing it.
