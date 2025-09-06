* planner()
  * standard_planner()
    * subquery_planner()
      * grouping_planner()
        * query_planner()
          * make_one_rel()
      * set_cheapest()

    * get_cheapest_fractional_path()
    * create_plan()
    * set_plan_references()

------------------

# planner()

## standard_planner()

Initializes `struct PlannerGlobal` and `struct PlannerInfo`

### subquery_planner()

1. `replace_empty_jointree()` adds a dummy `RTE_RESULT` range table entry if
   the range table list is empty.

   ```c
   /*
    * If the FROM clause is empty, replace it with a dummy RTE_RESULT RTE, so
    * that we don't need so many special cases to deal with that situation.
    */
   replace_empty_jointree(parse);
   ```

   ```sql
   steve=# explain select 1;
                   QUERY PLAN                
   ------------------------------------------
   Result  (cost=0.00..0.01 rows=1 width=4)
   (1 row)
   ```

2. `pull_up_sublinks()`

   What is a sublink? Difference between sublink and subquery?

   There is a type `struct SubLink` in `src/include/nodes/primnodes.h`, you can 
   read its documentation.  Generally, a sublink is a `ANY/EXISTS/...` claude 
   after `WHERE/HAVING`; subquery is the clause after `FROM`.


2. `pull_up_subqueries()` can simplify queries like

   ```sql
   select * from (select * from table)
   ```

   ```c
   /*
    * Check to see if any subqueries in the jointree can be merged into this
    * query.
    */
   pull_up_subqueries(root);
   ```

3. I do not understand why we can still see Views in the range table list, they 
   should be expanded by the rewriter

   ```c
    * Note, however, that we do need to check access permissions for any view
    * relations mentioned in the query, in order to prevent information being
    * leaked by selectivity estimation functions, which only check view owner
    * permissions on underlying tables (see all_rows_selectable() and its
    * callers).  This is a little ugly, because it means that access
    * permissions for views will be checked twice, which is another reason
    * why it would be better to do all the ACL checks here.
    */
   foreach(l, parse->rtable)
   {
        RangeTblEntry *rte = lfirst_node(RangeTblEntry, l);

        if (rte->perminfoindex != 0 &&
            rte->relkind == RELKIND_VIEW)
        {
            RTEPermissionInfo *perminfo;
            bool		result;

            perminfo = getRTEPermissionInfo(parse->rteperminfos, rte);
            result = ExecCheckOneRelPerms(perminfo);
            if (!result)
                aclcheck_error(ACLCHECK_NO_PRIV, OBJECT_VIEW,
                                get_rel_name(perminfo->relid));
        }
   }
   ```
#### grouping_planner()
##### query_planner() (within grouping_planner, line 1632)

###### make_one_rel() (within query_planner())
#### set_cheapest() (within subquery_planner)
### get_cheapest_fractional_path() (within standard_planner())

### create_plan() (within standard_planner())