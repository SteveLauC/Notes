* planner()
  * standard_planner()
    * subquery_planner()
      * grouping_planner()
        * query_planner()
          * make_one_rel()
      * set_cheapest()

    * fetch_upper_rel()
    * get_cheapest_fractional_path(RelOptInfo, tuple_fraction)
    * create_plan(root, path)
    * SS_finalize_plan(root, plan)
    * set_plan_references()

------------------

# planner()

## standard_planner()

This function initializes `struct PlannerGlobal` and `struct PlannerInfo`,  calls
`subquery_planner()` to do the actual planning job, then does postprocessing.

1. Parallel execution

   ```c
   if ((cursorOptions & CURSOR_OPT_PARALLEL_OK) != 0 &&
      IsUnderPostmaster &&
      parse->commandType == CMD_SELECT &&
      !parse->hasModifyingCTE &&
      max_parallel_workers_per_gather > 0 &&
      !IsParallelWorker())
   {
      /* all the cheap tests pass, so scan the query tree */
      glob->maxParallelHazard = max_parallel_hazard(parse);
      glob->parallelModeOK = (glob->maxParallelHazard != PROPARALLEL_UNSAFE);
   }
   else
   {
      /* skip the query tree scan, just assume it's unsafe */
      glob->maxParallelHazard = PROPARALLEL_UNSAFE;
      glob->parallelModeOK = false;
   }
   ```

   A query can be executed in parallel if following conditions are satisfied:

   * If `CURSOR_OPT_PARALLEL_OK` is contained in `cursorOptions`, it should be 
     safe

     > This is true if the planer is invoked from `exec_simple_query()`
     >
     > ```c
     > plantree_list = pg_plan_queries(querytree_list, query_string, CURSOR_OPT_PARALLEL_OK, NULL);
     > ```

   * We should have worker processes (we have them when we have postmaster)
   * This query won't modify/write anything
     * It is a SELECT statement
     * If it has CTE, the queries within CTE should not modify anything
   * Users permit parallelism (they can disable it by setting `max_parallel_workers_per_gather` to 0)

     > `gather` here means `struct Gather` plan node

   * The current process is not a parallel worker process.  Postgres currently
     does not allow parallel workers to create parallel workers.
   * Scan the query itself to check/ensure it is safe to be executed in parallel.

   > (Note that we do allow CREATE TABLE AS, SELECT INTO, and CREATE
   > MATERIALIZED VIEW to use parallel plans, but this is safe only because
   > the command is writing into a completely new table which workers won't
   > be able to see.  If the workers could see the table, the fact that
   > group locking would cause them to ignore the leader's heavyweight GIN
   > page locks would make this unsafe.  We'll have to fix that somehow if
   > we want to allow parallel inserts in general; updates and deletes have
   > additional problems especially around combo CIDs.)

   Write commands like:

   * create table as
   * select into
   * create materialized view

   could use parallel plans because they will create a new table/view that worker
   processes won't see due to snapshot isolation. The workers could do parallel
   read, then only leader process will do the write.

2. If parallel execution is safe to do, we won't do it unless it could improve
   performance. This is the case when the GUC option `debug_parallel_query`
   defaults to `off`.

   ```c
   // glob->parallelModeNeeded is normally set to false here and changed to
   // true during plan creation if a Gather or Gather Merge plan is actually
   // created (cf. create_gather_plan, create_gather_merge_plan).
   glob->parallelModeNeeded = glob->parallelModeOK &&
      (debug_parallel_query != DEBUG_PARALLEL_OFF);
   ```

   Users can force Postgres to use parallel plan by setting it to [`on`, `regress`].

3. This chunk of code initializes the `tuple_fraction` variable, which will be
   used later in `grouping_planner()`:

   When option `CURSOR_OPT_FAST_PLAN` is set, PG uses `cursor_tuple_fraction`
   to init `tuple_fraction`. But since `cursor_tuple_fraction` should be in
   range `(0, 1)`, handle (correct) the edge value separately.

   > QUES: when will this option be set?
   >
   > future steve: For `exec_simple_query()`, this parameter `cursorOptions` is 
   > hardcoded to `CURSOR_OPT_PARALLEL_OK`
   >
   > ```c
   > plantree_list = pg_plan_queries(querytree_list, query_string, CURSOR_OPT_PARALLEL_OK, NULL);
   > ```

   ```c
   /* Determine what fraction of the plan is likely to be scanned */
   if (cursorOptions & CURSOR_OPT_FAST_PLAN)
   {
      /*
      * We have no real idea how many tuples the user will ultimately FETCH
      * from a cursor, but it is often the case that he doesn't want 'em
      * all, or would prefer a fast-start plan anyway so that he can
      * process some of the tuples sooner.  Use a GUC parameter to decide
      * what fraction to optimize for.
      */
      tuple_fraction = cursor_tuple_fraction;

      /*
      * We document cursor_tuple_fraction as simply being a fraction, which
      * means the edge cases 0 and 1 have to be treated specially here.  We
      * convert 1 to 0 ("all the tuples") and 0 to a very small fraction.
      */
      if (tuple_fraction >= 1.0)
         tuple_fraction = 0.0;
      else if (tuple_fraction <= 0.0)
         tuple_fraction = 1e-10;
   }
   else
   {
      /* Default assumption is we need all the tuples */
      tuple_fraction = 0.0;
   }
   ```

   > tuple_fraction is the fraction of tuples we expect will be retrieved.
   >
   > tuple_fraction is interpreted as follows:
   >
   > * 0: expect all tuples to be retrieved (normal case, from psql)
   > * 0 < tuple_fraction < 1: expect the given fraction of tuples available
   >   from the plan to be retrieved
   > * tuple_fraction >= 1: tuple_fraction is the absolute number of tuples expected 
   >   to be retrieved (ie, a LIMIT specification).


4. Generating the plan

   ```c
   /* primary planning entry point (may recurse for subqueries) */
   root = subquery_planner(PlannerGlobal*: glob, Query*: parse, PlannerInfo*: NULL, has_recursion: false, tuple_fraction /*0.0*/, SetOperationStmt*: NULL);

   /* Select best Path and turn it into a Plan */
   final_rel = fetch_upper_rel(root, UPPERREL_FINAL, NULL);
   best_path = get_cheapest_fractional_path(final_rel, tuple_fraction);

   top_plan = create_plan(root, best_path);
   ```

   `subquery_planner()` plans this query (`parse`) and returns the `PlannerInfo` 
   structure created within it, `root->upper_rels[UPPERREL_FINAL]` is a list that
   is guaranteed to contains 1 final upper relation where you can get the cheapest
   path.

   > NOTE: `root->upper_rels` is an array of List<RelOptInfo> (indexed by kind 
   > `UpperRelationKind`), it is designed in such way because Postgres will have 
   > multiple upper relations for the same kind.
   >
   > For now, there is only 1 upper relation for each kind.

   Then you extract the cheapest path from the upper relation, and convert it to
   the final plan.

   > BTW, if you don't know what an upper relation is, check out the "Post scan/join planning"
   > in "optimizer/README"

5. Materialize the execution result if needed

   ```c
   /*
    * If creating a plan for a scrollable cursor, make sure it can run
    * backwards on demand.  Add a Material node at the top at need.
    */
   if (cursorOptions & CURSOR_OPT_SCROLL)
   {
      if (!ExecSupportsBackwardScan(top_plan))
         top_plan = materialize_finished_plan(top_plan);
   }
   ```

   Planner invoked from `exec_simple_query()` won't have this `CURSOR_OPT_SCROLL` 
   option set.

6. Artificially add a `Gather` node atop the generated plan, as a test. Note it
   only uses 1 worker. You should note that the condition should be simply:

   ```c
   if (debug_parallel_query != DEBUG_PARALLEL_OFF && top_plan->parallel_safe)
   ```

   The below part is to avoid regression in tests

   ```c
   top_plan->initPlan == NIL || debug_parallel_query != DEBUG_PARALLEL_REGRESS
   ```

   > We can add Gather even when top_plan has parallel-safe initPlans, but
   > then we have to move the initPlans to the Gather node because of
   > SS_finalize_plan's limitations.  **That would cause cosmetic breakage of
   > regression tests when debug_parallel_query = regress**, because initPlans
   > that would normally appear on the top_plan move to the Gather, causing
   > them to disappear from EXPLAIN output.  That doesn't seem worth kluging
   > EXPLAIN to hide, so skip it when debug_parallel_query = regress.


   ```c
   if (debug_parallel_query != DEBUG_PARALLEL_OFF &&
      top_plan->parallel_safe &&
      (top_plan->initPlan == NIL ||
         debug_parallel_query != DEBUG_PARALLEL_REGRESS))
   {
      Gather	   *gather = makeNode(Gather);
      Cost		initplan_cost;
      bool		unsafe_initplans;

      gather->plan.targetlist = top_plan->targetlist;
      gather->plan.qual = NIL;
      gather->plan.lefttree = top_plan;
      gather->plan.righttree = NULL;
      gather->num_workers = 1;
      gather->single_copy = true;
      gather->invisible = (debug_parallel_query == DEBUG_PARALLEL_REGRESS);

      /* Transfer any initPlans to the new top node */
      gather->plan.initPlan = top_plan->initPlan;
      top_plan->initPlan = NIL;

      /*
         * Since this Gather has no parallel-aware descendants to signal to,
         * we don't need a rescan Param.
         */
      gather->rescan_param = -1;

      /*
         * Ideally we'd use cost_gather here, but setting up dummy path data
         * to satisfy it doesn't seem much cleaner than knowing what it does.
         */
      gather->plan.startup_cost = top_plan->startup_cost +
         parallel_setup_cost;
      gather->plan.total_cost = top_plan->total_cost +
         parallel_setup_cost + parallel_tuple_cost * top_plan->plan_rows;
      gather->plan.plan_rows = top_plan->plan_rows;
      gather->plan.plan_width = top_plan->plan_width;
      gather->plan.parallel_aware = false;
      gather->plan.parallel_safe = false;

      /*
         * Delete the initplans' cost from top_plan.  We needn't add it to the
         * Gather node, since the above coding already included it there.
         */
      SS_compute_initplan_cost(gather->plan.initPlan,
                           &initplan_cost, &unsafe_initplans);
      top_plan->startup_cost -= initplan_cost;
      top_plan->total_cost -= initplan_cost;

      /* use parallel mode for parallel plans. */
      root->glob->parallelModeNeeded = true;

      top_plan = &gather->plan;
   }
   ```

7. Walk through the Plan tree, init all the Plans' `extParams` and `allParams` 
   fields. It has to do this to the `subplans` first because the values of these
   2 fields in `top_plan` relies on the ones in `subplans`

   > struct Plan:
   >
   > field `initPlans` stores the `SubPlan`s of those uncorrelated sub-queries
   >
   > extParam includes the paramIDs of all external PARAM_EXEC params
	> affecting this plan node or its children. **setParam params from the
	> node's initPlans are not included, but their extParams are**.
   >
   > allParam includes all the extParam paramIDs, plus the IDs of local
   > params that affect the node (i.e., **the setParams of its initplans**).
   > These are _all_ the PARAM_EXEC params that affect this node.


   ```c
   /*
   * If any Params were generated, run through the plan tree and compute
   * each plan node's extParam/allParam sets.  Ideally we'd merge this into
   * set_plan_references' tree traversal, but for now it has to be separate
   * because we need to visit subplans before not after main plan.
   */
   if (glob->paramExecTypes != NIL)
   {
      Assert(list_length(glob->subplans) == list_length(glob->subroots));
      forboth(lp, glob->subplans, lr, glob->subroots)
      {
         Plan	   *subplan = (Plan *) lfirst(lp);
         PlannerInfo *subroot = lfirst_node(PlannerInfo, lr);

         SS_finalize_plan(subroot, subplan);
      }
      SS_finalize_plan(root, top_plan);
   }
   ```

8. Adjust some plan tree representation details:


   ```c
   top_plan = set_plan_references(root, top_plan);
   /* ... and the subplans (both regular subplans and initplans) */
   Assert(list_length(glob->subplans) == list_length(glob->subroots));
   forboth(lp, glob->subplans, lr, glob->subroots)
   {
      Plan	   *subplan = (Plan *) lfirst(lp);
      PlannerInfo *subroot = lfirst_node(PlannerInfo, lr);

      lfirst(lp) = set_plan_references(subroot, subplan);
   }
   ```

9. Query compilation option
 
   > TODO: take a deeper look at this

   ```c
   result->jitFlags = PGJIT_NONE;
   if (jit_enabled && jit_above_cost >= 0 &&
      top_plan->total_cost > jit_above_cost)
   {
      result->jitFlags |= PGJIT_PERFORM;

      /*
         * Decide how much effort should be put into generating better code.
         */
      if (jit_optimize_above_cost >= 0 &&
         top_plan->total_cost > jit_optimize_above_cost)
         result->jitFlags |= PGJIT_OPT3;
      if (jit_inline_above_cost >= 0 &&
         top_plan->total_cost > jit_inline_above_cost)
         result->jitFlags |= PGJIT_INLINE;

      /*
         * Decide which operations should be JITed.
         */
      if (jit_expressions)
         result->jitFlags |= PGJIT_EXPR;
      if (jit_tuple_deforming)
         result->jitFlags |= PGJIT_DEFORM;
   }
   ```


### subquery_planner()

> Recursion happens from this function
>
> Call stack
>
> 1. subquery_planner
> 2. grouping_planner
> 3. query_planner
> 4. make_one_rel
> 5. set_base_rel_sizes
> 6. set_rel_size
> 7. set_subquery_pathlist
> 8. subquery_planner

1. It is guaranteed that `subquery_planner()` returns a `PlannerInfo` containing
   the final path in its `PlannerInfo.upper_rels[UPPERREL_FINAL]`. Caller should 
   invoke `fetch_upper_rel(root, UPPERREL_FINAL, NULL)` to extract the final upper 
   relation, then extract the cheapest path.

   Like what `standard_planner()` does:

   ```c
   root = subquery_planner(glob, parse, NULL, false, tuple_fraction, NULL);
   final_rel = fetch_upper_rel(root, UPPERREL_FINAL, NULL);
   best_path = get_cheapest_fractional_path(final_rel, tuple_fraction);
   ```

2. Variable initialization

   ```c
   /* PlannerInfo that will be returned */
   PlannerInfo *root;

   /* 
    * If the `Query.withCheckOptions` field is not NIL, call preprocess_expression() on 
    * their `qual` field. Then we will have a list of NEW withCheckOptions.
    *
    * Do `Query->withCheckOptions = newWithCheckOptions`
    */
   List	   *newWithCheckOptions;
   /*
    * Similar to what we do with withCheckOptions, preprocess the havingClause 
    * and append it to the newHaving list.
    *
    * QUES: this is more complex than the operations performed to withCheckOptions,
    * I need to take a deeper look at it.
    */
   List	   *newHaving;
   /* 
    * If we have an outer join in RTEs. This variable gets inited by inspecting 
    * every RTE and checking its type.
    *
    * Why we care about outer joins:
    * 1. We can potentially reduce it to an inner/anti join
    * 2. Check if we can remove useless RTE_RESULT entries 
    */
   bool		hasOuterJoins;
   /* 
    * If we have RTE_RESULT RTEs in rtable, true. Used to check if we can remove
    * some RTE_RESULT entries that are useless in remove_useless_result_rtes().
    */
   bool		hasResultRTEs;
   /* The final upper relation, we extract paths from it */
   RelOptInfo *final_rel;
   /* You know, it is a list element */
   ListCell   *l;
   ```

3. PlannerInfo initial setup

   ```c
   /* Create a PlannerInfo data structure for this subquery */
   root = makeNode(PlannerInfo);
   root->parse = parse;
   root->glob = glob;
   root->query_level = parent_root ? parent_root->query_level + 1 : 1;
   root->parent_root = parent_root;
   root->plan_params = NIL;
   root->outer_params = NULL;
   root->planner_cxt = CurrentMemoryContext;
   root->init_plans = NIL;
   root->cte_plan_ids = NIL;
   root->multiexpr_params = NIL;
   root->join_domains = NIL;
   root->eq_classes = NIL;
   root->ec_merging_done = false;
   root->last_rinfo_serial = 0;
   // Add `Query->resultRelation` to `all_result_relids`
   root->all_result_relids =
      parse->resultRelation ? bms_make_singleton(parse->resultRelation) : NULL;
   root->leaf_result_relids = NULL;	/* we'll find out leaf-ness later */
   root->append_rel_list = NIL;
   root->row_identity_vars = NIL;
   root->rowMarks = NIL;
   memset(root->upper_rels, 0, sizeof(root->upper_rels));
   memset(root->upper_targets, 0, sizeof(root->upper_targets));
   root->processed_groupClause = NIL;
   root->processed_distinctClause = NIL;
   root->processed_tlist = NIL;
   root->update_colnos = NIL;
   root->grouping_map = NULL;
   root->minmax_aggs = NIL;
   root->qual_security_level = 0;
   root->hasPseudoConstantQuals = false;
   root->hasAlternativeSubPlans = false;
   root->placeholdersFrozen = false;
   root->hasRecursion = hasRecursion;
   if (hasRecursion)
      root->wt_param_id = assign_special_exec_param(root);
   else
      root->wt_param_id = -1;
   root->non_recursive_path = NULL;
   root->partColsUpdated = false;
   ```

   QUES: I do not understand most of these fields.

4. Plan CTEs

   ```c
   /*
    * If there is a WITH list, process each WITH query and either convert it
    * to RTE_SUBQUERY RTE(s) or build an initplan SubPlan structure for it.
    */
   if (parse->cteList)
      SS_process_ctes(root);
   ```

   Iterate over all the CTEs, and plan it, Postgres has 3 options:

   1. Ignore it, if it is a select and not used/referenced anywhere
   2. Inline it, convert `RTE_CTE` to `RTE_SUBQUERY`
   
      ```sql
      with one as (select 1) select * from one; 
      
      select * from (select 1);
      ```
      
   3. Materialize it, plan it separately and add it to `Plan.initPlans`
   
      > For non-correlated subqueries, Postgres materializes it as well.

5. Pre-process querytree for the `MERGE` command, i.e., transform `MERGE` to a
   join:
   
   1. Add a join RTE to `parse->rtable`
   2. Create a `JoinExpr` and make it the only element in `parse->jointree->fromlist`
   3. Additional transformations needed by execution
   
      QUES: I do not understand this part

6. `preprocess_relation_rtes()`

   Fetch relation metadata, then use metadata to:
   
   > First metadata access in the optimizer

   > Note that this step does not descend into sublinks and subqueries; if we 
   > pull up any sublinks or subqueries below, their relation RTEs are processed
   > just before pulling them up.

   1. If this relation has `RangeTblEntry.inh` set and it has no children, clear 
      the flag
      
      > QUES: TOCTOU???
      >
      > `Query.table` was created by analyzer, and now it is used by the planner.
    
   2. Collect non-NULL attribute numbers
   3. Update `Query.targetlist`, for [**virtual** generated columns][vi_g_c], replace `Var`
      with the corresponding generation expression.
      
      [vi_g_c]: https://www.postgresql.org/docs/current/ddl-generated-columns.html

7. `replace_empty_jointree()` 

   1. Adds a dummy `RTE_RESULT` range table entry to `Query.rtable`
   2. Adds a reference to this RTE in `jointree.fromlist`
   
   if the `jointree` is empty (`NIL`).

   > `RTE_RESULT` represents an empty FROM clause; such RTEs are added by the 
   > planner, they're not present during parsing or rewriting

   ```c
   /*
    * If the FROM clause is empty, replace it with a dummy RTE_RESULT RTE, so
    * that we don't need so many special cases to deal with that situation.
    */
   replace_empty_jointree(parse);
   ```

   ```sql
   steve=# explain select 1;
                   QUERY PLAN                
   ------------------------------------------
   Result  (cost=0.00..0.01 rows=1 width=4)
   (1 row)
   ```

   I think this is a workaround for the limitation that the volcano model behaves
   like an iterator. To make the evaluator compute something, it has to yield an
   element.

8. `pull_up_sublinks()`

   Optimization/query rewrite, converts sublinks correlated subqueries in
   `WHERE <SUBLINK>` and `JOIN xxx ON <SUBLINK>` to joins, where `<SUBLINK>`
   could be:

   > This optimization can only be applied in the following cases:
   > 
   > > QUES: why? The explanation in the function comment is not that detailed
   > 
   > 1. It only works for top-level `WHERE` `JOIN/ON` clauses
   > 2. If it appears in the `ON` clause of an outer join, the subquery only 
   >    references the nullable side of join
   
   * `lefthand op ANY (sub-select)` => semi join
   * `EXISTS (sub-select)` => semi join
   * `NOT EXISTS (sub-select)` => anti join
   
   Example:
   
   ```sql
   create table teacher (id int, name text);
   create table teaches (subject text, teacher int);
   
   select * from teachers 
   where exists (select * from teaches where teaches.teacher = teachers.id);
   ```
   
   Before the rewrite:
   
   ```yaml
   rtable: [(RTE_RELATION, teachers)]
   jointree: 
     fromlist: [RangeTblRef(1)]
     quals: SubLink(EXISTS ...)
   ```
   
   After the rewrite
   
   ```yaml
   rtable: [(RTE_RELATION, teachers), (RTE_RELATION, teaches)]
   jointree: 
     fromlist: [
        JoinExpr:
          type: semi
          larg: RangeTblRef(1)
          rarg: RangeTblRef(2)
          quals: teaches.teacher = teacher.id
     ]
     quals: NULL
   ```
   
   The converted SQL will be:
   
   ```sql
   select * from
   teacher semi join teaches
   on teacher.id = teaches.teacher;
   ```
   
   `pull_up_sublinks()` does the transformations described above, it looks for 
   sublinks in the `jointree`, then:
   
   1. Add new range table entries. We won't create `RTE_JOIN` entries for the 
      created semi/anti-joins
   2. Replace the `jointree.fromlist` with a `JoinExpr` of `semi/anti-join`
   
   Implementation:
   
   ```c
   /*
    * Recurse through **jointree** nodes for pull_up_sublinks()
    *
    * In addition to returning the possibly-modified jointree node, we return
    * a relids set of the contained rels into *relids.
    */
   static Node *
   pull_up_sublinks_jointree_recurse(PlannerInfo *root, Node *jtnode,
   								  Relids *relids)
   ```
   
   `pull_up_sublinks()` uses this function to recursively **scans** the `jointree`
   nodes (the jointree node itself and the nodes in `fromlist` of types, i.e.,
   `FromExpr/RangeTblRef/JoinExpr`). For every `jtnode` it receives, **collects its 
   rtindexes**, clones it (if it is going to be modified, i.e., when `jtnode` is of 
   type `FromExpr` and `JoinExpr`. Otherwise, `jtnode` will be returned as-is) 
   and returns the clone.
    
   `jtnode` can be of the following types, which are the types that could appear
   in `Query.jointree`:
    
   * `FromExpr`: Iterate over the `fromlist`, call `pull_up_sublinks_jointree_recurse()`
     on every list item, which will return the re-built version of elements, use them to
     build a new `fromlist`
   * `RangeTblRef`: collect rtindex
   * `JoinExpr`: Scan `larg` and `rarg`
   
   TODO: revisit the `JoinExpr` branch
   
   `pull_up_sublinks_qual_recurse()` does the actual transformations, modifies
   `Query.rtable` and updates `jointree.fromlist`. QUES: I don't understand all
   the code here, but for the example SQL mentioned above, `joinexpr = convert_EXISTS_sublink_to_join()`
   modifies the `Query.rtable` and `pull_up_sublinks_qual_recurse()` updates the
   `jointree.fromlist`.
   
9. `preprocess_function_rtes()`

   > Should be invoked 
   >
   > * After `pull_up_sublinks()` as we want to pre-process the functions in
   >   the sublinks that just got pulled up.
   > * Before `pull_up_subqueries()` as new subqueries could be added in this
   >   step by Inlining functions (RTE_FUNCTION -> RTE_SUBQUERY).

   1. Simplify the `RangeTblFunction` nodes: constant-folding
   
      ```c
      /* Apply const-simplification */
      rte->functions = (List *)
          eval_const_expressions(root, (Node *) rte->functions);
      ```
      
   2. Inline it if 
   
      1. it is a **set-returning function** 
      2. it is written in **SQL** (not C)
      3. it is declared in the `FROM` clause
      4. It contains a simple `SELECT`
      
      convert the function body to a sub-`Query` (`RTE_FUNCTION` to a `RTE_SUBQUERY`),
      which may be further optimized by pulling it up.
  
      ```c
      /* Check safety of expansion, and expand if possible */
      funcquery: Query = inline_set_returning_function(root, rte);
      if (funcquery)
      {
              /* Successful expansion, convert the RTE to a subquery */
              rte->rtekind = RTE_SUBQUERY;
              rte->subquery = funcquery;
              rte->security_barrier = false;
                
             /*
              * Clear fields that should not be set in a subquery RTE.
              * However, we leave rte->functions filled in for the moment,
              * in case makeWholeRowVar needs to consult it.  We'll clear
              * it in setrefs.c (see add_rte_to_flat_rtable) so that this
              * abuse of the data structure doesn't escape the planner.
              */
              rte->funcordinality = false;
      }
      ``` 
      
      I haven't tried this myself because I don't know any functions that could
      be inlined.


10. `pull_up_subqueries()`

    A subquery will be pulled up if:
    
    1. It is a simple sub-query
    
       1. 不能有集合操作 (UNION, INTERSECT 等，UNION ALL 是特例，由另一分支处理)。
       2. 不能有聚合 (GROUP BY, HAVING, hasAggs)。
       3. 不能有窗口函数 (hasWindowFuncs)。
       4. 不能有排序或限制 (ORDER BY, LIMIT, OFFSET, DISTINCT)。
       5. 不能有 WITH 子句 (cteList)。
       6. 不能是安全屏障视图 (security_barrier)，否则会破坏行级安全。
       7. 不能有 FOR UPDATE/SHARE 锁，否则会改变加锁的语义。
       8. 目标列中不能包含易变函数 (VOLATILE functions)，否则拉平后可能导致函数被多次执行，产生非预期结果。
      
    2. It is a simple `UNION ALL` sub-query
    3. It is a simple `VALUES` rte: RTE_VALUES -> RTE_RESULT
    4. It is a `RTE_FUNCTION` whose `funcexpr` is a `Const`: RTE_FUNCTION -> RTE_RESULT
    
    QUES: why does this function handle cases other than sub queries

11. convert `UNION ALL` to `appendrel`

    ```c
    /*
    * If this is a simple UNION ALL query, flatten it into an appendrel. We
    * do this now because it requires applying pull_up_subqueries to the leaf
    * queries of the UNION ALL, which weren't touched above because they
    * weren't referenced by the jointree (they will be after we do this).
    */
    if (parse->setOperations)
    	flatten_simple_union_all(root);
    ```
    
    ```sql
    SELECT * FROM foo
    UNION ALL
    SELECT * FROM foo
    ```
       
    The `Query`tree of this query contains 2 `RTE_SUBQUERY` subqueries, the 
    top-level query is a set operation.

12. Scan the range table entries, collect some informations. Used for later
    processing. Check if some specific SQL features appear in the query, if 
    not, then we can skip processing them.
    
    1. root->hasJoinRTEs (bool): do we have any joins?
      
       Current usage: If we don't have joins, then in `preprocess_expression()`, 
       we don't need to invoke `flatten_join_alias_var()`.
      
    2. hasOuterJoins (bool): do we have outer joins?
    3. root->hasLateralRTEs (bool): if any RTEs have the `lateral` field set
    4. root->group_rtindex (rtindex): RT index of the first RTE_GROUP entry
    5. hasResultRTEs (bool): do we have `RTE_RESULT` RTEs
    6. root->qual_security_level (uint): Minimum security_level for quals.

13. If the `Query->resultRelation` is not `RangeTblEntry.inh`, then 
    `root->leaf_result_relids` should be set to it.
    
    ```c
	/*
	 * If we have now verified that the query target relation is
	 * non-inheriting, mark it as a leaf target.
	 */
	if (parse->resultRelation)
	{
		RangeTblEntry *rte = rt_fetch(parse->resultRelation, parse->rtable);

		if (!rte->inh)
			root->leaf_result_relids =
				bms_make_singleton(parse->resultRelation);
	}
    ```

    QUES: Why we init the `leaf_result_relids` field here? Because it relies on
    the `RangeTblEntry.inh` field? And the proceeding routines could update this
    field for the `RTE_RELATION` RTE entries? I only know that `pull_up_subqueries()`
    and `flatten_simple_union_all()` set this field for `RTE_SUBQUERY` RTE entries.
    
14. Check permissions of `(RTE_RELATION, RELKIND_VIEW)` RTE entries

    This code snippet was inroduced as a fix-up. Normally Postgres checks ACL
    at executor-startup

15. `preprocess_rowmarks()`

    ```c
    /*
    * Preprocess RowMark information.  We need to do this after subquery
    * pullup, so that all base relations are present.
    */
    preprocess_rowmarks(root);
    ```

    For all the **non-target** **base** relations, generate one `PlanRowMark`
    for each relation and store them in `PlannerInfo->rowMarks`:
    
    > target relation: here it means `Query.resultRelation`
    
    > base relations contain data, so that we can lock them

    1. Convert the `Query.rowMarks` (`List<RowMarkClause>`) added for `RTE_RELATION`
       entries (i.e., ignore the ones built for `RTE_SUBQUERY`) to a list of 
       `PlanRowMark`
       
       `RowMarkClause` are added to relations identified by `FOR UPDATE` and 
       `FOR SHARE`, which are not target relations in the current Implementation.
       
       QUES: why? From what I can tell, `PlanRowMark` contains moren information
       than `RowMarkClause`
    
    2. For all the non-target base relations that are not covered in the above step,
       create a `PlanRowMark` for it as well.
       
    What is a rowmark: A rowmark, the planner adds for a relation, is a mark 
    that tells the executor how to lock its rows. NOTE: this function (the 
    planner) does not lock rows, the executor locks them.
    
    This should be done after calling `pull_up_subqueries(root)`, as it could
    add more base relations.
    
    QUES: 
    
    * Why does this function only add rowmarks for **non-target** base 
      relations? Shouldn't the target relation (resultRelation) be locked as
      well? 
      
      Gemini says that the resultRelation will be locked by the `ModifyTable`
      node.
      
    * Why do we need to lock non-target base relations? I don't understand
      Gemini's explanation, it says this is realted "EvalPlanQual"
      
16. Set the `root->hasHavingQual` as `preprocess_expression()` needs it

    ```c
    /*
    * Set hasHavingQual to remember if HAVING clause is present.  Needed
    * because preprocess_expression will reduce a constant-true condition to
    * an empty qual list ... but "HAVING TRUE" is not a semantic no-op.
    */
    root->hasHavingQual = (parse->havingQual != NULL);
    ```

17. `preprocess_expression(PlannerInfo *root, Node *expr, int kind)`
 
    1. Constant-fold `expr` 
    2. Simplify qualifications
    3. Create `SubPlan` for sublinks
    4. Create Param nodes for inter-plan communication
    
    > The semantics of this function is not clear

    Argument `expr` is a node of kind `kind`. Here is the list of expression 
    kinds that are supported:
    
    ```c
    /* Expression kind codes for preprocess_expression */
    EXPRKIND_QUAL				0
      1. Query.havingQual
    EXPRKIND_TARGET:
      1. Query.targetList
      2. Query.returningList
      3. Query.onConflict->onConflictSet
      4. MergeAction.targetList
      
    EXPRKIND_RTFUNC				2
    EXPRKIND_RTFUNC_LATERAL		3
      1. RTE_FUNCTION `RangeTblEntry.functions` (List<RangeTblFunction>)
    
    EXPRKIND_VALUES				4
    EXPRKIND_VALUES_LATERAL		5
      1. RTE_VALUES `RangeTblEntry.values_list` (List<Expr>)
    EXPRKIND_LIMIT				6
      1. Query.limitOffset
      2. Query.limitCount
      3. WindowClause.startOffset
      4. WindowClause.endOffset
    EXPRKIND_APPINFO			7
      1. PlannerInfo.append_rel_list
    EXPRKIND_PHV				8
      1. PlaceHolderVer.phexpr
    EXPRKIND_TABLESAMPLE		9
      1. RTE_RELATION `RangeTblEntry.tablesample`
    EXPRKIND_ARBITER_ELEM		10
    
    EXPRKIND_TABLEFUNC			11
    EXPRKIND_TABLEFUNC_LATERAL	12
      1. RTE_TABLEFUNC `RangeTblEntry.tablefunc`
    
    EXPRKIND_GROUPEXPR			13
      1. RTE_GROUP `RangeTblEntry.groupexprs` 
    ```
    
    1. flatten_join_alias_vars(), we do this first because `Var`s extracted by 
       this step need to be `preprocess_expression()`ed.
       
       1. If A `Var` node references a column from a join result 
          (`RangeTblEntry.joinaliasvars`), we should update it to make it point
          to the actual table column. Why do we do this? If there is a qual on
          this Var, we can push the qual down to the table.
          
          If `Var.varno` points to a `RTE_JOIN`, we need to check the 
          `Var.varattno`'th element of `RangeTblEntry.joinaliasvars`. If this
          element points to a table column, task is complete. If not, we need
          to do this again.
          
       2. Whole-row `Var`s (`var->varattno == InvalidAttrNumber`) that reference
          JOIN relations are expanded into `RowExpr` constructs that name the 
          individual output Vars.  This is necessary since we will not scan 
          the JOIN as a base relation, which is the only way that the executor 
          can directly handle whole-row Vars.
       
       ```c
       /*
        * If the query has any join RTEs, replace join alias variables with
        * base-relation variables.  We must do this first, since any expressions
        * we may extract from the joinaliasvars lists have not been preprocessed.
        * For example, if we did this after sublink processing, sublinks expanded
        * out from join aliases would not get processed.  But we can skip this in
        * non-lateral RTE functions, VALUES lists, and TABLESAMPLE clauses, since
        * they can't contain any Vars of the current query level.
        */
       if (root->hasJoinRTEs &&
           !(kind == EXPRKIND_RTFUNC ||
     	     kind == EXPRKIND_VALUES ||
     	     kind == EXPRKIND_TABLESAMPLE ||
     	     kind == EXPRKIND_TABLEFUNC))
           expr = flatten_join_alias_vars(root, root->parse, expr);
       ```
       
       ```c
       if (IsA(node, Var))
       {
      		Var		   *var = (Var *) node;
      		RangeTblEntry *rte;
      		Node	   *newvar;
        
      		/* No change unless Var belongs to a JOIN of the target level */
      		if (var->varlevelsup != context->sublevels_up)
     			return node;		/* no need to copy, really */
      		rte = rt_fetch(var->varno, context->query->rtable);
      		if (rte->rtekind != RTE_JOIN)
     			return node;
      		if (var->varattno == InvalidAttrNumber)
      		{
     			/* Must expand whole-row reference */
       ```
       
       non-lateral RTE functions, VALUES list, TABLESAMPLE and table function
       nodes cannot reference other RTEs, which means they won't contain any 
       `Var` nodes, so we don't need to call `flatten_join_alias_vars()` on them.
       
    2. Evaluate const expressions. `RangeTblEntry.functions` has been processed
       in `preprocess_function_rtes`
       
       ```c
       /* Apply const-simplification */
       rte->functions = (List *)
           eval_const_expressions(root, (Node *) rte->functions);
       ```
       
       But if this RTE is lateral, it may have new `Var`s extracted by 
       `flatten_join_alias_vars()`, so we need to do it again, which means,
       if `kind == EXPRKIND_RTFUNC`, we can skip this step:
       
       ```c
       if (kind != EXPRKIND_RTFUNC)
           expr = eval_const_expressions(root, expr);
       ```
       
    3. Canoncalize qualifications, "convert them to the most useful form".
       Previously, this function converted qual to "AND of ORs" or "OR of ANDs",
       which was not that useful, so the routine got changed the function name 
       stayed.
    
       ```c
       /*
        * If it's a qual or havingQual, canonicalize it.
        */
       if (kind == EXPRKIND_QUAL)
           expr = (Node *) canonicalize_qual((Expr *) expr, false);
       ```
       
    4. For the sublinks that were not pulled up, make one `SubPlan` for them.
    
       ```c
       /* Expand SubLinks to SubPlans */
       if (root->parse->hasSubLinks)
           expr = SS_process_sublinks(root, expr, (kind == EXPRKIND_QUAL));
       ```
       
    5. Since we have sub-plans, create Param nodes for inter-plan communication

       ```c
       /* Replace uplevel vars with Param nodes (this IS possible in VALUES) */
       if (root->query_level > 1)
           expr = SS_replace_correlation_vars(root, expr);
       ```

18. preprocess_expression():

    1. Query.targetList
    2. Quals in `Query.withCheckOptions`
    3. Quals in `Query.returningList`
    4. Quals in `JoinExpr.qual`
    5. Quals in `Query.havingClause`
    6. LIMIT clauses in `Query.WindowClause`
    7. LIMIT clauses in `Query.limitOffset` and `Query.limitCount`
    8. `Query.onConflict`
    9. targetList and qual in `Query.mergeActionList`
    10. Qual in `Query.mergeJoinCondition`
    11. `PlannerInfo.append_rel_list`
    12. RTE_RELATION `RangeTblEntry.tablesample`
    13. **Partial** preprocessing RTE_SUBQUERY `RangeTblEntry.subquery`

        ```c
        /*
        * We don't want to do all preprocessing yet on the subquery's
        * expressions, since that will happen when we plan it.  But if it
        * contains any join aliases of our level, those have to get
        * expanded now, because planning of the subquery won't do it.
        * That's only possible if the subquery is LATERAL.
        */
        if (rte->lateral && root->hasJoinRTEs)
            rte->subquery = (Query *) flatten_join_alias_vars(root, root->parse, (Node *) rte->subquery);
        ```
        
        QUES: why?
        
    14. RTE_FUNCTION `RangeTblEntry.functions`
    15. RTE_TABLEFUNC `RnageTblEntry.tablefunc`
    16. RTE_VALUES `RnageTblEntry.tablefunc`
    17. RTE_GROUP `RangeTblEntry.groupexprs`
    
19. Set `RangeTblEntry.joinaliasvars` to `NIL` as they are no longer needed, all
    the references to it have been flattened by `flatten_join_alias_vars()`.
    
20. Similar to `flatten_join_alias_vars()`, replace the `Var`s in `targetList`
    and `havingClause` that reference group outputs with underlying expressions
    
    ```c
    /*
    * Replace any Vars in the subquery's targetlist and havingQual that
    * reference GROUP outputs with the underlying grouping expressions.
    *
    * Note that we need to perform this replacement after we've preprocessed
    * the grouping expressions.  This is to ensure that there is only one
    * instance of SubPlan for each SubLink contained within the grouping
    * expressions.
    */
    if (parse->hasGroupRTE)
    {
        parse->targetList = (List *)
            flatten_group_exprs(root, root->parse, (Node *) parse->targetList);
        parse->havingQual =
            group_exprs(root, root->parse, parse->havingQual);
    }
    ```
    
21. re-check if we have SRFs in `targetList` as `preprocess_expression(targetList)`
    might remove all SRFs

    ```c
    /* Constant-folding might have removed all set-returning functions */
    if (parse->hasTargetSRFs)
    	parse->hasTargetSRFs = expression_returns_set((Node *) parse->targetList);
    ```
    
22. Flatten nested `GROUPING SETS` clauses, needed by the next optimization, 
    move or copy qual from `HAVING` to `WHERE`.

    > See database/Postgres/pg_docs/Ch7_Queries/7.2_Table_Expressions.md
    
    ```c
    /*
    * If we have grouping sets, expand the groupingSets tree of this query to
    * a flat list of grouping sets.  We need to do this before optimizing
    * HAVING, since we can't easily tell if there's an empty grouping set
    * until we have this representation.
    */
    if (parse->groupingSets)
    {
        parse->groupingSets =
            expand_grouping_sets(parse->groupingSets, parse->groupDistinct, -1);
    }
    ```
    
23. **Move** or **copy** qualifications from `HAVING <qual>` to `WHERE <qual>` so that we could
    have tuples to aggregate.
    
    Cases where we cannot do this optimization:
    
    1. The qual contains aggregate functions, which is only available to 
       groups, not tuples
    2. The qual contains volatile functions, which will be executed once per
       group, not once per row.
    3. The qual references NULL columns generated by `GROUPING SETS`
       
       ```SQL
       select brand, segment
       from sales
       group by cube(brand, segment)
       HAVING segment IS NULL
       ```
       
       `segment IS NULL` should be applied to the groups created by grouping set
       `(brand)`, not tuples.
       
     4. If the qual is expensive to be executed once per row, estimating cost
        is hard, PG uses a heuristic rule here: clauses containing subplans are
        kept in `HAVING`
       
    Move or Copy, rules to follow:
    
    1. If this query has no empty GROUPING SETS, move the quals. Let me explain
       why we need to copy quals (i.e., qual still needs to be evaluated 
       against aggregation results) if we have an empty GROUPING SET, it is 
       because of an edge case: an empty GROUPING SET, creates a group that 
       contains all the rows of the table, even though this table is empty, 
       i.e., group is empty, the aggregation expressions still will emit 1 row, 
       which can be discarded by qual in `HAVING`, if we move the qual to `WHERE`, 
       this row will not be removed:
       
       ```SQL
       create table foo (id int);    -- an empty table
       
       postgres=# select count(*) from foo group by ();
        count 
       -------
           0
       (1 row)
       
       postgres=# select count(*) from foo group by () having false;
        count 
       -------
       (0 rows)
       
       postgres=# select count(*) from foo where false group by ();
        count 
       -------
           0
       (1 row)
       ```

24. Reduce join length

    1. outer join to plain inner join
    
       ```sql
       SELECT ... FROM a LEFT JOIN b ON (...) WHERE b.<ANY COLUMN> = 42;
       ```

    2. outer join to anti join
    
       If the outer join's qual (`JoinExpr.qual`) are strict for any nullable
       `Var` that was forced NULL by higher qual (WHERE, `FromExpr.qual`), 
       then this outer join only returns the rows that are null-extended.
       
       This optimization is different from 1, it has 2 conditions
       on the query tree:
       
       1. outer join `JoinExpr.qual` should be strict for "any column" from 
          the nullable table (if this is a left outer join, nullable table 
          is the right table)
       2. Where qual `FromExpr.qual` filters out rows whose that column ("any 
          column" mentioned in 1) is NOT NULL
      
       As long as the optimizer finds 1 such "any column" exists, it can perform
       this optimization.
       
       Why the optimizer can do this transformation, let's use the below query
       as an example. `foo.<Whatever> = bar.column`, operator `=` is strict for
       `bar.column`, which means if `bar.column` is NULL, the whole qual is NULL,
       which behaves similarly to `false`, which means the join operation won't 
       match, as long as `bar.column` is NULL. The join result of an outer join
       contains 2 kinds of rows:
       
       1. matched
       2. NULL-extened, i.e., not matched
       
       Since if `bar.column` is NULL, the join qual evaluates to false, then the
       rows of the matched kind won't have rows whose `bar.column` is NULL. Rows
       with `bar.column = NULL` only exists in NULL-extended rows.
       
       And, condition 2 only wants rows whose `bar.column` is NULL, i.e., the 
       NULL-extended rows, the rows that do not have a match. This is exactly what
       anti-join does.
       
       ```sql
       SELECT * FROM foo LEFT JOIN bar
       ON foo.<Whatever> = bar.column
       WHERE bar.column IS NULL;
       ```
      
       > NOTE: by definition, anti-join only returns columns from the left table,
       > a query like:
       >
       > select * from foo left join bar on foo.id=bar.id where bar.id IS NULL
       >
       > returns columns from both table foo and bar, but PG still does the 
       > optimization.
      
    3. Flip right outer join to left outer join
    
       This saves some code here and in some later planner routines; The main 
       benefit is to reduce the number of jointypes that can appear in SpecialJoinInfo
       nodes.

    ```c
    /*
    * If we have any outer joins, try to reduce them to plain inner joins.
    * This step is most easily done after we've done expression
    * preprocessing.
    */
    if (hasOuterJoins)
        reduce_outer_joins(root);
    ```
    
    This should be done after preprocess_expression(), to make find **strict**
    qual easier.
    
    QUES: why does it check operator/function stictness rather than evaluating
    the qual (`qual(NULL) == false`)

    ```sql
    create table foo (id int);
    create table bar (id int);
    create table buzz (id int);
    
    select * from
    foo join bar on foo.id = bar.id left join buzz on bar.id = buzz.id
    where buzz.id IS NOT NULL;
    ```
    
    rtable: 
    
    ```yaml
    1: foo
    2: bar
    3: foo join bar 
    4: buzz 
    5: foo join bar left join buzz
    ```
    
    jointree: 
    
    ```yaml
    FromExpr:
      fromlist": [
        JoinExpr:
          larg: 
            JoinExpr:
              larg: RangeTblRef(1)
              rarg: RangeTblRef(2)
          rarg: RangeTblRef(4)
      ],
      qual: 
        NullTest:
          arg:
            Var:
              varno: 4
              varattno: 1
    ```
    
    pass1 state:

    ```json
    {
        "relids": [1, 2, 4],
        "contains_outer": true,
        "sub_states": [
            {
                "relids": [1, 2, 4],
                "contains_outer": true,
                "sub_states": [
                    {
                        "relids": [1, 2],
                        "contains_outer": false,
                        "sub_states": [
                            {"relids": [1], "contains_outer": false, "sub_states": [] },
                            {"relids": [2], "contains_outer": false, "sub_states": [] }
                        ]
                    },
                    {
                        "relids": [4],
                        "contains_outer": false,
                        "sub_states": []
                    }
                ]
            }
        ]
    }
    ```
    
25. 
    

#### grouping_planner()
##### query_planner() (within grouping_planner, line 1632)

###### make_one_rel() (within query_planner())
#### set_cheapest() (within subquery_planner)
### fetch_upper_rel() (within standard_palner)
### get_cheapest_fractional_path() (within standard_planner())


### create_plan() (within standard_planner())

### SS_finalize_plan(root, plan) (within standard_planner())
### set_plan_references() (within standard_planner())

TODO: document the changes in detail here