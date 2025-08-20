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