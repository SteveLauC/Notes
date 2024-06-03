> Today's agenda:
>
> * Top-N Heap Sort
> * External Merge Sort
> * Aggregations

# Top-N heap sort

1. If `ORDER BY` and `LIMIT` (`OFFSET`) exist at the same time, and the data 
   (offset + limit rows) **can fit in memory**, then this will be a Top-N 
   operation.

   Naively, one can implement it with:

   ```
   LimitExec <- SortExec <- TableScan
   ```

   Though a more common optimization is to use the Top-N heap sort, where the
   DBMS only needs to maintain an ordered heap of capacity of `offset + limit`.

   > std::collections::BinaryHeap

   ```
   TopNExec <- TableScan
   ```

   > For reference, here are how DuckDB and RisingLight implement it:
   >
   > * [DuckDB](https://github.com/duckdb/duckdb/pull/287)
   > * [RisingLight](https://github.com/risinglightdb/risinglight/blob/b391b48584fffdafbc36aabb7d8b569be690d0a1/src/executor/top_n.rs)

# External Merge sort

1. Merge sort is a divide-and-conquer sorting algorithm.

2. What is in a sorted run

   A sorted run is an ordered list of key-value pairs:

   * key: value of the columns to sort on
   * value:
     
     We have 2 choices here:

     1. tuple data

        This is called early materialization

        * Pros: We don't need to load the data from the relation file again after the sort
        * Cons: The runs are bigger and thus we have more I/O and memory cost
        
     2. record ID

        This is called late materialization

        * Pros: uses less memory and emits less I/O
        * Cons: need to load the data from the disk after sorting

3. Memory available for sorting:

   * In PostgreSQL, it is called `work_mem`, the default value is 4MiB.
   * In MySQL, it is `sort_buffer_size`.

4. Double buffer optimization

   In the sorting stage, while sorting one run, we can prefetch the next run 
   into another buffer.

   > I mean, if buffer is totally sufficient, why not just do it in multiple
   > threads?

5. Optimization for comparison

   1. Code specialization

      Within the sort implementation, make the comparison procedure part of 
      the sorting code rather than a function pointer so that we don't need to
      do one indirection.
      
      > This optimization works well if comparison happens in memory
   
    2. 

# Aggregation

1. Aggregations can be implemented in 2 ways:

   1. Sorting
      
      1. Sort the relation by `GROUP BY` columns
      2. Sequentially scan the ordered relation to calculate the aggregation
      
   2. Hashing 
      
      For relations that fit in memory:
      1. Partition the relation by `GROUP BY` columns
      2. For each partition, do the aggregation

      If not, then we need to do an extra partition step before the start, just
      like the grace hash join.

2. Hashing is almost always better than sorting.
