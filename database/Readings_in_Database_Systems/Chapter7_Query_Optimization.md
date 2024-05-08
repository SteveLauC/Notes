1. Query optimization is the hardest part of a database system, so it is no 
   surprise they remain a clear differentiator for mature commercial DBMSs.

2. Always remeber that the no query optimizer is truly producing the optimal plan,
   there are 2 reasons for it:

   1. They all take a guess on the query cost, in a case where there is no 
      statistic, they can use the random value.

   2. since the search space can be huge, and the optimizer cannot take too long
      to inspect every plan in the space, so they will use heuristics to limit
      the search space.

3. Serious query optimizers usually have a tendency to evolve to handle richer
   workloads and more corner cases, i.e., more `if` statments.

4. There are 2 reference architectures for query optimization from the early days
   of database research that cover most of the serious optimizer implementations
   today:
  
   1. The System R optimizer by Selinger et al.
   2. A series projects: Exodus, Volcano, and Cascades by Goetz Graefe

5. What is Adaptive Query Processing

   A typical database will:
   
   1. Parse the query
   2. Generate plan
   3. Execute the plan
   
   During execution, we won't update the plan. While with Adaptive Query Processing,
   execution plan could be changed mid-query.
   