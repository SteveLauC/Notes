> Today's agenda:
>
> * Heuristic/Rule-based optimization
> * Query cost model: a model to estimate query cost
> * Cost-based optimization

1. The first query optimizer is the IBM System R optimizer, it was designed in 
   1970s. Before this, people don't believe the DBMS can write better SQL than
   human.

   Many concepts and designs from the System R optimizer still apply today.

   > Volcano model was invented in 1994

2. Query optimizations generally have 2 strategies:
   
   1. Heuristic, i.e., use static rules

      > Heuristic is also rule-based
      >
      > QUES: But what is the difference between heuristic rules and the equivalence
      > rules that are used for plan enumeration?
      >
      > I think heuristic rules should be a subset of the equivalence rules, and as
      > human being, we trust that a plan rewritten with heuristic rules is 
      > guaranteed to be more efficient. However, equivalence rules do not have
      > such a guarantee.

   2. Cost-based optimization 
      
      > built on top of plan rewriting with equivalence rules

3. System architecture

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-06-16%20at%204.23.07%20PM.png)

   > binder replaces entry names with ID, Postgres stores `OID` internally.
   >
   > Converting a logical plan to a physical plan, in this architecture, it
   > uses the cost model, but we can use heuristic rules instead, then there
   > will also be a tree rewriter. (DataFusion does this)

# Heuristic/Rule-based optimization

1. Some heuristic may need to examine catalog, but does NOT need to example data.
   As a contrast, cost estimation needs information of data size/distribution.

2. Some heuristic rules

   1. Predicate pushdown
   2. Projection pushdown
   3. Replace cartesian product with join
   4. Reorder predicates so that the DBMS applies the most selective one first
   5. de-correlate correlated sub-queries
   6. Elide predicates that won't change the query result 
      
      For instance, query `SELECT * FROM t WHERE 1 = 0` will return nothing 
      because of predicate `1 = 0`, Postgres uses [One-time filter][link] to return
      an empty result:

      [link]: https://github.com/postgres/postgres/blob/32d3ed8165f821f6994c95230a9a4b2ff0ce9f12/src/backend/executor/nodeResult.c#L30-L34

      ```sh
      steve=# explain select * from students where 1=0;
                      QUERY PLAN
      ------------------------------------------
       Result  (cost=0.00..0.00 rows=0 width=0)
         One-Time Filter: false
      (2 rows)
      ```

      Also, predicate like `1 = 1` will eliminated:

      ```sh
      steve=# explain select * from students where 1=1;
                             QUERY PLAN
      ---------------------------------------------------------
       Seq Scan on students  (cost=0.00..1.01 rows=1 width=42)
      (1 row)
      ```

   7. Join elimination

      ```sql
      SELECT a1.* 
      FROM a AS a1 JOIN a AS a2 
      ON a1.id = a2.id;
      ```

      QUES: Well, this example from the class seems like only work if field `id`
      is key:

      ```sql
      steve=# create table A (id int, text varchar(10));
      CREATE TABLE
      steve=# insert into A values (0, '0');
      INSERT 0 1
      steve=# insert into A values (0, '0');
      INSERT 0 1


      steve=#       SELECT a1.*
            FROM a AS a1 JOIN a AS a2
            ON a1.id = a2.id;
       id | text
      ----+------
        0 | 0
        0 | 0
        0 | 0
        0 | 0
      (4 rows)

      steve=# select * from A;
       id | text
      ----+------
        0 | 0
        0 | 0
      (2 rows)
      ```


   8. Merge predicates with redundancy

      > QUES: will this make expression evaluation faster?
      >
      > You don't need to access `val` twice?

      ```sql
      SELECT * FROM t WHERE a BETWEEN 1 AND 100 OR a BETWEEN 50 AND 150;
      ```

      ```sql
      SELECT * FROM t WHERE a BETWEEN 1 AND 150;
      ```

   9. Split conjunctive rules

      > QUES: will this make expression evaluation faster?

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
   >
   > [link]: https://www.postgresql.org/docs/current/runtime-config-query.html#RUNTIME-CONFIG-QUERY-CONSTANTS

3. Statistic is needed to estimate cost

   DBMSes maintain internal statistics about tables, attributes, and indexes in
   its catalog.

   And they have different update strategy.

4. Different kinds of statistics

   1. Histogram
   2. Equi-width histogram
   3. Sketches

5. How DBMS builds statistics, sampling! And samples will be updated when the 
   underlying data changed significantly.

4. Some


# Cost-based optimization

1. Single relation planning

   For single relation queries, the biggest problem is choosing the table access
   path (sequential scan, binary search (can we?), index scan).

   > I think for such queries, choosing the appropriate access path and using 
   > heuristics to reduce the cost would work pretty well.
   
   OLTP queries are usually easy to deal with because:
   
   * just pick the best index 
   * Joins are alomost always on the foreign key relations with a small cardinity
   * Can be implemented with heuristics

2. Multi-relation query planning
  
   For multi-relation query palnning, there are generally 2 choices:
   
   1. Bottom-up optimization
      
      Do the initial optimization with heuristic rules, then use dynamic programming
      to determine the join orders.
      
      > IBM System R, DB2, MySQL, PostgreSQL 
   
   2. Top-down optimization(?)
   
      Start with the output you want(how), then work down the tree to find the 
      optimial plan.
      
      > MSSQL, Greenplum, CockroachDB
      
3. Bottom-up optimization


4. Top-down optimization

https://www.cockroachlabs.com/blog/building-cost-based-sql-optimizer/


4. System R optimizer



5. When a query involving multiple relations, the equivalent query plans.
