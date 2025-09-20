# Overview

This type contains information of planning a `Query`

# Fields

* parse: query tree

* glob: The `PlannerGlobal` instance

* query_level (u32): `PlannerInfo` will be for each sub-query, sub-queries can be 
  nested, this field stores its sub-query depth. (starts from 1)

* parent_root (* PlannerInfo): Since Postgres creates multiple nested `PlannerInfo`
  for sub-queries, these `PlannerInfo` are arranged in a tree.

  This field is a pointer to its parent `PlannerInfo`, if exists. Otherwise, it is
  NULL

* plan_params (List<PlannerParamItem>): Parameters that the current plan/query 
  should prepare for the subquery.

  When `subquery_planner()` plans the current plan and finds it references a 
  variable from the parent query, it modifies its parent query's 
  `PlannerInfo.plan_params`

  ```c
  typedef struct PlannerParamItem
  {
      pg_node_attr(no_copy_equal, no_read, no_query_jumble)

      NodeTag		type;

      Node	   *item;			/* the Var, PlaceHolderVar, or Aggref */
      int			paramId;		/* its assigned PARAM_EXEC slot number */
  } PlannerParamItem;
  ```

  `struct PlannerParamItem` contains the expression that should be provided and
  a slot number that specifies where the value should be put in `EState.es_param_exec_vals`

* outer_params (IDs): contains the paramIds of PARAM_EXEC Params that outer query 
  levels will make available to this query level.

  See the notes of field `plan_params`, after the planner edits the parent 
  PlannerInfo's `plan_params`, it stores the `EState.es_param_exec_vals` slot
  number in the current PlannerInfo's `outer_params`.

* simple_rel_array (RelOptInfo **): an array of `RelOptInfo`s that are either:
  
  * base relation
  * other relation

  This array are indexed by range table indexes (start from 1), so `simple_rel_array[0]`
  is always NULL. Range table that are not base or other relation will also have NULL.

  > backend/optimizer/README
  >
  > The optimizer builds a RelOptInfo structure for each base relation used in
  > the query.

  The `RelOptInfo` it builds will be stored in this field:

  ```c
  /* backend/optimizer/plan/planmain.c */

  /*
   * Set up arrays for accessing base relations and AppendRelInfos.
   */
  setup_simple_rel_array(root);
  ```

* simple_rel_array_size: the allocated array size of `simple_rel_array`

* simple_rte_array: This array also has length `simple_rel_array_size`. It has the 
  same content as `PlannerInfo->parse->rtable`, but that is a linked list, which is
  not good for random access. This field is an array, it is faster to access
  the nth element.

* append_rel_array: 

* all_baserels: `RelOptInfo.relids` of all the base relations. Initialized in `deconstruct_recurse()`

  > `RelOptInfo.relids` is a set of indexes to the `PlannerInfo->parse->rtable`

* outer_join_rels: `RelOptInfo.relids` of all the outer join relations

* all_query_rels: union of `all_baserels` and `outer_join_rels`

* join_rel_list (List<RelOptInfo>):  all the join relations we considered in this planning run

  ```c
  /* Is the given relation a join relation? */
  #define IS_JOIN_REL(rel)	\
    ((rel)->reloptkind == RELOPT_JOINREL || \
    (rel)->reloptkind == RELOPT_OTHER_JOINREL)
  ```

* join_rel_hash: a HashMap<Relids, RelOptInfo> for `join_rel_list` when the list
  goes too long

  ```c
  /*
    * Switch to using hash lookup when list grows "too long".  The threshold
    * is arbitrary and is known only here.
    */
  if (!root->join_rel_hash && list_length(root->join_rel_list) > 32)
      build_join_rel_hash(root);
  ```

* join_rel_level/join_cur_level

  join_rel_level is an array of List. During planning join order, join_rel_level[k]
  is a list of join relations for level k.  And join_cur_level is k

  > QUES: join_rel_level[0] 是所有基表，join_rel_level[1] 是所有两表连接。is this true?

* init_plans (List<SubPlan>): This field contains the uncorrelated subqueries, they
  will be transferred to `Plan.initPlans` in `SS_attach_initplans(root, plan)`.

* cte_plan_ids: It is a List of Plan ID (index of the subplan made for this CTE 
  in PlannedStmt.subplans), or -1 if Postgres decides to ignore or inline it.

  Postgres has 3 ways to plan a CTE:

  1. Ignore it, if it is a SELECT and not referenced anywhere (plan ID: -1)
  2. Inline it to make it a part of the query (plan ID: -1)
  3. Materialize it, make it a subplan (initPlan) (will have a positive plan ID)

  ```c
  /*
   * If there is a WITH list, process each WITH query and either convert it
   * to RTE_SUBQUERY RTE(s) or build an initplan SubPlan structure for it.
   */
  if (parse->cteList)
      SS_process_ctes(root);
  ```

* multiexpr_params (List<List<Param>>): Parameters of the return values of a 
  `MULTIEXPR_SUBLINK`

  ```sql
  CREATE TABLE targets (
      id int primary key,
      val1 text,
      val2 int
  );

  CREATE TABLE sources (
      id int primary key,
      s_val1 text,
      s_val2 int
  );

  UPDATE targets
  SET
      (val1, val2) = (SELECT s_val1, s_val2 FROM sources WHERE sources.id = targets.id)
  WHERE
      targets.id = 1;
  ```

  that subselect is a multi-expr sublink subquery. PARAM_MULTIEXPR is a plan-time
  thing, it will be replaced by `PARAM_EXEC` during execution.

* join_domains (List<JoinDomain>):

  TODO: revisit this 

* eq_classes (List<EquivalenceClasses>): 

* ec_merging_done (bool): Its core purpose is to signal that the process of 
  building and merging Equivalence Classes (ECs) is complete. Once this flag 
  is set to true, the set of EquivalenceClass nodes stored in `PlannerInfo.eq_classes`
  is considered stable and canonical.

* canon_pathkeys;

* left_join_clauses;

* right_join_clauses;

* full_join_clauses;

* join_info_list;

* last_rinfo_serial (int): Counter used to assign a unique ID (within a PlannerInfo 
  context) for every `RestrictInfo` created during planning.

  Postgres assigns a unique identifier to each `RestrictInfo` primarily for making 
  checking if a `RestrictInfo` is in a set easier.

-------------------------------------------------------------------------------

* all_result_relids (Relids, aka Bitmapset of RT index): RT indexes of all the 
  result relations (i.e., the relations this query will modify).

  It would contains `parse->resultRelation` if this is not a `SELECT` query, the 
  primary relation that will be modified by the query.
  
  For `UPDATE/DELETE/MERGE` across an inheritance or partitioning tree, the result
  rel's child relids are added (by `expand_single_inheritance_child()`)

* leaf_result_relids (relids, aka bitmapset of RT index): A "leaf" relation is 
  a table in the "hierarchy" that actually stores data.

  * In a partitioning setup, these are the final **partitions** where rows are 
    physically located.

  * In a inheritance setup, every table can store physical data, the tables that
    the executor will modify are the leaf tables.

-------------------------------------------------------------------------------

* append_rel_list (List<AppendRelInfo>): 

* row_identity_vars (List<RowIdentityVarInfo>);

* rowMarks;

* placeholder_list;

* placeholder_array

* placeholder_array_size pg_node_attr(read_write_ignore);

* placeholder_array_size

* fkey_list

* query_pathkeys

* group_pathkeys

* num_groupby_pathkeys

* window_pathkeys

* distinct_pathkeys

* sort_pathkeys

* setop_pathkeys;

* part_schemes 

* initial_rels

* upper_rels (Fixed-size (UPPERREL_FINAL+1) array of List<RelOptInfo>): contains 
  all the upper relations

* upper_targets (Fixed-size (UPPERREL_FINAL+1) array of `PathTarget`): 

* processed_groupClause
* processed_distinctClause
* processed_tlist
* update_colnos
* grouping_map
* minmax_aggs

* planner_cxt (MemoryContext): Memory context holding this PlannerInfo 

* total_table_pages
* tuple_fraction
* limit_tuples
* qual_security_level
* hasJoinRTEs
* hasLateralRTEs
* hasHavingQual
* hasPseudoConstantQuals
* hasAlternativeSubPlans
* placeholdersFrozen
* hasRecursion
* group_rtindex
* agginfos
* aggtransinfos
* numOrderedAggs
* hasNonPartialAggs
* hasNonSerialAggs
* wt_param_id
* non_recursive_path
* curOuterRels
* curOuterParams
* isAltSubplan
* isUsedSubplan
* join_search_private
* partColsUpdated
* partPruneInfos
