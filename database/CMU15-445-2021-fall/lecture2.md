1. SQL can be classified into 3 classes:

   1. DDL
   2. DML
   3. DCL(data control language): for security and access mode

2. `count(1)` is equivalent to `count(*)`

   > Aggregate functions can only be used in a `SELECT` output list

3. string
	
   SQL standard sayes string is single-quotes only. But MySQL also support double
   quotes. And the standard also demand that string is case-sensitive, but by
   default, MySQL is not.

   ![screenshot](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2018-50-41.png)

   ![scree](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2018-57-47.png)
4. insert into a table with results from another query(output redirection)

   ```SQL
   insert into table(....)
   select ...
   ```

   SQL standard also specifies another grammer to do this
   ```sql
   select ...
   into table
   from ...
   ```

   But I just tried finding MySQL does not support this grammer

   ![sc](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2019-08-33.png)

5. output control

       > reformat the output
    
    `order by 1` is equivalent to `order by [column 1 of SELECT list]`

6. nested query

   ![sc](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-13%2019-21-12.png)


5. Common Table experssion

   ```
   WITH tmp_table_name as (
   	select * from table
   )
   ```

   You can think it as a temporary table scoped to a single query

6. SQL is based on bags whereas relational algebra is based on sets

   * bag: an unorderd collection of elements **with** duplicates
   * set: an unorderd collection of elements without duplicates

