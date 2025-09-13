* initPlan (List<SubPlan>): List of uncorrelated, expr (return 1 row that has 
  only 1 column) subquery. initPlans will be executed only once before executing
  the main plan, then the query result will be stored in `EState.es_param_exec_vals[paramID]`
  
  > Difference between initPlan and SubPlans
  > 
  > 1. They are both subqueries, but different kinds of subqueries: initPlan 
  >    is uncorrelated, subPlan could be correlated
  > 2. initPlan will be executed only once, before the executor executes the 
  >    main plan
  > 3. They share the same data structure `struct SubPlan`
 

* extParams (bitmap containing paramId (int)): includes the paramIDs of all 
  external PARAM_EXEC params affecting this plan node or its children.

  > QUES: the code comment only explains what this field is, not what it is for.

* allParams (bitmap containing paramId (int)): Dependency list

  Includes all the extParam paramIDs, plus the IDs of local params that affect 
  the node (i.e., the setParams of its initPlans).  These are _all_ the PARAM_EXEC 
  params that affect this node.

  It is used to decide which nodes to reset when we need to re-scan.

  For example, a parameterized index scan needs to produce different results if the 
  parameter changes. Some nodes, like Sort and Materialize, cache the data they 
  output so that they can cheaply produce the same output again.

  But, if any of the parameters listed in allParam change, then the node needs 
  to throw away any cached data and reread its input.

  As the input will have changed due to the different parameter, the output will 
  also change.