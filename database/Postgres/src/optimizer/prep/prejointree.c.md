1. transform_MERGE_to_join()

   Postgres's [`MERGE`] command will does a join operation first to see how the
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
