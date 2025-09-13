* initPlan (List<SubPlan>): List of uncorrelated, expr (return 1 row that has 
  only 1 column) subquery. initPlans will be executed only once, which is
  different from SubPlans (though they share the same data structure)
 

* extParams (bitmap containing paramId (int)): 
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