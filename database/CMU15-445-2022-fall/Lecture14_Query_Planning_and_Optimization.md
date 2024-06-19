> Today's agenda:
>
> * Heuristic/Rule-based optimization
> * Query cost model
> * Cost-based optimization

1. The first query optimizer is the IBM System R optimizer, it was designed in 
   1970s. Before this, people don't believe the DBMS can write better SQL than
   human.

   Many concepts and designs from the System R optimizer still apply today.

2. Query optimizations generally have 2 strategies:
   
   1. Heuristic, i.e., use static rules

      > Heuristic is also rule-based

   2. Cost-based optimization

3. System architecture

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-06-16%20at%204.23.07%20PM.png)

# Heuristic/Rule-based optimization

1. Heuristic needs to examine catalog, but does NOT need to example data. As a 
   contrast, cost estimation needs information of data distribution.

2. Some heuristic rules

   1. Split conjunctive rules
   2. Predicate pushdown
   3. Projection pushdown
   4. Replace cartesian product with join
   5. Reorder predicates so that the DBMS applies the most selective one first
   6. de-correlate correlated sub-queries
   7. Expression rewriting

3. Expression rewriting

   > You need a rule pattern matching engine

   Procedure:

   1. For a given expression, find the parts that match a pattern
   2. Rewrite the matched part 
   3. Stop if there are no more rules that match

   Rules:

   1. Remove impossible/unnecessary expression
   2. Merge predicates that operate on the same field
    

# Query cost model

> How to estimate the cost of a query
>
> 1. Know the size of the input of this query
> 2. Know the probability of the operation involved in this query
> 3. Then you know the size of the output of this query

1. Cost model components

   1. Physical costs

      * CPU cycles
      * I/O
      * Cache misses
      * RAM usage
      * Network messages

      This relies heavily on hardware.

   2. Logical costs

      Estimate output size of operators.
   
   3. Algorithmic costs

      Time/space complexity of the algorithm used in an operator.

2. The cost model used by PostgreSQL

   PostgreSQL uses a combination of CPU and I/O costs weighted by magic constant
   factors.

   The default factors are for disk-based databases:

   1. Reading a tuple from RAM is 400x faster than doing so from disk
   2. Sequential I/O is 4x faster than random I/O

   > Ref: [Planner Cost Constants][link]
   > [link]: https://www.postgresql.org/docs/current/runtime-config-query.html#RUNTIME-CONFIG-QUERY-CONSTANTS

3. Statistic is needed to estimate cost

   DBMSes maintain internal statistics about tables, attributes, and indexes in
   its catalog.

   And they have different update strategy.

4. Different kinds of statistics

   1. Histogram
   2. Equi-width histogram
   3. Sketches

5. How DBMS builds statistics, sampling!

4. Some


# Cost-based optimization


4. For single relation queries, the biggest problem is choosing the table access
   path (sequential scan, binary search (can we?), index scan).

   > I think for such queries, choosing the appropriate access path and using 
   > heuristics to reduce the cost would work pretty well.

5. When a query involving multiple relations, the equivalent query plans.
