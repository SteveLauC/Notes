1. Planning phases
  
   > Postgres planning has 4 stages:
   >
   > 1. Preprocessing
   > 2. Plan "Scan" and "Join"
   > 3. Query special features handling
   > 4. Postprocessing

   1. Preprocessing: Simplify query if possible, and collect information such as 
      join order restriction.
   
      > The preprocessing steps are not done in a specific order, they are 
      > intermixed.
      
      * Simplify scalar expression
      
        > Why
        >
        > 1. If we simplify it, **we do computations only once**.  Otherwise, we 
        >    need to do it for every single row.
        > 2. View expansion (by rewriter) and SQL function inlining can expose 
        >    constant-folding opportunities not visible in the original query
        >
        >    > What is constant-folding
        >    >
        >    > An optimization from the compiler field.  Evaluate expression to 
        >    > its final result at compile-time.
        >    >
        >    > In database field, it means evaluating the expressions during 
        >    > planning phase.
        >
        > 3. Simplifying takes a lot of load off the estimation functions, which 
        >    by and large can’t cope with anything more complex than `Variable = 
        >    Constant`. (cost estimation, e.g., evaluating selectivity, would 
        >    be easier if the expression is simple)

        * function calls
          
          * For a strict function: `int4eq(1, NULL)` can be simplified to `NULL`
          * Immutable function: `2 + 2 >= 4`, we can pass `2+2` to the executor
            so that we only compute it once. (This is more like constant-folding)

        * Boolean expressions

          * `x OR true` -> `true`
          * `x AND false` -> `false`
        
      * In-line simple SQL functions
        
        > QUES: why would this make the query faster
        >
        > Can expose constant-folding opportunities not visible in the original 
        > query
        
      * Simplify join tree

        * Flatten/Pull up sub-queries

          Sub-queries will be planned independently, it will be treated as a block box
          during planning of the outer query. By pulling it up, we could have better
          plans.

        * Flatten UNION ALL, expand inheritance trees
        * Reduce join strength
          
          * Reduce outer join to inner join
        
            > This has a more popular name: NULL rejection
            >
            > See also: database/Database_System_Concepts/Ch16_Query_Optimization.md

            If there is a strict qualification above the outer join that constricts a Var
            from the nullable side of the join to be non-null, then this outer join can
            be converted into inner join.

          * Reduce outer join to anti joins

            > QUES: I do not understand this. Transforming outer join to inner join makes
            > sense to me as outer join needs the executor to perform some extra steps
            > after finishing the inner join part.
            >
            > I do not know if anti join is easier to execute.
           
            If the outer join's own qualifications are strict for any nullable 
            Var that was forced NULL by higher qualification levels

            ```sql
            SELECT * FROM
            foo LEFT JOIN bar
            ON foo.a = bar.c
            WHERE bar.c IS NULL;

            -- Can be transformed to

            SELECT * FROM
            foo ANTI JOIN bar
            on foo.a = bar.c;
            ```

        * Convert IN, EXISTS sub-selects to semi-joins (Correlated subqueries)
        * Identify anti-joins

          TODO: check if this is same as "Reduce outer join to anti joins"

      * Later preprocessing

        * Determine where `WHERE/ON` clauses (“quals”) should be evaluated (Predicate pushdown)

          > In general, we want to use each qual at the lowest possible join level

        * Identify all referenced table columns (Vars), and find out how far up
          in the join tree their values are needed  (Projection pushdown)

        * Build equivalence classes for provably-equal expressions

          QUES: Why do we need to collect this information, how does it help the 
          query optimization.

        * Gather information about join ordering restrictions

          QUES: do not understand this

        * Remove useless joins (needs results of above steps)

          * A left join can be omitted if:

            * inner relation is a single base relation
            * the attributes of inner relation are not used above the join
            * The join condition cannot match more than 1 row on inner relation side
              
              > `SELECT DISTINCT c` ensures this
            
            Example:

            ```sql
            SELECT foo.a
            FROM foo LEFT JOIN (SELECT DISTINCT c AS c FROM bar) sub
            ON foo.a = sub.c

            -- can be transformed into
            SELECT foo.a FROM foo
            ```
      
   2. Plan "Scan" and "join"
   
      Deal with `FROM` and `WHERE/ON` clauses.  It also knows about `ORDER BY` 
      information in order to generate merge-join paths.

      > Why will will deal with `WHERE/ON` here?
      >
      > The predicates, could be
      >
      > * table scan predicates
      > * join conditions
      >
      > affect how we access the base relation and join order.

      1. Identify feasible scan methods for base relation
      2. Figuring the best join order
         
         1. Multi-way joins are built up from pairwise joins
         2. For any given pairwise join step, we can identify the best input 
            Paths and join methods via straightforward cost comparisons

      Standard join search method (System R approach):

      1. Generate paths for accessing base relations involved in the join (SeqScan, 
         IndexScan, TidScan)
      2. Generate paths for each possible 2-way join
      3. Generate paths for each possible 3-way join
      4. Continue until all base relations are joined into a single join 
         relation; then use that relation’s best path

      Searching for the best join order is hard as there are too many possibilities,
      Postgres use a few heuristic rules to make the search simpler.  With too many 
      relations (configuration entry `geqo_threshold`, defaults to 12), fall back to 
      “GEQO” (genetic query optimizer) search, which is even more heuristic and tends 
      to fail to find desirable plans.  Heuristic join rules used by Postgres:

      1. Don't join relations that are not connected by any join clause, unless 
         forced to by join-order restrictions 

         We will create a Cartesian Product if we do it.
         
         > Implied equalities count as join clauses, so this rule seldom leads us 
         > astray


      2. Break down large join problems into sub-problems by not flattening JOIN
         clauses according to collapse limit `join_collapse_limit`

         > Actually, it's done by not merging sub-problems to make a big problem 
         > in the first place (see “collapse limits”)
         >
         > This frequently sucks; would be useful to look for smarter ways of 
         > subdividing large join trees

         > QUES: I do not understand this.  What does this configuration entry do?
         > Per the doc, looks like it also controls when Postgres will give up trying
         > to reorder the joins, i.e., it executes joins using the order specified
         > in SQL.
      
      GEQO: Treats join order searching as a Traveling Salesman Problem, i.e., 
      minimize the length of a “tour” visiting all “cities” (base relations).
      Does a partial search of the tour space using heuristics found useful for 
      Traveling Salesman Problem. Problem: join costs don’t behave very much like
      inter-city distances; they interact. This makes the TSP heuristics not so 
      effective (This area desperately needs improvement)
   
   3. Query special feature handling

      * Deal with GROUP BY, DISTINCT, aggregate functions, window functions
      * Deal with UNION/INTERSECT/EXCEPT
      * Apply sort order if needed by ORDER BY

   4. Postprocessing
   
      * Expand `Path`s to `Plan`s (Actually, this happens after scan/join 
        planning, and before query special feature handling. Because )

      * Adjust Plan representation details
        * Flatten subquery rangetables into a single list
        * Label Vars in upper plan nodes as `OUTER_VAR` or `INNER_VAR` to refer
          to the outputs of their subplans
        * Remove unnecessary SubqueryScan, Append, and MergeAppend plan nodes
        * etc
