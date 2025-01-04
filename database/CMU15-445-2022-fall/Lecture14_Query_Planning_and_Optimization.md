> Today's agenda:
>
> * Heuristic/Rule-based optimization
> * Query cost model: a model to estimate query cost
> * Cost-based optimization

> Agenda in my opinion:
>
> 1. Optimization
>    * Heuristic/Rule-based optimization
>    * Query cost model: a model to estimate query cost
>    * Cost-based optimization
> 2. How to construct the plan (query planning)

# Query optimization

1. The first query optimizer is the IBM System R optimizer, it was designed in 
   1970s. Before this, people don't believe the DBMS can construct better execution 
   plan than human.

   Many concepts and designs from the System R optimizer still apply today.

   > Volcano model was invented in 1994
   >
   > SQL was invented in 1974
   >
   > Relational model was invented in 1970
   >
   > IBM system R started in 1974

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


   The SQL rewriter part is rare, in the previous impl of Pizza, before passing
   the SQL to DataFusion, we replace `<namespace>:<collection>` with the internal
   table name.

   binder replaces entry names with ID, Postgres stores `OID` internally. Catalog
   errors like "table not found" is handled in this stage.

   Converting a logical plan to a physical plan, in this architecture, it
   uses the cost model, but we can use heuristic rules instead, then there
   will also be a tree rewriter. (DataFusion does this)

## Heuristic/Rule-based optimization

1. Some heuristic rules may need to examine catalog, but does NOT need to example 
   data.  As a contrast, cost estimation needs information of data size/distribution.

2. Some heuristic rules to optimize the logical plan (tree rewriter)

   > This is commonly used to optimize the logical plan, but there are database 
   > systems that also use rule-based optimization to generate (logical -> physical) 
   > and optimize physical plan (rewrite it).

   There are generally 2 kinds of rules, one is for the logical plan tree,
   i.e., manipulate the logical plan tree nodes, the other is for the expressions 
   within logical plan nodes.

   How to apply these rules, you can implement them using a bunch of if-else(Postgres, MySQL), 
   or you could implement a pattern matching engine (CockroachDB), which is better.

   1. Rules for the logical plan tree nodes

      1. Predicate pushdown (selection)
      2. Projection pushdown (projection)
      3. Replace cartesian product with join
      4. de-correlate correlated sub query or de-compose non-correlated sub-queries
         
         See [Database System Concepts - 16.4.4 Optimizing Nested Sub-queries](https://github.com/SteveLauC/Notes/blob/main/database/Database_System_Concepts/Ch16_Query_Optimization.md#1644-optimizing-nested-sub-queries)

      5. Split conjunctive predicates (split one node to multiple nodes)

   2. Rules for the expression

      > Projection, Filter, and Join nodes have expressions, to name a few.

      1. Elide predicates that won't change the query result (selection)
      
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

      2. Merge predicates with redundancy

         > QUES: will this make expression evaluation faster?
         >
         > You don't need to access `val` twice?

         ```sql
         SELECT * FROM t WHERE a BETWEEN 1 AND 100 OR a BETWEEN 50 AND 150;
         ```

         ```sql
         SELECT * FROM t WHERE a BETWEEN 1 AND 150;
         ```

         > Compiler should be good at this.

      3. Reorder predicates so that the DBMS applies the most selective one first (selection)

      4. Join elimination

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
      

## Cost-based optimization (for physical plans, the metrics are all physical stuff)

1. Cost model components (The things you need to consider when computing the cost of a query)

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

   Most real-world databases that employ CBO use a mix of 1 and 2, the third one 
   is only used in the academic papers.

   > The cost model used by PostgreSQL
   >
   > PostgreSQL uses a combination of CPU and I/O costs weighted by magic constant
   > factors.
   >  
   > The default factors are for disk-based databases:
   > 
   > 1. Reading a tuple from RAM is 400x faster than doing so from disk
   > 2. Sequential I/O is 4x faster than random I/O
   >
   > Ref: [Planner Cost Constants][link]
   >
   > [link]: https://www.postgresql.org/docs/current/runtime-config-query.html#RUNTIME-CONFIG-QUERY-CONSTANTS

2. To compute the cost, you need to know the size of the data that the query 
   will process. For simple queries like `SELECT * FROM table`, this is simple,
   you simply need to **store the table size**. 
   
   Once your query has some predicates, things become complex 
   
   ```sql
   SELECT MAX(score) FROM table WHERE column > value
   ```
   
   How can we know the size of `SELECT * FROM table WHERE column > value`, besides
   knowing the size of `table`, we also need to know the selectivity of 
   `column > value`, which requires us to **store the data distribution** info.

   For queries like:

   ```sql
   SELECT MAX(score) FROM (SELECT DISTINCT column FROM table);
   ```

   We also want to store the cardinality of the distinct values of `column`.

   To summarize, roughly, for a table, we need the following statistics information:

   1. The # of tuples 
   2. A rough distribution statistics
   3. The # of distinct values of its columns


   The following are some ways to store statistics 

   1. Histogram (for data distribution)
      
      ![d](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-04%20at%204.55.06%E2%80%AFPM.png)
       
   2. Equi-width histogram (for data distribution, less accurate, less space usage)

      ![d](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-04%20at%204.55.23%E2%80%AFPM.png)

   3. Sketches

      1. Count-min sketch: could answer the frequency of a specific value
      2. HyperLogLog: could give a rough answer of a column's distinct value count

   4. For a table, store a much smaller, sample table for it (by sampling data 
      into it), then for probability questions, query them in the sample table,
      and you assume the answer would match the full table.

      SQLServer and DB2 do this.

3. If you maintain statistics, how often do you refresh them?

   Different DBMSs have different update strategy. For example, Oracle has a Cron job that
   updates the statistics regularly. And Postgres provides a command so that users
   can manually update them: [`ANALYZE [table_name ...]`](https://www.postgresql.org/docs/current/sql-analyze.html).
   I think you can make it automatic with `pg_cron` as well.

4. Now do the cost-based optimization

   1. Convert the logical plan to multiple physical plans

      There will be multiple plans, because a logical operator can have multiple
      corresponding physical operators:

      1. LogicalTableScan -> [TableScan, IndexScan]
      2. LogicalJoin -> [NestedLoopJoin, BlockNestedLoopJoin, IndexJoin, HashJoin, SortMergeJoin]
      3. ...
      
   2. Estimate their cost
   3. Pick the plan with the lowest cost


# Query Planning

1. Single relation planning

   For single relation queries, the most important thing to do in planning is 
   choosing the table access path (table scan, or index scan). Most new databases
   just use heuristics to choose the AM.

   > I think for such queries, choosing the appropriate access path and using 
   > heuristics to reduce the cost would work pretty well.
   
   OLTP queries are usually easy to deal with because:
   
   * just pick the best index (when generating the physical plan)
   * Joins are almost always on the foreign key relations with a small cardinality
   * Can be implemented with heuristics

2. Multi-relation query planning
  
   For multi-relation query planning, there are generally 2 choices:
   
   1. Bottom-up optimization
      
      Do the initial optimization with heuristic rules, then use dynamic programming
      to determine the join orders.

      ![d](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-01-04%20at%206.08.28%E2%80%AFPM.png)

      
      > IBM System R, DB2, MySQL, PostgreSQL 
   
   2. Top-down optimization(?)
   
      Start with the output you want(how), then work down the tree to find the 
      optimal plan.
      
      > MSSQL, Greenplum, CockroachDB

      > QUES: NO idea on how this actually works, looks really complicated.
