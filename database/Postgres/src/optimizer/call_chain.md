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
    * set_plan_references()

------------------

# planner()

## standard_planner()

Initializes `struct PlannerGlobal` and `struct PlannerInfo`

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
   glob->parallelModeNeeded = glob->parallelModeOK &&
      (debug_parallel_query != DEBUG_PARALLEL_OFF);
   ```

   Developers can force Postgres to use parallel plan by setting it to [`on`, `regress`].

3. This chunk of code initializes the `tuple_fraction` variable, which will be
   used later in `grouping_planner()`:

   When option `CURSOR_OPT_FAST_PLAN` is set, PG uses `cursor_tuple_fraction`
   to init `tuple_fraction`. But since `cursor_tuple_fraction` should be in
   range `(0, 1)`, handle (correct) the edge value separately.

   > QUES: when will this option be set?
   >
   > future steve: For `exec_simple_query()`, this parameter is hardcoded to `CURSOR_OPT_PARALLEL_OK`
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
   > * 0: expect all tuples to be retrieved (normal case)
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

7. 


### subquery_planner()

1. `replace_empty_jointree()` adds a dummy `RTE_RESULT` range table entry if
   the range table list is empty.

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

2. `pull_up_sublinks()`

   What is a sublink? Difference between sublink and subquery?

   There is a type `struct SubLink` in `src/include/nodes/primnodes.h`, you can 
   read its documentation.  Generally, a sublink is a `ANY/EXISTS/...` claude 
   after `WHERE/HAVING`; subquery is the clause after `FROM`.

   `SELECT a FROM test WHERE NULL = ANY (SELECT b FROM bar)`, `NULL = ANY (SELECT b FROM bar)`
   is a ANY sublink.


2. `pull_up_subqueries()` can simplify queries like

   ```sql
   select * from (select * from table)
   ```

   ```c
   /*
    * Check to see if any subqueries in the jointree can be merged into this
    * query.
    */
   pull_up_subqueries(root);
   ```

3. I do not understand why we can still see Views in the range table list, they 
   should be expanded by the rewriter

   ```c
    * Note, however, that we do need to check access permissions for any view
    * relations mentioned in the query, in order to prevent information being
    * leaked by selectivity estimation functions, which only check view owner
    * permissions on underlying tables (see all_rows_selectable() and its
    * callers).  This is a little ugly, because it means that access
    * permissions for views will be checked twice, which is another reason
    * why it would be better to do all the ACL checks here.
    */
   foreach(l, parse->rtable)
   {
        RangeTblEntry *rte = lfirst_node(RangeTblEntry, l);

        if (rte->perminfoindex != 0 &&
            rte->relkind == RELKIND_VIEW)
        {
            RTEPermissionInfo *perminfo;
            bool		result;

            perminfo = getRTEPermissionInfo(parse->rteperminfos, rte);
            result = ExecCheckOneRelPerms(perminfo);
            if (!result)
                aclcheck_error(ACLCHECK_NO_PRIV, OBJECT_VIEW,
                                get_rel_name(perminfo->relid));
        }
   }
   ```
#### grouping_planner()
##### query_planner() (within grouping_planner, line 1632)

###### make_one_rel() (within query_planner())
#### set_cheapest() (within subquery_planner)
### fetch_upper_rel() (within standard_palner)
### get_cheapest_fractional_path() (within standard_planner())

### create_plan() (within standard_planner())