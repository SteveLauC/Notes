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
>   * 14.4.1 B+Tree File Organization
>   * 14.4.2 Secondary Indexes and Record Relocation
>   * 14.4.3 Indexing Strings
>   * 14.4.4 Bulk Loading of B+Tree Indexes
>   * 14.4.5 B-Tree Index Files
>   * 14.4.6 Indexing on Flash Storage
>   * 14.4.7 Indexing in Main Memory
>
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

   The entries stored in a table **may themselves be sorted in some sorted order**,
   just as books in a library are stored according to some attribute such as
   the Dewey decimal number.

   A `Clustering Index` is an index whose `Search key` also defines the 
   sequential order of the entries, i.e., reflects the phycial order of how
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
      after the new one backwards, which will result in a lot of useless I/O.
      
      Same applies to removal.
      
   2. Sorted Linked List

      (On disk)To apply binary search to linked list, you have to find the 
      middle node, and this requires two pointer with different speeds to traverse
      the list, the fast pointer moves 2 steps each time and the slower one
      move one step, when the fast one reaches the end, the slower one will be
      in the middle node.

      This operation will sequentially scan the list, which possibily needs
      tons of random I/O.

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
   * Internal node (nonleaf node)
     Has children

   * Leaf node
     Has no children

   > Root node can be internal node and leaf node, which depends whether it has
   > children or not.

3. B+Tree is balanced, which means that every path from the root of the tree
   to a leaf node **has the same length**.

   All leaf nodes appear in the same level.

   And the depth of a B+Tree **ONLY changes** when you modify the Root node.

4. `order` and children amount bound
   
   We say that a B+Tree has `order` of `n` when its **internal node** has `n` 
   **children** at most (upper bound).

   > And the minimal `order` of a B+Tree is 3. If `order` is 1, then it will be
   > a link list. And for a B+Tree stored on the disk, you may want the `order`
   > to be as large (storing more children in a disk page) as possible to make 
   > the disk read more efficient

   > I am actually NOT sure about the minimal `order` B+Tree, in this
   > answer, the author states it is 3. I tried to insert `1, 2, 3` to a B+Tree
   > with order 2, and this results in an **empty**(has 0 key) internal node.
   > So I guess order of 2 is not allowed.

   1. Internal node, should have `[ceil(n/2), n]` children to make the tree balanced.

   2. Leaf node has 0 children, but leaf nodes in a B+Tree with order `n` have 
      `[ceil((n-1)/2), n-1]` search-key values.

      > You should note that `order` is the upper bound on the amount of 
      > children, leaf node has no children but is still kinda restricted 
      > under it.

   3. Root node can has
      1. 0 children when it is a leaf node
      2. Otherwise, `[2, n]` children

      > Why it is gt or eq to 2 here? Think about a B+tree with oder 2, it 
      > becomes a nonleaf node ONLY when there are 3 entries in the tree,
      > when the third entry comes, the B+tree **branches(split)**, now the 
      > root node has 2 children(from 0 to 2).

   > When updating a B+Tree, we split when a node becomes full, a leaf node is
   > full if it has `ceil(n-1)` search key values, an internal node is full if it
   > has `n` children(pointers).

5. B+Tree index can be seen as a multilevel index

6. A node(regardless of its kind) in B+Tree, if has `n` pointers, then it should 
   have `n-1` search key values. For nodes other than leaf node, `n` pointers means 
   `n` children.

   If it is a leaf node, then the last pointer points to the next leaf node. 
   The last leaf node ONLY has `n` pointers as it is the last one.

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

      > This is true because B+Tree grows by splitting the old Root and making
      > a new root.

      > The first node in a B+Tree is both leaf node and root node.

   2. B+Tree grows by splitting the Root node.

   3. One difference between B+Tree and B-Tree is that B+Tree has duplicated 
      keys, such a duplication occurrs when **splitting leaf nodes**.

      > Splitting non-leaf node won't result in a duplicated key as the selected
      > key will be moved into the parent node.

   4. B+Tree's self-rebalance is amazing 

      * Image inserting `[1, 2, 3, 4, 5]` to a BST

        ![BST](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-06-08%2016-24-37.png)

        This tree is absolutely not balanced, and actually, you will get `O(n)` 
        time complexity when searching this tree.

        > There is self-rebalancing BST, 
        > [AVL tree](https://www.cs.usfca.edu/~galles/visualization/AVLtree.html)
      
      * With a B+Tree

        ![B+Tree](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-06-08%2016-29-36.png)

        When splitting a full node, we always copy(or move) the selected (the 
        middle one) node to the parent node, which can be used to speed up the
        query.

        As you can see, the root node has entry 3, if you wanna find the values 
        smaller than 3, you know that they are on the left subtree.

## 14.3.2 Queries on B+Tree

> [B+Tree **set** implementation(no values, just keys)](https://github.com/SteveLauC/BPlusTreeSet)

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

   1. Since all values are stored in leaf nodes, we first traverse down to the
      leaf node that possibily contains our target value:

      ```rust
      fn traverse_to_leaf_node<Q>(&self, value: &Q) -> Node<T>
      where
          Q: PartialOrd,
          T: Borrow<Q>,
      {
          let mut ptr = Node::clone(&self.root);

          while !ptr.is_leaf() {
              let idx = ptr.search_non_leaf_node(value);
              let ptr_read_guard = ptr.read();
              let node = Node::clone(&ptr_read_guard.ptrs[idx]);
              drop(ptr_read_guard);

              ptr = node;
          }

          assert!(ptr.is_leaf());

          ptr
      }
      ```

   2. Within that leaf node, do binary search.
    
      ```rust
      /// Returns a `Rc` to the value in the set, if any.
      pub fn get<Q>(&self, value: &Q) -> Option<Rc<T>>
      where
          T: Borrow<Q>,
          Q: Ord,
      {
          let leaf = self.traverse_to_leaf_node(value);
          let leaf_guard = leaf.read();
          let idx = leaf_guard
              .keys
              .binary_search_by(|item| (item as &T).borrow().cmp(value))
              .ok()?;

          Some(Rc::clone(&leaf_guard.keys[idx]))
      }
      ```


2. Find all tuples in range `[low_bound, upper_bound]`

   The basic algorithm is:
   1. Traverse down to the leaf node that possibily contains `low_bound`
   2. Starting from this leaf node, iterate over all the leaf nodes and collect
      all pointers with search key `key` s.t. `low_bound <= key <= upper_bound`.

   > For range queries, B+Tree can do this easily since its leaf node stores
   > a pointer to the next leaf node. B-Tree cannot do this I guess.

3. Cost of querying on a B+Tree
   
   When querying a B+Tree, we traverse the tree from the root node to the leaf 
   node, assume this tree has N entries and order of `n`, typically this 
   path has length that is not longer than `[log ceil(n/2) N]`.

   > Remeber that the amount of children of a non-leaf node is `ceil(n/2)`

   On disk, each node takes a disk page, assume that the search key is fix-sized
   and takes 12 bytes, and a disk pointer has size 8, then a index entry takes 20
   bytes, a disk page (assume it is 4000 bytes), can accommodate 200 index entries,
   i.e., n = 200, `[log ceil(n/2) N]` = `[log100 N]`. If we have 1 million index entries
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
   rebalence itself by spliting (more than `order`) or coalescing(fewer than ceil(n/2))
   nodes.

### 14.3.3.1 Insertion

1. How to do insertion on a B+Tree

   1. Traverse to the leaf node that will contains this `value`
   2. If that leaf node contains `value`, return
   3. If the amount of search keys in that leaf node is smaller than `order - 1`, 
      insert `value` to it and return
   4. The leaf node is full, let's do split on it.

   ```rust
   /// Insert `value` into the set.
   ///
   /// Return `true` if insertion is successful.
   pub fn insert(&mut self, value: T) -> bool
   where
       T: Ord + Debug,
   {
       let order = self.order;
       let (leaf_node, parent_nodes) = self.traverse_to_leaf_node_with_parents(value.borrow());
       if leaf_node.contains(value.borrow()) {
           return false;
       }

       if !self.is_node_full(&leaf_node) {
           // have enough room, just insert
           leaf_node.insert_in_leaf(value);
       } else {
           // split leaf node
           //
           // 1. Create a new Node
           // 2. Move the entries in this `leaf_node` to `tmp`
           // 3. Insert `value` into `tmp`
           // 4. `leaf_node_plus.ptrs.last()` = `leaf_node.ptrs.last()`;
           //    `leaf_node.ptrs.last()` = `leaf_node_plus`
           //
           //     Since we are implementing a BPlusTreeSet, leaf node in such
           //     trees ONLY have one pointer, i.e., the pointer to the next
           //     leaf node.
           // 5. Move `tmp[0..(order/2)]` to `leaf_node.keys`
           // 6. Move the remaining elements in `tmp` to `leaf_node_plus`.
           // 7. Let `K'` be the smallest entry in `leaf_node_plus`, insert it
           //    and a pointer to `leaf_node_plus` to the parent node of `leaf_node`.
           let leaf_node_plus = Node::new(leaf_node.kind());

           let mut leaf_node_write_guard = leaf_node.write();
           let mut leaf_node_plus_write_guard = leaf_node_plus.write();

           let mut tmp = leaf_node_write_guard.keys.drain(..).collect::<Vec<Rc<T>>>();
           insert_into_vec(&mut tmp, value);

           if !leaf_node_write_guard.ptrs.is_empty() {
               assert_eq!(leaf_node_write_guard.ptrs.len(), 1);
               leaf_node_plus_write_guard.ptrs.push(
                   leaf_node_write_guard
                       .ptrs
                       .pop()
                       .expect("Should have exactly 1 ptr"),
               );
           }
           leaf_node_write_guard
               .ptrs
               .push(Node::clone(&leaf_node_plus));

           assert_eq!(leaf_node_write_guard.keys.len(), 0);
           let idx = (order as f64 / 2.0).ceil() as usize;
           leaf_node_write_guard.keys.extend(tmp.drain(0..idx));
           leaf_node_plus_write_guard.keys = tmp;

           // Duplication occurs here
           let k = Rc::clone(&leaf_node_plus_write_guard.keys[0]);
           drop(leaf_node_write_guard);
           drop(leaf_node_plus_write_guard);

           self.insert_in_parent(leaf_node, parent_nodes, (k, leaf_node_plus));
       }

       self.len += 1;
       true
   }
   ```

2. Split a **leaf** node
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/split_of_leaf_node_b_plus_tree.jpg)

   Take the above case as an example, the above B+Tree's order is 4, when a leaf
   node has entries `1, 2, 3`, and we want to insert `4` to it, the leaf node is
   full, we have to split it.

   1. Create a new `Node` that has the same kind with the split node.
   2. Move the values stored in the split node to a temporary place.

      > Since we are implementing a Set, not a key-value store, and a set's leaf
      > node ONLY has 1 or 0 pointer(to the next leaf node), so in this step, we
      > only need to move the keys.

      ```
      // tmp
      1, 2, 3
      ```

   3. Insert the value to be inserted to that temporary place.

      ```
      // tmp
      1, 2, 3, 4
      ```
   4. Migrate pointer if it exists
      
      If the split node contains a pointer, move this pointer to the new node.

   5. Update the split node's pointer to the new node

   6. `tmp` has `order` entries now, move `tmp[0, ceil(order/2))` to the split node,
      and the remaining ones to the new node.

   7. Clone the first entry in the new node

   8. Insert `(fist_entry_clone, pointer_to_new_node)` to the parent node of the
      split node.

3. Insert into parent node
   
   > To find the parent node of the split node, we collect the parent nodes that
   > are in our path when traversing down to the leaf node.

   1. If the split node has no parent node, i.e., the split node is the Root node,
      create a new Node, insert a pointer to the split node to it(Don't need this
      for the case where parent node already exists).

      Insert `(first_entry_clone, pointer_to_new_node)` to the new Root.

      Make this new root node the Root.

   2. Find the parent node of the split node. If the amount of its entries is
      smaller than the `order`, insert `(first_entry_clone, pointer_to_new_node)`
      to it and return.
   
   3. Alright, unforunately, split the parent node (non-leaf node)

4. Split a **non-leaf** node

   1. Create a new node, let's call it `parent_plus`.
   2. Create a temporary node `tmp`, move all the keys and pointers of the parent node
      to it.

   3. Insert `(fist_entry_clone, pointer_to_new_node)` to the tmp node.

   4. let idx = ceil(order/2)

   5. Move `tmp.keys[0, idx)` to the parent node, move `tmp.ptrs[0, idx]`to the
      parent node.

   6. Move the first entry(tmp[idx]) of `tmp` out, name it `k`

   7. Replace `parent_plus` with `tmp`.

   8. Recursion: Insert `k, pointer_to_parent_plus` to the parent of the parent node.


   ```rust
   /// Insert key and pointer `kp` to the parent node of `split`, i.e.,
   /// `parents.pop().unwrap()`.
   ///
   /// We have a vector of parent nodes as this operation can be recursive.
   ///
   /// # Recursion exits:
   /// 1. `parents.is_empty()`, which means that the root node has just been split.
   /// 2. Split is no more triggered.
   fn insert_in_parent(&mut self, split: Node<T>, mut parents: Vec<Node<T>>, kp: (Rc<T>, Node<T>))
   where
       T: Ord + Debug,
   {
       if parents.is_empty() {
           // We are gonna do insertion on the parent node of `split`, but
           // unfortunately it does not have a parent node, no worries, we can
           // create one for it.
           let new_root = Node::new(NodeKind::ROOT);
           let mut new_root_write_guard = new_root.write();
           new_root_write_guard.keys.push(kp.0);
           new_root_write_guard.ptrs.extend([split, kp.1]);
           drop(new_root_write_guard);

           self.root = new_root;
       } else {
           let parent_of_split = parents.pop().expect("parents is not empty");
           let order = self.order;

           // Finally, no recursions anymore!
           if parent_of_split.len() < order {
               let mut parent_write_guard = parent_of_split.write();
               let idx = parent_write_guard
                   .ptrs
                   .binary_search(&split)
                   .expect("`split` should be there");
               parent_write_guard.ptrs.insert(idx + 1, kp.1);
               parent_write_guard.keys.insert(idx, kp.0);
           } else {
               // split `parent_of_split` (non leaf node)
               let parent_plus = Node::new(parent_of_split.kind());
               let mut parent_write_guard = parent_of_split.write();
               let mut tmp_keys = parent_write_guard.keys.drain(..).collect::<Vec<Rc<T>>>();
               let mut tmp_ptrs = parent_write_guard.ptrs.drain(..).collect::<Vec<Node<T>>>();
               let idx = tmp_keys
                   .binary_search(&kp.0)
                   .expect_err("kp.0 should not be there");
               tmp_keys.insert(idx, kp.0);
               tmp_ptrs.insert(idx + 1, kp.1);

               assert_eq!(tmp_keys.len(), order);
               assert_eq!(tmp_ptrs.len(), order + 1);

               let idx_of_k = ((order as f64 + 1.0) / 2.0).ceil() as usize - 1;
               parent_write_guard.keys.extend(tmp_keys.drain(0..idx_of_k));
               parent_write_guard.ptrs.extend(tmp_ptrs.drain(0..=idx_of_k));

               let k = tmp_keys.remove(0);

               let mut parent_plus_write_guard = parent_plus.write();
               parent_plus_write_guard.keys = tmp_keys;
               parent_plus_write_guard.ptrs = tmp_ptrs;

               drop(parent_write_guard);
               drop(parent_plus_write_guard);

               self.insert_in_parent(parent_of_split, parents, (k, parent_plus));
           }
       }
   }
   ```

5. Difference between spliting a leaf node and a non-leaf node

   1. When splitting leaf node, `tmp.pointers[0, ceil(order/2))` should be given to
      the old leaf, but when splitting non-leaf nodes, `tmp.pointers[0, ceil(order/2)]`
      should be given. (One more pointer)

   2. Splitting a leaf node can result in key duplication (in both the new leaf 
      node and the parent node), this replication won't happen when splitting
      non-leaf node as the selected entry(first entry in the new node) will be
      **moved** to the parent node.

### 14.3.3.2 Deletion

1. How to do deletion on a B+Tree
   
   1. Traverse down to the leaf node
   2. `delete(target_value, Key, Pointer)`

2. How to do `delete(Node, Key, Pointer)`

   > Delete `(key, Pointer)` from `Node`

   1. Just Delete them

   2. If `Node` is a Root node and it has ONLY one child,
      
      > A Root with one child is NOT a leaf node (leaf node has 0 children),
      > and a Root node that is not a leaf node should have `[2, n]` children.

      1. Make the child the new Root node 
      2. Abandon the old root `Node`
      3. Return
   
   3. If `Node` has too few search-key values or pointers

      > ```rust
      > pub(crate) fn node_has_too_few_entries(&self, node: &Node<T>) -> bool {
      >    let search_key_threshold = ((self.order - 1) as f64 / 2.0).ceil() as usize;
      >    let ptr_threshold = ((self.order as f64) / 2.0).ceil() as usize;
      >
      >    if node.is_leaf() {
      >        node.read().keys.len() < search_key_threshold
      >    } else {
      >        node.read().ptrs.len() < ptr_threshold
      >    }
      > }
      > ```

      1. Let `Node'` be a sibling of `Node`
         
         > Here, you have to choose which sibling to use
         >
         > In my implementation, I prefer coalescence rather than redistribution,
         > so I will try to find the best sibling (i.e., the sibling has fewer
         > entries)

      2. Let `K'` be the value between pointers `Node` and `Node'` in `parent(Node)`
         
         ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-06-10%2015-52-40.png)

      3. If entries in `Node` and `Node'` can fit into a single node, begin coalescence:

         > * leaf node
         >
         >   The # of Search-key <= `n-1`
         >
         > * non-leaf node
         >
         >   The # of pointer <= `n`

         1. If `Node` is a predecessor of `Node'`, then let's rename `Node` to
            `Node'`, `Node'` to `Node`.

            > Make `Node'` **always the left one** so that we can **append** to
            > it.
         
         2. If `Node` is not a leaf node, append `K'` and all keys and pointers
            in `Node` to `Node'`

            Else(`Node` is a leaf node), simply append all keys and pointers in
            `Node` to `Node'`, and set `node_plus.next_leaf` to `node.next_leaf`

         3. Delete `Node` (drop a `Rc`)

         4. Recursion: `delete_entry( parent(Node), K', Node)` 

      4. Redistribution

         > Always remeber that redistribution **borrows** an entry from `Node'`

         1. If `Node'` is predecessor of `Node`
            1. Move `Node'`'s last key and pointer out
            2. If `Node` is a non-leaf node
               1. Prepend `K'` to `Node`'s keys
               2. Prepend `Node'`'s last pointer to `Node`'s pointers
               3. replace `k_plus` in the `parent` with the last key in `node_plus`
            3. Else
               1. Prepend `Node'`'s last key to `Node`'s keys
               2. Prepend `Node'`'s last pointer to `Node`'s pointers
               3. replace `k_plus` in the `parent` with the last key in `node_plus`
         2. Else
            1. Move `Node'`'s first key and pointer out
            2. If `Node` is a non-leaf node
               1. Prepend `K'` to `Node`'s keys
               2. Prepend `Node'`'s first pointer to `Node`'s pointers
               3. replace `k_plus` in the `parent` with the first key in `node_plus`
            3. Else
               1. Prepend `Node'`'s first key to `Node`'s keys
               2. Prepend `Node'`'s first pointer to `Node`'s pointers
               3. replace `k_plus` in the `parent` with the first key in `node_plus`


## 14.3.4 Complexity of B+Tree updates

1. Although insertion and deletion on B+Tree are complicated, they require 
   relatively few I/O operations as the `order` will be pretty big, and thus
   the tree will be short.

   And the amount of I/O operations of insertion and deletion is proportional
   to the height of the B+Tree(recursion until the Root in the worse case), 
   and is therefore low.

## 14.3.5 Nonunique Search Keys

1. Recall what we have learned eariler, for non-unique search key values, we
   can add extra attributes to it to compose a conposite search key that is
   unique for all the rows.

   A common way is to add the primary key to the search key.

   The extra attribute added is called a **uniquifier** attribute

2. Or we can modify the algorithm to make B+Tree support duplicate entries.

   > NOTE: this is for the B+Tree index
   >
   > i.e., `BPlueTreeMap<SearchKey, PointerToDisk>`

   * For duplicate search-key entries, assign a Bucket for each search-key, store
     all pointer in that bucket.

     > This method is spcae-efficient as the duplicate search-keys are only stored
     > once.

   * Store the search-key value once per record, this way is not space efficient
     as a search-key may be stored multiple times, and the worse thing is it is 
     possible that two nodes have two search-keys with the same value, which makes
     lookup significantly more complicated.

   All the ways other than composite search-key will have its hard-to-deal drawback,
   so most databases choose to use composite search-key.

# 14.4 B+Tree Extensions

> In this section, we discuss several extensions and variations of B+Tree index
> structure.

## 14.4.1 B+Tree File Organization

1. B+Tree File Organization

   In a B+Tree index, we store pointers in the leaf nodes, but we can also store
   records in them. Storing records in a B+Tree within a file is called B+Tree
   file organization.

   > SQLite stores its records in a B+Tree file organization.

2. Space utilization of B+Tree file organization

   Space utilization becomes more important because records are much larger than
   keys and pointers. We can improve this (make it use less space) by involving
   more sibling nodes during split and merge.

   > Adjusting the split and merge strategy.

   * Insertion

     If the node to insert is full, try to redistribute some entries with its 
     sibling node.(In the prev algorithm, we directly create a new node and
     split). 

     > Assume each node can hold 6 entries at most, then the node to insert
     > has 6 entries, the sibling may have `3-5` entries, plus the one to be
     > inserted, we have `10-12` entries in total.
     >
     > Assume the redistribution algorithm used here is to evenly redistribute
     > them, each node will have `5-6` entries, which is pretty efficient in
     > space utilization.


     If the sibling node is also full, then we split a new node and
     evenly redistribute the entries with these 3 nodes (orig node, split node,
     sibling).

     > With the assumption stated in the prev node, we have `6+6+1` entries, 
     > each node will have `13/3 = 4.3` entries.
     >
     > As a comparsion, if we follow the orig algorithm, i.e., directly splitting
     > the node, each node will have `7/2=3.5` enties. Our assumption of `n` is 6
     > is not pratical, in real work, it should be greatly bigger than this, and
     > thus the new algorithm should be much more efficient.

   * Deletion

     Duration deletion of a record, if the occupancy of a node falls below 
     `floor(2n/3)`, we attempts to borrow an entry from one of its sibling nodes.

     > Now we are asking every node should be at least `2/3` full.

     If the above attempt fails due to the reason that both sibling nodes have
     `floor(2n/3)` entries, then we distribute all the entries in these 3 nodes
     (2 sibling nodes and the middle, orig node) into 2 nodes and delete the 
     third node. After this, the remaining 2 nodes will be almost full 
     ([3 * (2n/3)] -1 = 2n - 1, each node has `n-0.5` entries).

## 14.4.2 Secondary Indexes and Record Relocation

1. The problem, Record Relocation

   Say we use B+Tree File Organization to store the actual records, when 
   inserting records, assume a split is needed, half node records need to be
   moved to a new node, i.e., a new location.

   Secondary Indexes, which stores the actual location in leaf nodes, need to
   be updated as well to be in sync with the data, which causes tons of I/O, 
   and could be very expensive.

2. Workaround

   Rather than storing record locations in leaf nodes, we can actually store
   the values of the primary-index search-key attributes.

   With this, we don't need to update the secondary indexes anymore. However,
   querying on a secondary index requires 2 steps now:

   1. Find the primary index search-key value
   2. Query the priamry index

## 14.4.3 Indexing Strings

1. Creating B+Tree index on an attribute that is of type `String` can cause 
   problems:

   1. String is variable-sized
   2. String can be long, leading to a low fanout and a correspondingly increased
      tree height.

2. Solve the low fanout problem with prefix compression
   
   If a prefix of a string is able to distinguish it from other strings, you
   can ONLY store the prefix instead of the whole string. (make it smaller).

## 14.4.4 Bulk Loading of B+Tree Indexes (important)

1. What is bulk loading of index

   > Insertion of a large number of entries at a time into an index is called 
   > bulk loading of the index.

   Let's assume that our relation is so huge that the index is bigger than the
   memory, under such cases, it is quite likely that each leaf node is not in
   the memory buffer when accessed. Inserting to this index will probably needs 
   2 random disk accesses, one for fetching the block containing the leaf node, 
   the other for writing the block back (buffer is full, evict a page beforing 
   loading another page).

   Since our relation is huge, building an index for such a relation requires
   tons of random accesses. 

   One solution to this issue is bulk loading, and to do bulk loading:

   1. Create a temporary file containing the index entries (This file is not a B+Tree)
   2. Sort the index entries
      > For how to sort a large relation, see in Section 15.4

   3. Iterating over the ordered entries, and insert them into the B+Tree.
      
   The key idea of bulking loading is you need to insert entries **in sorted order**,
   in this way, insertion into a leaf node will be **continuous** and done **ONLY 
   ONCE**, which means ONLY 1 I/O is needed for a leaf node.

## 14.4.5 B-Tree Index Files

1. Diff between B-Tree and B+Tree
   
   In B-Tree, keys (assume they are unique) ONLY exist once, while in a B+Tree,
   they exist both in leaf node (actual storage) and non-leaf node (for 
   navigation)

## 14.4.6 Indexing on Flash Storage
## 14.4.7 Indexing in Main Memory

# 14.5 Hash Indices
# 14.6 Multiple-key Access
# 14.7 Creation of Indices
# 14.8 Write-Optimized Index Structures
# 14.9 Bitmap Indices
# 14.10 Indexing of Spatial and Temporal Data
# 14.11 Summary
