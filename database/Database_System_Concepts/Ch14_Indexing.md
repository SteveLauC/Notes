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
> one layer of index, then it probably cannot be fetched into memory.

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
   > (probably with type system support?)and disk (just some bytes).

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

   And the depth of a B+Tree **ONLY changes** when you modify the Root node.

4. `order` and children amount bound
   
   We say that a B+Tree has `order` of `n` when its **internal node** has `n` 
   **children** at most (upper bound).

   > And the minimal `order` of a B+Tree is 2. If `order` is 1, then it will be
   > a link list. And for a B+Tree stored on the disk, you may want the `order`
   > to be as large (storing more children in a disk page) as possible to make 
   > the disk read more efficient

   1. Internal node, has `([n/2], n)` children to make the tree balanced.

      > `[x]` denotes that the smallest integer that is bigger than x, e.g., 
      > [1.9] = 2.

   2. Leaf node has 0 children, but leaf nodes in a B+Tree with order `n` have 
      `[[(n-1)/2], n-1]` search-key values.

      > You should note that `order` is the upper bound on the amount of 
      > children, leaf node has no children but is still kinda restricted 
      > under it.

   3. Root node can has
      1. 0 children when it is a leaf node
      2. Otherwise, [2, n) children

         > Why it is gt or eq to 2 here? Think about a B+tree with oder 2, it 
         > becomes a nonleaf node ONLY when there are 3 entries in the tree,
         > when the third entry comes, the B+tree branches, now the root node 
         > has 2 children(from 0 to 2).

5. B+Tree index can be seen as a multilevel index

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

9. Some properties of B+Tree

   1. Leaf nodes are always leaf nodes, internal nodes are always internal nodes.

      > The first node in a B+Tree is both leaf node and root node.

   2. B+Tree grows by splitting the Root node.

   3. One difference between B+Tree and B-Tree is that B+Tree has duplicated 
      keys, such a duplication occurrs when **splitting leaf nodes**.

      > Splitting non-leaf node won't result in a duplicated key as the selected
      > key will ONLY be inserted into the parent node.


## 14.3.2 Queries on B+Tree

> B+Tree supports:
>
> 1. Equality query
>     
>    ```
>    =
>    ```

> 2. Range query
>    ```
>    < <= > >=
>    ```

1. How to do equality query on a B+Tree

   > The following rust code should be seen as a pseudocode for the following
   > reason:
   >
   > 1. Pointer on leaf nodes should be something like this:
   >    ```rust
   >    struct LeafNodePtr {
   >        /// file path and offset 
   >        page_id: (PathBuf, u64),
   >        /// slot id
   >        slot_id: u64,
   >    }
   >    ```
   >
   > 2. Whether a pointer should be null needs futuer research.

   ```rust
   use std::ops::Deref;

   #[derive(PartialEq)]
   enum NodeKind {
       Root,
       Internal,
       Leaf,
   }

   struct Node {
       search_keys: [i32; Node::ORDER - 1],
       ptrs: [Box<Option<Node>>; Node::ORDER],
       kind: NodeKind,
   }

   impl Node {
       const ORDER: usize = 3;
   }

   struct BPlusTree {
       root: Node,
   }

   impl BPlusTree {
       fn find_search_key(&self, key: i32) -> Option<&Node> {
           let mut curr_node = &self.root;

           // We traverse until reaches leaf nodes
           while curr_node.kind != NodeKind::Leaf {
               // In the `curr_node`, find the first search-key value that is
               // bigger than or equal to `key`.
               if let Some(idx) = curr_node
                   .search_keys
                   .iter()
                   .position(|search_key| *search_key >= key)
               {
                   // for search key `ki`, search-key values of the nodes on 
                   // subtree pointed by `pi` should be **smaller than** `ki`. 
                   // And search-key values on nodes pointed by `p i+1` should
                   // be greater than or equal to `ki`.

                   // find a search key that is > key
                   if curr_node.search_keys[idx] > key {
                       curr_node = curr_node.ptrs[idx]
                           .deref()
                           .as_ref()
                           .expect("Probably should be non-null");
                   } else {
                   // find a search key that is = key
                       curr_node = curr_node.ptrs[idx + 1]
                           .deref()
                           .as_ref()
                           .expect("Probably should be non-null");
                   }
               } else {
                   // Every search-key in `curr_node` is smaller than `key`,
                   // Let's check the rightmost child node.

                   if let Some(rightmost_non_null_ptr) =
                       curr_node.ptrs.iter().rev().find(|ptr| ptr.is_some())
                   {
                       curr_node = rightmost_non_null_ptr
                           .deref()
                           .as_ref()
                           .expect("guaranteed to be non-null");
                   }
               }
           }

           if let Some(found_idx) = curr_node
               .search_keys
               .iter()
               .position(|search_key| *search_key == key)
           {
               Some(
                   curr_node.ptrs[found_idx]
                       .deref()
                       .as_ref()
                       .expect("Leaf node's ptr can not be null"),
               )
           } else {
               None
           }
       }
   }
   ```

2. Find  all tuples in range [low_bound, upper_bound]

   The basic algorithm is:
   1. Traverse down to the leaf node that possibily contains `low_bound`
   2. Starting from this leaf node, iterate over all the leaf nodes and collect
      all pointers with search key `key` s.t. `low_bound <= key <= upper_bound`.

   
   ```rust
   use std::ops::{Deref, Range};

   #[derive(PartialEq)]
   enum NodeKind {
       Root,
       Internal,
       Leaf,
   }

   struct Node {
       search_keys: [i32; Node::ORDER - 1],
       ptrs: [Box<Option<Node>>; Node::ORDER],
       kind: NodeKind,
   }

   impl Node {
       const ORDER: usize = 3;
   }

   struct BPlusTree {
       root: Node,
   }

   impl BPlusTree {
       /// Assume `key` exists in this B+Tree, return the leaf node containing it.
       fn find_leaf_node(&self, key: i32) -> &Node {
           let mut curr_node = &self.root;

           while curr_node.kind != NodeKind::Leaf {
               if let Some(idx) = curr_node
                   .search_keys
                   .iter()
                   .position(|search_key| *search_key >= key)
               {
                   if curr_node.search_keys[idx] > key {
                       curr_node = curr_node.ptrs[idx]
                           .deref()
                           .as_ref()
                           .expect("Probably should be non-null");
                   } else {
                       curr_node = curr_node.ptrs[idx + 1]
                           .deref()
                           .as_ref()
                           .expect("Probably should be non-null");
                   }
               } else {
                   if let Some(rightmost_non_null_ptr) =
                       curr_node.ptrs.iter().rev().find(|ptr| ptr.is_some())
                   {
                       curr_node = rightmost_non_null_ptr
                           .deref()
                           .as_ref()
                           .expect("guaranteed to be non-null");
                   }
               }
           }

           curr_node
       }

       /// Find search-key `key`
       ///
       /// Return `None` if it does not exist in this B+Tree.
       fn find_search_key(&self, key: i32) -> Option<&Node> {
           let leaf_node = self.find_leaf_node(key);

           if let Some(found_idx) = leaf_node
               .search_keys
               .iter()
               .position(|search_key| *search_key == key)
           {
               Some(
                   leaf_node.ptrs[found_idx]
                       .deref()
                       .as_ref()
                       .expect("Leaf node's ptr can not be null"),
               )
           } else {
               None
           }
       }

       fn find_range(&self, range: Range<i32>) -> Vec<&Node> {
           let lb = range.start;
           let up = range.end;
           let mut ret = Vec::new();

           let mut leaf_node = self.find_leaf_node(lb);

           loop {
               ret.extend(
                   leaf_node
                       .search_keys
                       .iter()
                       .enumerate()
                       .filter(|(_, &search_key)| search_key >= lb && search_key <= up)
                       .filter(|(idx, _)| leaf_node.ptrs[*idx].deref().is_some())
                       .map(|(idx, _)| {
                           leaf_node.ptrs[idx]
                               .deref()
                               .as_ref()
                               .expect("guaranteed to be non-null")
                       }),
               );

               if let Some(next_leaf_node) = leaf_node
                   .ptrs
                   .last()
                   .expect("should have at least one element")
                   .deref()
                   .as_ref()
               {
                   leaf_node = next_leaf_node;
                   continue;
               } else {
                   break;
               }
           }

           ret
       }
   }
   ```

3. Cost of querying on a B+Tree
   
   When querying a B+Tree, we traverse a tree from the root node to the leaf node,
   assume it this tree has N entries and order of `n`, typically this path has 
   length that is not longer than `[log[n/2] N]`.

   > Remeber that the amount of children of a non-leaf node is `[n/2]`

   On disk, each node takes a disk page, assume that the search key is fix-sized
   and takes 12 bytes, and a disk pointer has size 8, then a index entry takes 20
   bytes, a disk page (assume it is 4000 bytes), can accommodate 200 index entries,
   i.e., n = 200, `[log[n/2] N]` = `[log100 N]`. If we have 1 million index entries
   in the index file, this value is [log100 100_000_000] = 4, which means 4 pages
   need to accessed in this query. In practise, the root node is usually heavily 
   accessed, and thus will be fetched in memory, then we only need to access 3
   nodes.

   After traversing down to the leaf node, we find the appropriate pointer, 
   accessing the contents pointed by that pointer needs another random I/O.

4. An important distinction between in-memory tree and disk tree

   For disk tree, the node size is typically the size of a disk page, and thus
   can have more entries accommodated, which means the tree is usually fat and
   shot, and thus takes less steps to traverse.

5. When dealing with duplicate search-key using composite search-key such as
   adding primary key to the search-key, how to do query in such case?

   Assume the search-key is `ai`, it is not unique, we add the PK to it: `(ai, PK)`,
   to search entries with `ai` set to `v`, we use `find_range(lb, up)` with `lb`
   set to `(v, -∞)` and `up` set to `(v, +∞)` respectively.

## 14.3.3 Updates on B+Tree

> We ONLY focus on insertion and deletion as update can be modeled by insertion
> and deletion.
>
> If you are building a set backed by B+Tree (i.e., all the operations are 
> operated on keys), you cannot modify a key in place as modification to its 
> value will change its position. HashSet also suffers from this problem, 
> changes on keys will potentially put it into another bucket(No `get_mut()`
> method).
>
> If you are building a Map, then in-place modification should be fine as changes
> are operated on values. 

1. Insertion and deletion are more expensive than lookup as a B+tree needs to
   rebalence itself by spliting (more than `order`) or coalescing(fewer than [n/2])
   nodes.

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
