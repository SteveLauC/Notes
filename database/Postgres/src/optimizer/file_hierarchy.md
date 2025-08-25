# src/backend/optimizer

* geqo: GEQO join searchin
* path: Path generation and cost estimation
* plan: Main planning driver code
* prep: Preprocessing
* util: Miscellaneous

And

* src/backend/utils/adt/selfuncs.c:  operator-specific selectivity functions


## src/backend/optimizer/geqo

geqo/geqo_copy.c boring support code
geqo/geqo_cx.c unused method for generating a mutated tour
geqo/geqo_erx.c active method for generating a mutated tour
geqo/geqo_eval.c evaluate cost of tour
geqo/geqo_main.c glue code
geqo/geqo_misc.c debug printout code
geqo/geqo_mutation.c randomly mutate a tour (by swapping cities)
geqo/geqo_ox1.c unused method for generating a mutated tour
geqo/geqo_ox2.c unused method for generating a mutated tour
geqo/geqo_pmx.c unused method for generating a mutated tour
geqo/geqo_pool.c boring support code (manage “pool” of tours)
geqo/geqo_px.c unused method for generating a mutated tour
geqo/geqo_random.c boring support code
geqo/geqo_recombination.c boring support code
geqo/geqo_selection.c randomly select two “parent” tours from pool

## src/backend/optimizer/path

path/allpaths.c core scan/join search code (mostly about base rels)
path/clausesel.c clause selectivity (driver routines mostly)
path/costsize.c estimate path costs and relation sizes
path/equivclass.c support code for managing equivalence classes
path/indxpath.c core path generation for indexscan paths
path/joinpath.c core path generation for joins
path/joinrels.c core scan/join search code (mostly about join rels)
path/orindxpath.c path generation for “OR” indexscans
path/pathkeys.c support code for managing PathKey data structures
path/tidpath.c core path generation for TID-scan paths (WHERE ctid = constant)


## src/backend/optimizer/plan

plan/analyzejoins.c late-stage join preprocessing
plan/createplan.c build Plan tree from selected Path tree
plan/initsplan.c scan/join preprocessing (driven by planmain.c)
plan/planagg.c special hack for planning min/max aggregates
plan/planmain.c driver for scan/join planning
plan/planner.c driver for all “extra” query features
plan/setrefs.c Plan tree postprocessing
plan/subselect.c handle sub-selects (that aren't in FROM)

## src/backend/optimizer/prep

* prep/prepjointree.c: early-stage join preprocessing
* prep/prepqual.c: WHERE clause (qual) preprocessing
* prep/preptlist.c: target list preprocessing (mostly just for 
  INSERT/UPDATE/DELETE)
* prep/prepunion.c: plan set operations (UNION/INTERSECT/EXCEPT, but not simple
  UNION ALL); also has some support code for inheritance cases (“appendrels”)


## src/backend/optimizer/util

* util/clauses.c: assorted code for manipulating expression trees, includes
  constant folding and SQL function inlining
* util/joininfo.c support code for managing lists of join clauses
* util/pathnode.c: code for creating various sorts of Path nodes, and for 
  comparing the costs of different Path nodes (add_path() can be seen as the 
  very heart of the planner)
* util/placeholder.c: code for managing PlaceHolderVars
* util/plancat.c: code for extracting basic info about tables and indexes 
  from the system catalogs (sets up RelOptInfo and IndexOptInfo)
* util/predtest.c: code for proving that a WHERE clause implies or contradicts
  another one; used for constraint exclusion and seeing if partial indexes can 
  be used
* util/relnode.c: support code for managing RelOptInfo nodes
* util/restrictinfo.c: support code for managing RestrictInfo nodes
* util/tlist.c: support code for managing target lists
* util/var.c support code for managing Vars