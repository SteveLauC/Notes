1. The first query optimizer is the IBM System R optimizer, it was designed in 
   1970s. Before this, people don't believe the DBMS can write better SQL than
   human.

   Many concepts and designs from the System R optimizer still apply today.

2. For single relation queries, the biggest problem is choosing the table access
   path (sequential scan, binary search (can we?), index scan).

   > I think for such queries, choosing the appropriate access path and using 
   > heuristics to reduce the cost would work pretty well.

3. When a query involving multiple relations, the equivalent query plans.

   