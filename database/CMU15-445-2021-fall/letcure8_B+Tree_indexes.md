1. Properties of a B+Tree

   1. It is perfectly balanced (All the leaf nodes are in the same level)
   2. Every node other than root is half full (i.e., amount of keys range: `[M/2, M-1]`)
   3. Every internal node with `k` keys has `k+1` non-null children
   4. When used as an index:
      1. Every node is an array of key-value pairs.
      2. The keys are derived from the attributes that the index is based on
      3. The values will differ based on whehter this is a internal node or leaf node.
      4. The array is (usually) kept in order.

2. What should be stored in the leaf node:

   1. Pointer (Record ID)
      A pointer to the location of the tuple to which the index entry corresponds. 

      > Then you have to do a second look-up to fetch the actual data.

      > Implementations that use this approach:
      > * PostgreSQL
      > * SQL Server

   2. Actual data
      We store the actual data in leaf nodes

      > Implementations that use this approach:
      > * SQLite
      > * MySQL 

3. Advantage of using B+TREE

   In B-TREE, each key ONLY appears once in the tree, which is obviously more
   space-efficient. But when traversing the tree, you need to jump up and down,
   have accesses to different pages and thus more random access.

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

5. Deletion a entry in a B+TREE
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
   key can be boosted, i.e., a query that exactly selects attributes `a, b, c`
   can be accelerated.

7. How to handle duplicate keys in B+TREE

   1. Append RecordID
      Add the tuple's unique RecordID as part of the key to ensure that are the
      keys are unique.

      > The DBMS can still use partial key to find the tuple.
      > 
      > Then if the key for which we are building index includes RecordID, then
      > the key will never be duplicate?

   2. Overflow Leaf Nodes
      Allow leaf nodes to spill into overflow nodes that contain the duplicate
      keys.

      > This is more complex to maintain and modify

8. What is clustered index
   A clustered index is a special kind of index that reorders the way that
   records are phycially stored on the disk. And ONLY one clustered index 
   can exist for a table.
