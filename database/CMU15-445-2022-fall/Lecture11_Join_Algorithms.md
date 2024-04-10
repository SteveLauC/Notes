1. Why do we need join

   We **normalize** tables in a relational database to **avoid unnecessary duplicate**
   information.

   > Normalization is a branch of relational theory that provides design 
   > insights. It is the process of determining how much redundancy exists in a table.

   We then use the join operator to reconstruct the original tuples without any
   information loss.

2. This lecture ONLY covers 

   * inner 
   * equi

   join.

   > For how to implement outer joins, see [15.6.4 Outer Join][link].
   >
   > [link]: https://github.com/SteveLauC/Notes/blob/main/database/Database_System_Concepts/Ch15_Query_Processing.md#1564-outer-join

3. Operator Output

   For a tuple from one table and a tuple from another table that satisfy the 
   join condition, the join operator concatenates the contents of these 2 tuples
   into a new output tuple.

   > If this is a natural join, then duplicate columns will be removed.

   In reality, we can concatenate them (i.e., we output data, this is called early 
   materialization) or we can simply use the Record IDs of these 2 tuples (late
   materialization).


4. What are materialization and late materialization

   > FYI, materialization is called "物化" in chinese, and late materialization
   > is "延迟物化".

   Meaterialization refers to the process of converting the data from a format
   to the data format that can be consumed by the upper execution node/operator.

   This concept is specifical for column-based stores, e.g., for the below SQL:

   ```sql
   SELECT * FROM table;
   ```

   Assume `table` is stored in a column-based format, reading all the columns
   and concatenate them into rows is called materialization.

   Then what is late materialization, read columns lazily:

   ```sql
   SELECT a FROM table WHERE b IS NOT NULL;
   ```

   A simple implementation would read columns `a` and `b`, then filter the rows
   based on the values of column `b`. A more efficient one, will read column `b`
   first, then do the filter, then we fetch the corresponding column `a` values.

5. Cost analysis

   We only analyze the # of block transfer in this class, the # of seeks (random
   I/O) won't be counted.


6. Join algorithms

   1. Nested loop join

      > QUES: The note says that we should use the smaller table as the outer 
      > table, I don't understand this, for simple nested loop joins, I think 
      > the inner table should be the smaller one as it will be read multiple 
      > times.
      >
      > Future steve: I think this is not a deterministic problem, why not just
      > calculate the costs during the optimization stage.
      >
      > Join reorder

      1. Simple nested loop join

         The note says that cost is: `M +(m×N)`, which assumes that there is
         only 1 buffer available for the inner table, i.e., the cost under
         the worst case.

      2. Block nested loop join
          
         In the worse case, i.e., 1 buffer for each relation, the # of block 
         transfers will be `M + M*N`.

         If we follow the idea of block nested loop join, i.e., 1 tuple per inner
         relation scan -> 1 block per innner relation scan, then we can do more
         blocks at a time we have enough buffers.

         Say we scan the inner relation per `B` blocks, 1 buffer for the inner 
         relation, then the # of block transfers will be `M + (M/B)*N`.

      3. Index nested loop join

   2. Sort merge join

      1. This algorithm will be useful (chosen by the planner) if:

         1. One or both relations are already sorted
         2. The result of the join needs to be sorted
            
            ```sql
            SELECT students.name, students.score 
            FROM students 
                JOIN takes 
                ON students.score = takes.score
            ORDER BY students.score;
            ```

      2. The worse scenario for this algorithm is that all the tuples have the 
         same value on their join attributes.

         > QUES: why

         > For the in-memory sorting algorithms, insertion sort is the fastest
         > one if the input data is ordered.

      3. Cost

         1. Sort
             
            Assume we have `B` buffers for merge join, sorting the outer relation
            needs `{1+[log(B-1)(M/B)]} * 2M`. Similarly, `{1+[log(B-1)(N/B)]} * 2N`
            for the inner relation.

         2. Merge 
            
            Only 1 scan for each relation is required, so `M+M`.

         
         3. Cost: sort + merge

   3. Hash join
      
      1. Basic hash join
        
         1. The in-memory hash index should be a hash table, whose key is the 
            value of the join attributes, the value can be Record ID or the 
            real tuple value.

            > We always need to store join attributes since we want to give it
            > a double check in case of hash collisions.

            Which hash table implementation to use? Slides say that linear probing
            hashing works best.
         
         2. If the DBMS knows the size of the table on which the hash index is
            going to be built, then it can use a static hash table, or it has
            to use a dynamic hash table.

         3. One can use bloom filter to optimize the probe phase

            > QUES: how?

            Future steve: Bloom filter is a probabilistic data structure that can
            answer the question: if an item is in the set with:

            1. The target item is guaranteed to not exist in the set
            2. The item is probably in the set

            Bloom filter is compact and can be well fit in the CPU cache, we can
            build a bloom filter during the build phase, and check the bloom filter
            before probing the hash table.

            Such an optimization is sometimes called `Sideways information passing`.

            > Sideways information passing origins from Vertica (a columar analytics
            > database).

         4. Yet another Sideways information passing. During the build stage, you 
            actully know the statistics of the build relation, then you can use 
            this statistics to prune the probe relation while accessing it 
            (if supported, e.g., parquet).
         
      2. Grace/partitioned hash join

         > It is called Grace Hash Join, named after the [Grace database machine][link]
         >
         > [link]: https://museum.ipsj.or.jp/en/computer/other/0014.html

         The algorithm introduced in this class do not build an in-memory hash
         index for each bucket, it instead uses a nested loop join.

         The # of block transfers in grace/partitioned hash join is that:

         1. Partition: 2 * (M+N)
         1. Build and Probe/nested loop join: M+N

7. Cost conparison between different join algorithms

   | algorithm              |  I/O Cost                 | The # of block transfer  | Example    |
   |------------------------|---------------------------|--------------------------|------------|
   |simple nested loop join | M + (m · N ) (worst case) | 50_001_000               | 1.4 hours  |
   | block nested loop join | M + (M · N ) (worst case) | 501_000                  | 50 secs    |
   | index nested loop join | M + (m · C )              | ?                        | ?          |
   | merge join             | M + N + sort cost         | 7_500                    | 0.75 secs  |
   | grace hash join        | 3 * (M + N)               | 4500                     | 0.45 secs  |

   The estimated time assumes that:

   M = 1000, m = 10_0000, N = 500, n = 4_0000, B = 100, and 0.1 milisecond for I/O.

   Hash joins are almost always better than sort-based join algorithms, but 
   there are cases in which sorting- based joins would be preferred. This 
   includes queries on non-uniform data, when the data is already sorted on 
   the join key, and when the result needs to be sorted.

