# Overview

SubPlan is a sub-plan's descriptor, it will have a `struct Plan` stored in `PlannedStmt.subplans`
indexed by `SubPlan.plan_id`.

# Fields

* set_param (List<param ID (int)>): Executor, once you executed me, store the 
  result in the paramExec slots identified by the indexes stored in this field