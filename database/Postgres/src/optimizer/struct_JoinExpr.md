Why does Postgres need a dedicated type for join? I think this type can be merged
into `RangeTblEntry` as `RTE_JOIN` fields, then the `jointree` node could be a simple
`RangeTblRef`.

Future steve: because `rtable` is a flat list? it cannot express nested strucutre?
Well, looks like it can handle that:

```text
[table1, table2, table3, join(larg: 1, rarg: 2), join(3, 4)]
```

Perhaps the philosophy here is that `rtable` contains base or descriptor, then
`jointree` contains the **dynamic** expressions.

Alright, I just read the doc, `RangeTblEntry` stores the join result:

> A range table entry may represent a plain relation, a sub-select in FROM, or 
> the **result** of a JOIN clause.

They split result and expression on purpose. 

QUES: Could the result of a join be referenced in `rtable`, like the `join(.., 4)` case?