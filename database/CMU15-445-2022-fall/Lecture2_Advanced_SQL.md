0. How to pronounce `SQL`, remember its orig name `SEQUEL`.

1. SQL can be classified into 3 classes:

   1. DDL
   2. DML
   3. DCL(data control language): for security and access mode

2. `count(1)` is equivalent to `count(*)`

   > Aggregate functions can only be used in a `SELECT` output list

3. string
	
   SQL standard says string is single-quotes only. But MySQL also support double
   quotes. And the standard also demands that string is case-sensitive, but by
   default, MySQL is not.

   ![screenshot](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2018-50-41.png)

   ![scree](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2018-57-47.png)
4. insert into a table with results from another query(output redirection)

   * To a existing table:
     ```SQL
     insert into TABLE_TO_INSERT(....)
     select ...
     ```

   * To a new table:

     > The target table must have the same number of columns with the same 
     > types as the target table, but the names of the columns in the output 
     > query do not have to match.

     ```sql
     select ...
     into NEW_TABLE
     from ...
     ```

     > This syntax is supported in PostgreSQL but not supported in MySQL.

5. output control

   1. Since SQL is based on bag, which is unordered, there has to be a way to 
      reformat the output: `ORDER BY`.
    
      > `order by 1` is equivalent to `order by [column 1 of SELECT list]`

   2. By default, the DBMS will return all of the tuples produced by the query.
      We can use the `LIMIT` clause to restrict the number of result tuples.

6. nested query

   ![sc](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2019-21-12.png)

   1. The scope of outer query is included in an inner query (i.e. the inner 
      query can access attributes from outer query), but not the other way 
      around.

5. Common Table expression

   ```SQL
   WITH tmp_table_name as (
   	   select * from table
   )
   ```

   You can think it as a temporary table scoped to a single query

6. SQL is based on bags whereas relational algebra is based on sets

   * bag: an unordered collection of elements **with** duplicates
   * set: an unordered collection of elements without duplicates

