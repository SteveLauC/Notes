1. `planner()` is the entry point for the query optimizer, it is basically a 
   dispatcher for the custom planner and the standard planner.
   
   ```c
   PlannedStmt *
   planner(Query *parse, const char *query_string, int cursorOptions,
		ParamListInfo boundParams)
   {
       PlannedStmt *result;
   
       if (planner_hook)
           result = (*planner_hook) (parse, query_string, cursorOptions, boundParams);
       else
           result = standard_planner(parse, query_string, cursorOptions, boundParams);
    
       pgstat_report_plan_id(result->planId, false);
    
       return result;
   }
   ```

2. `standard_planner()` is Postgres's built-in planner.

3. `struct Plan` (defined in "src/include/nodes/plannodes.h") is the base type 
   for all the plan nodes

   ```c
   /* ----------------
    *		Plan node
    *
    * All plan nodes "derive" from the Plan structure by having the
    * Plan structure as the first field.  This ensures that everything works
    * when nodes are cast to Plan's.  (node pointers are frequently cast to Plan*
    * when passed around generically in the executor)
    *
    * We never actually instantiate any Plan nodes; this is just the common
    * abstract superclass for all Plan-type nodes.
    * ----------------
    */
   typedef struct Plan
   {
    	pg_node_attr(abstract, no_equal, no_query_jumble)
    
    	NodeTag		type;
    
    	/*
    	 * estimated execution costs for plan (see costsize.c for more info)
    	 */
    	/* count of disabled nodes */
    	int			disabled_nodes;
    	/* cost expended before fetching any tuples */
    	Cost		startup_cost;
    	/* total cost (assuming all tuples fetched) */
    	Cost		total_cost;
    
    	/*
    	 * planner's estimate of result size of this plan step
    	 */
    	/* number of rows plan is expected to emit */
    	Cardinality plan_rows;
    	/* average row width in bytes */
    	int			plan_width;
    
    	/*
    	 * information needed for parallel query
    	 */
    	/* engage parallel-aware logic? */
    	bool		parallel_aware;
    	/* OK to use as part of parallel plan? */
    	bool		parallel_safe;
    
    	/*
    	 * information needed for asynchronous execution
    	 */
    	/* engage asynchronous-capable logic? */
    	bool		async_capable;
    
    	/*
    	 * Common structural data for all Plan types.
    	 */
    	/* unique across entire final plan tree */
    	int			plan_node_id;
    	/* target list to be computed at this node */
    	List	   *targetlist;
    	/* implicitly-ANDed qual conditions */
    	List	   *qual;
    	/* input plan tree(s) */
    	struct Plan *lefttree;
    	struct Plan *righttree;
    	/* Init Plan nodes (un-correlated expr subselects) */
    	List	   *initPlan;
    
    	/*
    	 * Information for management of parameter-change-driven rescanning
    	 *
    	 * extParam includes the paramIDs of all external PARAM_EXEC params
    	 * affecting this plan node or its children.  setParam params from the
    	 * node's initPlans are not included, but their extParams are.
    	 *
    	 * allParam includes all the extParam paramIDs, plus the IDs of local
    	 * params that affect the node (i.e., the setParams of its initplans).
    	 * These are _all_ the PARAM_EXEC params that affect this node.
    	 */
    	Bitmapset  *extParam;
    	Bitmapset  *allParam;
   } Plan;
   ```
   
   Fields to note:
   
   * `total_cost`: assuming all the tuples will be fetched, the cost of executing 
     this plan
   * `startup_cost`: cost expended before we fetch any tuples, think a statement
     with `LIMIT` caluse.
   * `plan_rows`: number of rows that this plan would emit
   * `row_width`: average row width, if the parent plan is a blocking operator, this 
     is used to estimate the size of the buffer needed by the blocking operator.
   * `lefttree` and `righttree`: input plans.  2 input plans for a join plan.
     Otherwise, we would have 1 input plan.