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

# Post scan/join planning

1. GROUP BY, aggregation and other high-level features are considered upper,
   Postgres has to create a `RelOptInfo` as a container for the `pathlist`,
   this kind of `RelOptInfo` is of type `RELOPT_UPPER_REL`, these `RelOptInfo`
   are dummy, `pathlist` is probably the only field that will be useful?

   > Hmm, this looks like a compromise :D

   Currently, we have these types of upper `RelOptInfo` during upper-level
   planning:

   ```c
   /*
    * This enum identifies the different types of "upper" (post-scan/join)
    * relations that we might deal with during planning.
    */
   typedef enum UpperRelationKind
   {
      /* result of UNION/INTERSECT/EXCEPT, if any */
      UPPERREL_SETOP,
      /* result of partial grouping/aggregation, if any */
      UPPERREL_PARTIAL_GROUP_AGG,
      /* result of grouping/aggregation, if any */
      UPPERREL_GROUP_AGG,
      /* result of window functions, if any */
      UPPERREL_WINDOW,
      /* result of partial "SELECT DISTINCT", if any */
      UPPERREL_PARTIAL_DISTINCT,
      /* result of "SELECT DISTINCT", if any */
      UPPERREL_DISTINCT,
      /* result of ORDER BY, if any */
      UPPERREL_ORDERED,
      /* 
       * Result of any remaining top-level actions any final processing steps, currently:
       *
       * 1. LockRows (SELECT FOR UPDATE)
       * 2. LIMIT/OFFSET 
       * 3. ModifyTable.
       *
       * All these 3 operations share the same node UPPERREL_FINAL rather than having their 
       * own UPPERREL_XXX nodes because the order of executing them is fixed, there is no
       * flexibility:
       *
       * - LIMIT <- LockRows
       * - ModifyTable
       */
      UPPERREL_FINAL,
   } UpperRelationKind;
   ```