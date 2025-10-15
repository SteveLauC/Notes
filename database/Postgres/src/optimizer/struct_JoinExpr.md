Why does Postgres need a dedicated type for join? I think this type can be merged
into `RangeTblEntry` as `RTE_JOIN` fields, then the `jointree` node could be a simple
`RangeTblRef`.