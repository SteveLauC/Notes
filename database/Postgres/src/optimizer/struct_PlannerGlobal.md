# Overview

This type contains information about planning a whole SQL statement (and thus 
Global). Different from `struct PlannerInfo`, which only contains information 
about planning a single `Query`. (Think about SQL with subqueries).  It will
be created in `standard_planner()`.


# Fields

* boundsParams:

  type: ParamListInfo 

  External parameters passed to `planner()` and `standard_planner()`. The most 
  typical case is the parameters in prepared parameters.

* subplans/subpaths/subroots

  * subroots: `struct PlannerInfo`s of sub-queries
  * subpaths: `struct Paths` of sub-queries
  * subplans: plans of sub-queries, they are made from `subpaths`

  ```c
  // `build_subplan()` in backend/optimizer/plan/subselect.c

  root->glob->subplans = lappend(root->glob->subplans, plan);
  root->glob->subpaths = lappend(root->glob->subpaths, path);
  root->glob->subroots = lappend(root->glob->subroots, subroot);
  ```

  TODO: check the backtrace in `build_subplan()`, understand the call stack. I tried but 
  had no idea why `errbacktrace()` didn't work.

* rewindPlanIDs: (could be wrong) some sub-plans need to support the rewind the 
  operation, i.e., start over and execute again. This field stores the indexes 
  to such sub-plans. 

  QUES: I do not quite understand this

* finalrtable: (could be wrong) the final, flat range table list

* allRelids: (could be wrong) indexes to the `finalrtable` that point to a relation range table entry

* prunableRelids: 

* finalrteperminfos: `finalrtable` permission info

* finalrowmarks: (could be wrong) row level locks

* resultRelations: indexes to `finalrtable` that point to result relations.

* appendRelations: 

* partPruneInfos

* relationOids: the OIDs of the relations that this plan depends on

* invalItems: Compared to `relationOids`, this field stores more general 
  plan dependencies

  > Whenever a dependency changes, this plan no longer works

* paramExecTypes (List<Oid>): Type OIDs of the internal parameter

  > PARAM_EXEC: internal executor parameter, used for passing values into and 
  > out of sub-queries or from nestloop joins to their inner scans.
 
* lastPHId: Highest PlaceHolderVar ID

* lastRowMarkId: highest PlanRowMark ID

* lastPlanNodeId: highest plan node ID

* transientPlan/dependsOnRold: these 2 fields decide if the plan could be cached 
  and reused

  * transientPlan: If true, this plan depends on snapshot's `xmin` field.  If that
                   field changes, redo the planning.

    > database/Postgres/mvcc.md

  * dependsOnRole: If true, this plan depends on current role.

* parallelModeOk: can we safely execute this query using more workers?

* parallelModeNeeded: If it is safe to do parallel execution, is it required to 
  do so? i.e., will we have performance gains?

* maxParallelHazard: worst proparallel value
  
  * PROPARALLEL_SAFE		's' /* can run in worker or leader */
  * PROPARALLEL_RESTRICTED	'r' /* can run in parallel leader only */
  * PROPARALLEL_UNSAFE		'u' /* banned while in parallel mode */

* partition_directory: (could be wrong) cache for partition descriptor

* rel_notnullatts_hash: A hashmap<Relation_Oid, Bitmapset_of_not_null_attribute_numbers>



 


