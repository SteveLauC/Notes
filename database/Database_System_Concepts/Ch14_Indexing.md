> * 14.1 Basic Concepts
>
> * 14.2 Ordered Indices
>   * 14.2.1 Dense and Sparse Indexes
>   * 14.2.2 Multilevel Indices
>   * 14.2.3 Index Update 
>     * 14.2.3.1 Insertion
>     * 14.2.3.1 Deletion
>
>   * 14.2.4 Secondary Indices
>   * 14.2.5 Indices on Multiple Keys
>
> * 14.3 B+Tree Index Files
>   * 14.3.1 Structure of a B+Tree
>   * 14.3.2 Queries on B+Tree
>   * 14.3.3 Updates on B+Tree
>     * 14.3.3.1 Insertion
>     * 14.3.3.2 Deletion
>   * 14.3.4 Complexity of B+Tree updates
>   * 14.3.5 Nonunique Search Keys
>
> * 14.4 B+Tree Extensions
> * 14.5 Hash Indices
> * 14.6 Multiple-key Access
> * 14.7 Creation of Indices
> * 14.8 Write-Optimized Index Structures
> * 14.9 Bitmap Indices
> * 14.10 Indexing of Spatial and Temporal Data
> * 14.11 Summary


# 14.1 Basic Concepts

0. Relation between:
   
   > Read the other notes first if you have no idea what these concepts are.

   1. Clustering Index
   2. Nonclustering Index
   3. Dense Index
   4. Sparse Index

   Rules:

   > Think this in the perspective of whether the index works or not.

   1. Clustering Index can be Dense with absolutely no problem
   2. Clustering Index can be Sparse as entries have the same order as index
      entries

   3. Nonclustering Index must be Dense
   4. Nonclustering Index cannot be Sparse as 

      1. Entries and index entries have different orders
      2. We ONLY have partial data on where some enries are stored on the disk.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-19%2020-57-54.png) 


1. Keeping a sorted list of entries would NOT work well on large databases, for
   the reasons that:

   1. The index itself wuold be very big
   2. Find a student can still be rather time-consuming

      > Temporal Complexity of `binary search` is `O(log n)`, which theoretially
      > should be the best choice, but pratically, with big volume of data, I 
      > guess it won't work that well?

   3. Updating such a sorted list would be expensive.

2. General categories of indexes

   1. Ordered Index
   2. Hash Index

3. How to evaluate an index
   
   1. Access types: the types of access that are supported efficiently

      > See [doc of index of PostgreSQL](https://www.postgresql.org/docs/current/indexes.html)

   2. Access Time: The time it takes to find a particular data item, or set
      of items.

   3. Insertion time: The time it takes to insert a new data item.

      Including:
      1. the time it takes to find the correct place
      2. the time it takes to update the index structure

   4. Deletion time: The time it takes to delete a data item

      Including:
      1. the time it takes to find the item to be deleted
      2. the time it takes to update the index structure
      
   5. Space overhead: The additional space occupied by an index structure

4. Search Key

   An attribute or set of attributes used to look up records in a file is 
   called a `Search Key`.

   > We should note that this definition of `key` differs from that used in
   > `primary key/candidate key/superkey`.

   Index is built around the `Search Key`, each index is associated with a 
   `Search Key`, if there are several indexes on a table, there are several 
   `Search Key`s.
   
# 14.2 Ordered Indices

1. What is `Ordered Index`

   An Ordered Index stores the values of the search keys in sorted order.

2. What is `Clustering Index/Primary Index/Clustered Index`

   The entries stored in a table may themselves be sorted in some sorted order,
   just as books in a library are stored according to some attribute such as
   the Dewey decimal number.

   A `Clustering Index` is an index whose `Search key` also defines the 
   sequential order of the entries, i.e., relects the phycial order of how
   those entries are stored. `Clustering Index` is also called `Primary Index`.


   > The term `Primary Index` may refer to denote an index on `Primary key`,
   > in most cases, this is true, but it is not necessarily so.


3. What is `Nonclustering Index/Secondary Index/Nonclustered Index`?

   Index whose `Search key` specifies an order different from the phycial
   order is called `Nonclustering Index`, and is also called `Secondary Index`.

4. `index-sequential file`

   A file with a `Clustering Index` is called `index-sequential file`.

## 14.2.1 Dense and Sparse Indexes

1. Structure of an `Index Entry`

   An `Index Entry` basically consists of two parts:
   1. Search-Key value
   2. Pointer to one or more entries with that value as their Search-Key 
      value.

   ```rust
   struct IndexEntry {
       search_key_val: val,
       ptr: (PageID, SlotID),
   }
   ```

2. Categories of an `Ordered Index`

   1. Dense index

      In a dense index, an index entry appears for **every** search-key value in 
      the file.

      > It is for every search-key value instead of every tuple (i.e., search-key
      > is not necessarily the primary key)

      ![diagram](https://github.com/SteveLauC/pic/blob/main/dense_index.jpg)

      > In a dense clustering index, if there are multiple entries with the same
      > search-key value, then the index entry for that search-key would point
      > to **the first** entry with that search-key value.


   2. Sparse index

      > This is called `Partial Index` in PostgreSQL term.

      In a sparse index, an index entry appears for **only some** of the search
      key values.

      `Sparse Index` is quite restricted, to find entries that does not have
      index entries associated, the table must be sorted in the order of the 
      search key, i.e., **the index itself is a clustering index**.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/sparse_index.jpg)

      > To search an entry with a specific search-key value using `sparse index`,
      > you must find the index entry whose search-key value is less than or
      > equal to the target value, from the location pointed by this index entry,
      > scan sequentially until we find the desired record.

      > Question/TODO: If multiple entries with the same search-key value exist,
      > and an index entry for this search-key exists in the index, and the index
      > entry ONLY stores one pointer to the entry, which entry should this ptr
      > points to?
      >
      > The first entry? I guess.

3. Tradeoff when choosing ordered index

   Obviously, it is much faster to locate a record using dense index than 
   sparse index, but sparse index uses less space and is easier to maintain.

   So there is a tradeoff that the system designer must take between **access 
   time** and **space overhead**.

   In practice, it would be great if we have a sparse index that has an index
   entry for each disk block. The reason for this is that the dominant cost
   of processing a database query is the time that it takes to bring a block
   from disk into memory, after that, the time took by scanning a block in the
   memory is negligible as memory is pretty fast.

   > Generally I think this idea makes sense, but I cannot image how such an
   > index looks like, someting like this?
   >
   > ```rust
   > struct IndexEntry {
   >     search_key: Key,
   >     ptr: PageID,
   > } 
   > ```


## 14.2.2 Multilevel Indices

> We want our index to be in memory, if the data volume is big and we only have
> one layer of index, then it cannot be fetched into memory.

1. 没有什么是一层索引解决不了的，如果有，那就再加一层索引。

   ![diagram](https://github.com/SteveLauC/pic/blob/main/two_levels_index.jpg)

2. When the data volume becomes huge, our index becomes **too big** to be fetched
   into memory, and thus, needs **disk access** when accessing it, which is obviously
   **not what we want**.

   Assume our index takes `b` blocks on the disk, using binary search, [log2(b)]
   blocks need to be read for an index access, and such an access is **random I/O**
   as blocks accessed are **not adjacent**.

## 14.2.3 Index Update
1. Create

   Covered by `14.2.3.1 Insertion`

2. Remove
   
   Covered by `14.2.3.2 Deletion`

3. Update

   ```
   Update = Remove + Create
   ```

### 14.2.3.1 Insertion

1. How to do insertion in dense index
   
   1. If the new entry does not have an index entry in the index

      Just insert one
   2. An index entry with new entry's search-key value already exists in
      the index

      1. If entries are stored in the order of `search-key` and the index 
         entry stores the pointer to the first entry with that search-key 
         value,i.e., dense clustering index, index does not needs to be 
         changed, just append tnis entry to the place after other entries 
         with the same search-key.

      2. If that existed index entry stores multiple pointer to the entries
         with that search-key, append another pointer to the index entry.

2. How to do insertion in sparse index

   > TODO

### 14.2.3.2 Deletion

1. How to do deletion

   1. Use index to find the entry to be deleted.

   2. Update the index
      
      1. Dense index

         1. If the deleted entry is the only entry with that search-key value,
            delete this index entry.

         2. Otherwise
         
            1. dense clustering index, i.e., the index entry stores the 
               pointer to the first entry with that search-key value, if 
               the deleted entry is the first entry, then update the 
               corresponding pointer value.
            
            2. If the index entry stores pointers to all the entries with 
               that search-key value, delete the corresponding pointer.

      2. Sparse index

         1. If the index does not contain an index entry with that search-key
            value, do nothing.

         2. Otherwise
            
            1. If the deleted entry was the only entry with that search-key 
               value, update its index entry to the entry with the next
               search-key.

               If the entry with the next search-key already has its index
               entry, do nothing.

            2. If the index entry points to the deleted entry, update it.

## 14.2.4 Secondary Indices

1. Secondary Index must be dense.

## 14.2.5 Indices on Multiple Keys
1. So far, serch key of all examples given in the book has only one attribute,
   in general, a search key can have more than one attribute.

   A search key containing more than one attribute is referred to as `composite
   search key`:

   ```
   (a1, a2, a3, a4)
   ```

2. Comparsion of search keys that have more than one attribute

   > Basically a string comparsion

# 14.3 B+Tree Index Files

1. For ordered index, why is B+Tree preferred? 

   What about other data structures like:

   > Let's assume that the `IndexEntry` has a fixed length so that binary 
   > search can be used in some cases.
   >
   > To answer this question, you should note that index exists both in Memory
   > (with type system support)and disk (just some bytes).

   1. Sorted Array
      
      To insert a new entry in the middle, you have to move all the entries
      after the new one backwards, which will uses a lot of useless I/O.
      
      Same applies to removal.
      
   2. Sorted Linked List

      (On disk)To apply binary search to linked list, you have to find the 
      middle node, and this requires two pointer to traverse the list with 
      different speeds, which needs two sequential scans.

   3. Binary Search Tree
      
      Theoretically, BST has the best complexity when invoking CRUD, but in 
      memory, each node ONLY stores one entry, which requires a heap memory
      allocation and makes the performance bad.

      And incontiuous memory (tree) is not good for caching.

   4. B-Tree

## 14.3.1 Structure of a B+Tree

![diagram](https://github.com/SteveLauC/pic/blob/main/b%2Btree.jpg)

1. B+Tree is performant on lookup but not that good on insertion and deletion
   as it needs to re-balance itself.

   > This kind of trade-off is unavoidable.

2. Three kinds of nodes

   * Root node
   * Internal node(nonleaf node)
   * Leaf node

3. B+Tree is balanced, which means that every path from the root of the tree
   to a leaf node **has the same length**.

   All leaf nodes appear in the same level.

4. `order` and children amount bound
   
   We say that a B+Tree has `order` of `n` when its **internal node** has `n` 
   **children** at most (upper bound).

   1. Internal node, has `([n/2], n)` children to make the tree balanced.

      > `[x]` denotes that the smallest integer that is bigger than x, e.g., 
      > [1.9] = 2.

   2. Leaf node has 0 children, but leaf nodes in a B+Tree with order n have 
       [[(n-1)/2], n-1] search-key values.

      > You should note that `order` is the upper bound on the amount of 
      > children, leaf node has no children but is still kinda restricted 
      > under it.

   3. Root node can has
      1. 0 children when it is a leaf node
      2. Otherwise, [2, n) children

5. B+Tree can be seen as a multilevel index

6. A node(regardless of its kind) in B+Tree, if has n pointers, then it has
   n-1 search key values. For nodes other than leaf node, n pointers means 
   n children.

   If it is a leaf node, then the last pointer points to the next leaf node.

7. Search-key values within a node remains ordered.

   If `Li` and `Lj` are leaf nodes and `i<j`, then every search-key value in
   `Li` is less than every search-key value in `Lj`.

   Since all search-key values in leaf nodes are sorted, the last pointer in
   leaf node points to the next leaf node, enables us to do efficient binary
   search on them.

8. How to handle duplicate search-key values

   > This will be explained in detail in Section 14.3.5

   One common approach that almost every implementation does is to add the
   primary key to search-key to ensure the absence of duplciates.


## 14.3.2 Queries on B+Tree
## 14.3.3 Updates on B+Tree
### 14.3.3.1 Insertion
### 14.3.3.2 Deletion
## 14.3.4 Complexity of B+Tree updates
## 14.3.5 Nonunique Search Keys



# 14.4 B+Tree Extensions
# 14.5 Hash Indices
# 14.6 Multiple-key Access
# 14.7 Creation of Indices
# 14.8 Write-Optimized Index Structures
# 14.9 Bitmap Indices
# 14.10 Indexing of Spatial and Temporal Data
# 14.11 Summary
