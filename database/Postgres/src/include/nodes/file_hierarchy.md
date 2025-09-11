* primenodes.h: Definitions for "primitive" node types, those that are used in 
  more than one of the parse/plan/execute stages of the query pipeline.
* parsenodes.h: parse tree nodes (including raw parser and analyzer)

  > RawStmt, Parse

* pathnodes.h: data types used during planning 

  > PlannerGlobal, PlannerInfo, RelOptInfo

* plannodes.h: query plan nodes
* execnodes.h: executor state nodes
