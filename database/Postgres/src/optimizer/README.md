# Optimizer

1. /geqo is the separate "genetic optimization" planner, it does a semi-random 
   search through the join tree space, rather than exhaustively considering all 
   possible join trees.  
   
   (But each join considered by /geqo is given to /path to create paths for, so 
   we consider all possible implementation paths for each specific join pair 
   even in GEQO mode.)

   QUES: What does "so we consider all possible implementation paths for each 
   specific join pair even in GEQO mode" mean? Gemini says it means that GEQO
   only decides the join ordering, and all the possible join method will be enumerated.

   > GEQO is for join

## Paths and Join Pairs

1. Path vs Plan

   There is a nearly one-to-one correspondence between Path and Plan.  But Path
   is for the planner, and Plan is for the executor.  So Path has information 
   that would be needed by the planner, which plan does not have. Similarly, Plan
   has info that is needed by the executor, but is pointless to the planner.

2. Term: What is a base relation

   A base rel can be:

   * A primitive table
   * A subquery appeared in the range table
     * sub-SELECT
     * a function call
   
   It is uniquely identified by an RT index.

3. Term: What is a join relation

   A join rel is a set of base rels, regardless of the order of how they are 
   joined.

4. Postgres builds a `struct RelOptInfo` for each base/join relation (fn: 
   `build_simple_rel()` in `src/backend/optimizer/util/relnode.c`), which contains
   all the feasible `Path`s for this `scan/join` operator in `RelOptInfo.pathlist`.

5. Pair-wise joins use 2 `struct RelOptInfo`, the left one is outer, the right one
   is inner.

## Join Tree Construction (Paths enumerations)

> How PG plans Join, with more details


1. For each base relation, create a `struct RelOptInfo` and put all the feasible
   Paths into `RelOptInfo.pathlist`. 

   All the predicates (`WHERE/ON`) involving this relation will be put in 
   `RelOptInfo.joininfo`

   check if we have only 1 base relation, if so, we are done.  Otherwise, figure 
   out how to join these relations.

2. Folded join clauses

   The join clause may not be flattened