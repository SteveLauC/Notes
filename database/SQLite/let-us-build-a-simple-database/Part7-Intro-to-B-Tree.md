1. Diff between BST and B-Tree

   In B-Tree, a node can have more than 2 children, and the maximum number of
   nodes ("m") is called the tree's `order`.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/B-tree.png)

   Like the above tree, its order is 4.

   > To make a B-Tree balanced, we also restrict that a node has to have at
   > least m/2 (rounded up) children.
   >
   > Exceptions:
   >
   > * Leaf nodes have 0 children
   > * The root node can have fewer than m children but must have at least 2
   > * If the root node is a leaf node (the only node), it still has 0 children

2. Diff between B-Tree and B+Tree (employed in SQLite)

   |              | B-Tree      |   B+Tree    |
   |--------------|-------------|-------------|
   | Pronounced   | Bee Tree    |Bee Plus Tree|
   |Used to store |Indexes      | Tables      |
   |Internal nodes store keys (ROWID) | Yes  | Yes |
   |Internal nodes store values       | Yes  | No (Can be Yes if we treat ptr as some kind of value) |
   |Number of childeren per node      |Less  |More |
   |Internal node vs. leaf node       | Same structure | Different structure|

   > * Internal Node: nodes with children

3. Diff between internal node and leaf node

   |For an order-m tree | Internal Node	| Leaf Node |
   |--------------------|---------------|-----------|
   |Stores              |keys and pointers to children|keys and values|
   |Number of keys      |up to m-1      |as many as will fit|
   |Number of pointers	|number of keys + 1(m)| none |
   |Number of values	|none	        |number of keys|
   |Key purpose	        |used for routing|paired with value|
   |Stores values       |No	            |Yes|

4. For how insertion and delection happens in a B+TREE, see [this note]()
