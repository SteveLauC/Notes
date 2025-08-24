src/include/parser/analyze.h
src/backend/parser/analyze.c


1. What will be done in the analyzer stage

   * Expand * in the target list
   
     ```c
     /*
      * transformTargetList()
      * Turns a list of ResTarget's into a list of TargetEntry's.
      *
      * This code acts mostly the same for SELECT, UPDATE, or RETURNING lists;
      * the main thing is to transform the given expressions (the "val" fields).
      * The exprKind parameter distinguishes these cases when necessary.
     */
     List *
     transformTargetList(ParseState *pstate, List *targetlist,
    					ParseExprKind exprKind)
     ```
    
   * Resolve table names to the internal identifier
   
     ```c
     /*
      * Add an entry for a relation to the pstate's range table (p_rtable).
      * Then, construct and return a ParseNamespaceItem for the new RTE.
      *
      * We do not link the ParseNamespaceItem into the pstate here; it's the
      * caller's job to do that in the appropriate way.
      *
      * Note: formerly this checked for refname conflicts, but that's wrong.
      * Caller is responsible for checking for conflicts in the appropriate scope.
      */
     ParseNamespaceItem *
     addRangeTableEntry(ParseState *pstate,
				   RangeVar *relation,
				   Alias *alias,
				   bool inh,
				   bool inFromCl)
     ```
     
     Which opens the relation and load its metadata/descriptor (RelationData)
     to the relcache.
     
   * 