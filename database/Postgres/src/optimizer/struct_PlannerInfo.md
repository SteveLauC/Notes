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

* plan_params (List<PlannerParamItem>): parameters that will be passed to sub-queries.

* outer_params (IDs): contains the paramIds of PARAM_EXEC Params that outer query 
  levels will make available to this query level.

  > QUES: I do not understand this

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

* initPlan: (could be wrong) Gemini said this is the list of non-correlated sub-query 
  plans

* cte_plan_ids:

* 

