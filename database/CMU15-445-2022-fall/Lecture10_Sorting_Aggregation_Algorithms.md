# Sort

1. If `ORDER BY` and `LIMIT` (`OFFSET`) exist at the same time, then this will 
   be a Top-N operation.

   One can implement it with:

   ```
   LimitExec - SortExec - TableScan
   ```

   Though a more common optimization is to use the Top-N heap sort, where the
   DBMS only needs to maintain an ordered heap of capacity of `offset + limit`.

   ```
   TopNExec - TableScan
   ```

   > For reference, here are how DuckDB and RisingLight implement it:
   >
   > * [DuckDB](https://github.com/duckdb/duckdb/pull/287)
   > * [RisingLight](https://github.com/risinglightdb/risinglight/blob/b391b48584fffdafbc36aabb7d8b569be690d0a1/src/executor/top_n.rs)

# Aggregation
