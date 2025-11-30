This type represents a variable that references something, in most cases, it 
references a column from a table.

In the parser and planner, `Var` refers a column:

* from a table
* from a join result (`RangeTblEntry.joinaliasvars` of an `RTE_JOIN` rte)

At the end of planning, `Var` refers to a column from <TODO>


# Fields

* varno: RT index
* varattno: attribute number, `InvalidAttrNumber` if it references the whole 