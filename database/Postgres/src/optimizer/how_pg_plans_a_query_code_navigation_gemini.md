The main entry point for the planner is the `planner` function in planner.c. This 
function orchestrates the entire process, calling other functions to handle each stage.

### 1. Preprocessing

This stage simplifies the query and gathers necessary information before the main 
planning begins. These steps are primarily handled within `subquery_planner` in 
planner.c and functions it calls.

*   **Simplify Scalar Expressions & In-line Functions**:
    * `eval_const_expressions()` (in util/clauses.c): This function performs 
      constant-folding for expressions. For example, it simplifies `2+2` to `4` 
      or `x AND false` to `false`.
    * `inline_function()` (in util/clauses.c): This handles the inlining of simple SQL functions.

*   **Simplify Join Tree**:
    *   **Pull up Sub-queries**: `pull_up_subqueries()` and `pull_up_sublinks()` (in subselect.c) are responsible for flattening subqueries into the parent query to allow for more optimal join ordering.
    *   **Reduce Join Strength**: `reduce_outer_joins()` (in `src/backend/optimizer/plan/joins.c`) attempts to convert outer joins to inner joins if `WHERE` clauses reject nulls from the nullable side.
    *   **Convert IN/EXISTS to Semi-Joins**: `convert_sublink_to_join()` (in subselect.c) transforms subselects in `EXISTS` or `IN` clauses into semi-joins.

*   **Later Preprocessing**:
    *   **Predicate Pushdown & Equivalence Classes**: `deconstruct_jointree()` (in planmain.c) and `distribute_qual_to_rels()` (in `src/backend/optimizer/plan/joins.c`) process `WHERE` and `ON` clauses. They determine the lowest possible evaluation level for each predicate and build `EquivalenceClass` structures for clauses like `a.x = b.y`.
    *   **Join Ordering Restrictions**: `SpecialJoinInfo` structs are created for non-inner joins to enforce join order constraints. The function `join_is_legal()` (in joinrels.c) checks these constraints during join planning.
    *   **Remove Useless Joins**: `remove_useless_joins()` (in `src/backend/optimizer/plan/joins.c`) removes joins whose inner side does not contribute any columns to the query result.

### 2. Plan "Scan" and "Join"

This is the core optimization phase, where the planner finds the cheapest way to execute the `FROM`/`WHERE`/`ON` parts of the query. The main function for this stage is `query_planner()` in planmain.c.

*   **Identify Scan Methods**: `set_base_rel_pathlists()` (in allpaths.c) is called to generate all feasible access paths (e.g., sequential scan, index scan) for each base table.
*   **Find Best Join Order**: `query_planner` calls `make_one_rel()`, which in turn calls the join search algorithm.
    *   **Standard Join Search**: `standard_join_search()` (in `src/backend/optimizer/path/joinsearch.c`) implements the System R-style dynamic programming algorithm. It builds up join relations one level at a time, from 2-way joins up to the final N-way join, pruning more expensive paths at each step.
    *   **GEQO**: If the number of relations exceeds `geqo_threshold`, the planner switches to the genetic query optimizer, located in the geqo directory.

### 3. Query Special Feature Handling

After finding the best path for the core join query, this stage handles features like grouping, sorting, and set operations. This is primarily managed by `grouping_planner()` in planner.c.

*   **GROUP BY, DISTINCT, Aggregates, Window Functions**: `grouping_planner()` takes the best path from `query_planner` and adds plan nodes for these operations. Key functions include `plan_grouping_sets()`, `make_agg_plan()`, and `plan_window_functions()`.
*   **Set Operations (UNION/INTERSECT/EXCEPT)**: `grouping_planner` handles these by recursively calling the planner on the child queries and then adding a `SetOp` plan node.
*   **ORDER BY**: If the query requires a specific sort order, `grouping_planner` will add a `Sort` node to the plan by calling `make_sort_from_pathkeys()`, unless the best path from `query_planner` already provides the required ordering.

### 4. Postprocessing

In the final stage, the abstract `Path` tree is converted into an executable `Plan` tree, and some final cleanup is performed.

*   **Expand Path to Plan**: `create_plan()` (in createplan.c) walks the `Path` tree and creates the corresponding `Plan` tree. This is called from `grouping_planner`.
*   **Adjust Plan Representation**: `set_plan_references()` (in setrefs.c) performs final adjustments to the `Plan` tree. This includes:
    *   Fixing `Var` nodes to correctly reference the outputs of subplans.
    *   Flattening range tables.
    *   Tidying up the plan tree for the executor.