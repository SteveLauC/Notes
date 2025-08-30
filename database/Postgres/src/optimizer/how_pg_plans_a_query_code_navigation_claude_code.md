 PostgreSQL's 4-Stage Query Optimization

# Stage 1: Preprocessing

Location: plan/planner.c (subquery_planner function) and prep/ directory

Main function: subquery_planner() at planner.c:659

Key operations and code locations:

- Simplify scalar expressions: prep/prepqual.c - constant folding, function simplification
- Inline SQL functions: util/clauses.c - function inlining

- Flatten/pull up subqueries: prep/prepjointree.c - subquery flattening
- Simplify join tree: prep/prepjointree.c:150 - prepjointree.c handles join tree transformations

- Predicate pushdown: plan/initsplan.c - deconstruct_jointree() function
- Build equivalence classes: path/equivclass.c - equivalence class processing

# Stage 2: Plan "Scan" and "Join"

  Location: plan/planmain.c (query_planner function) and path/ directory

  Main function: query_planner() at planmain.c:54

  Key operations and code locations:
  - Base relation setup: planmain.c:170 - add_base_rels_to_query()
  - Generate scan paths: path/allpaths.c:221 - set_base_rel_pathlists()
  - Join order planning: path/allpaths.c:226 - make_rel_from_joinlist()
  - Standard join search: path/joinrels.c - implements System R algorithm
  - GEQO fallback: geqo/ directory - genetic query optimization for large joins

  Stage 3: Query Special Features Handling

  Location: plan/planner.c (grouping_planner function)

  Main function: grouping_planner() at planner.c:1412

  Key operations and code locations:
  - GROUP BY/aggregation: plan/planagg.c and grouping_planner processing
  - Window functions: Upper-level path generation in grouping_planner
  - DISTINCT: Upper-level planning in grouping_planner
  - UNION/INTERSECT/EXCEPT: prep/prepunion.c
  - ORDER BY: Upper-level path generation for sorting

  Stage 4: Postprocessing

  Location: plan/createplan.c and plan/setrefs.c

  Main functions:
  - createplan.c - Convert Paths to Plans
  - setrefs.c - Final plan adjustments

  Key operations and code locations:
  - Path to Plan conversion: plan/createplan.c - converts Path trees to executable Plans
  - Var reference fixing: plan/setrefs.c - adjusts variable references between plan levels
  - Subplan flattening: setrefs.c - flattens rangetables and removes unnecessary nodes

  Key Entry Points

  1. Main planner: planner() at planner.c:293
  2. Per-subquery planning: subquery_planner() at planner.c:659
  3. Scan/join planning: query_planner() at planmain.c:54
  4. Upper-level planning: grouping_planner() at planner.c:1412
  5. Join enumeration: make_one_rel() at allpaths.c:171

  The optimizer README at
  /Users/steve/Documents/workspace/postgres-fork/src/backend/optimizer/README provides
  additional architectural details about how these components work together.
