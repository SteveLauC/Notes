1. Comparison between various formats

   | x | Unsorted Array | Sorted Array | B+TREE |
   |Pages contain|only data|only data|metadata, primary keys, and data |
   |Rows per page|more|more|fewer|
   |Insertion|O(1) (since we append)|O(n)|O(log(n))|
   |Deletion|O(n)|O(n)|O(log(n))|
   |Lookup by id|O(n)|O(log(n))|O(log(n))|
