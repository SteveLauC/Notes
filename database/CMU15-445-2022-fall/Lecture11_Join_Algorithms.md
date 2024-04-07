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

