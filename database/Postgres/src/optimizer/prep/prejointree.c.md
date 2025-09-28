1. `transform_MERGE_to_join(parse)`

   1. Create a new join RTE and add it to `parse->rtable`
   2. Create a `JoinExpr` and make it the sole element in `parse->jointree->fromlist`

   ```c
   // MERGE does a Join, this is the join range table entry that we will create
   RangeTblEntry *joinrte;
   JoinExpr   *joinexpr;
   bool		have_action[NUM_MERGE_MATCH_KINDS];
   JoinType	jointype;
   // Join RTE index, will be assigned to `joinexpr.rtindex`
   int			joinrti;
   List	   *vars;
   RangeTblRef *rtr;
   FromExpr   *target;
   Node	   *source;
   // RT index to the source relation, inited from `parse->jointree.from_list[0]`
   int			sourcerti;
   ```

   Postgres's [`MERGE`] command does a join operation first to see how the
   target table and source table are matched. The actual join type depends on
   the specified merge actions:

   * If we **only** have `WHEN MATCHED`: inner join
   * If we **only** have `WHEN NOT MATCHED BY SOURCE`: left outer join
   * If we **only** have `WHEN NOT MATCHED BY TARGET`: right outer join
   * If we have both `WHEN NOT MATCHED BY SOURCE/TARGET`: full outer join

    ```c
    /*
     * Work out what kind of join is required.  If there any WHEN NOT MATCHED
     * BY SOURCE/TARGET actions, an outer join is required so that we process
     * all unmatched tuples from the source and/or target relations.
     * Otherwise, we can use an inner join.
     */
    have_action[MERGE_WHEN_MATCHED] = false;
    have_action[MERGE_WHEN_NOT_MATCHED_BY_SOURCE] = false;
    have_action[MERGE_WHEN_NOT_MATCHED_BY_TARGET] = false;

    foreach_node(MergeAction, action, parse->mergeActionList)
    {
        if (action->commandType != CMD_NOTHING)
            have_action[action->matchKind] = true;
    }

    if (have_action[MERGE_WHEN_NOT_MATCHED_BY_SOURCE] &&
        have_action[MERGE_WHEN_NOT_MATCHED_BY_TARGET])
        jointype = JOIN_FULL;
    else if (have_action[MERGE_WHEN_NOT_MATCHED_BY_SOURCE])
        jointype = JOIN_LEFT;
    else if (have_action[MERGE_WHEN_NOT_MATCHED_BY_TARGET])
        jointype = JOIN_RIGHT;
    else
        jointype = JOIN_INNER;
    ```
    
    Initialize the RTE for the join operation. It is not a natural join, so
    `joinmergedcols` will be 0

    ```c
    /* Manufacture a join RTE to use. */
    joinrte = makeNode(RangeTblEntry);
    joinrte->rtekind = RTE_JOIN;
    joinrte->jointype = jointype;
    joinrte->joinmergedcols = 0;
    // QUES: I don't quite understand why this is set NIL
    joinrte->joinaliasvars = vars;
    joinrte->joinleftcols = NIL;	/* MERGE does not allow JOIN USING */
    joinrte->joinrightcols = NIL;	/* ditto */
    joinrte->join_using_alias = NULL;
    
    joinrte->alias = NULL;
    joinrte->eref = makeAlias("*MERGE*", NIL);
    joinrte->lateral = false;
    joinrte->inh = false;
    joinrte->inFromCl = true;
    ```
    
    Append the RTE_JOIN to `parse->rtable`. The RT index is `parse->rtable`'s
    length becuase RT index is 1-based.
    
    ```c
    /*
    * Add completed RTE to pstate's range table list, so that we know its
    * index.
    */
    parse->rtable = lappend(parse->rtable, joinrte);
    joinrti = list_length(parse->rtable);
    ```