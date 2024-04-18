> * 16.1 Overview
> * 16.2 Transformation of Relational Expressions
>   * 16.2.1 Equivalence Rules
>   * 16.2.2 Examples of Transformations
>   * 16.2.3 Join Ordering
>   * 16.2.4 Enumeration of Equivalent Expressions
> * 16.3 Estimating Statistics of Expression Results
> * 16.4 Choice of Evaluation Plans (physical plan)
> * 16.5 Materialized Views
>   * How to maintain (keep it update-to-date) it
>   * How to do queries on materialized views
> * 16.6 Advanced Topics in Query Optimization
> * 16.7 Summary

# 16.1 Overview

1. What is query optimization
  
   Query optimization is the process of **selecting** the most efficient 
   query-evaluation plan from among the many strategies usually possible
   for processing a given query.

   > Translate the given SQL to the least costly query plan.

2. There are generally 2 kinds of query optimization

   1. At the relational algebra level, for a given expression, find an equivalent
      expression that is less costly.

   2. Choose the best algorithm while generating physical plans.

# 16.2 Transformation of Relational Expressions

1. 2 relational algebra expressions are said to be equivalent if, on every legal
   database instance, the 2 expressions generate the same set of tuples.

## 16.2.1 Equivalence Rules

1. Null rejecting or null rejection

   A outer join query like 

   ```sql
   SELECT * FROM table1 LEFT OUTER JOIN table2 ON {JOIN CONDITION} WHERE {CONDITION};
   ```

   If `CONDITION` will be evaluated to `false` or unknown when the fields of
   `table2` are `NULL`, then the above query can be converted into an inner join:

   > For the extra tuples introduced by the left outer join, their values of 
   > `table2` will be `NULL`, since `CONDITION` will be evaluated to `false`
   > or unknown under this case, they will be removed from the result.

   ```sql
   SELECT * FROM table1 JOIN table2 ON {JOIN CONDITION} WHERE {CONDITION};
   ```

   Take a look at an example:

   Table customer:

   | name | order ID |
   |------|----------|
   |steve | 1        |
   |mike | 3        |

   Table orde:

   | price | order ID |
   |-------|----------|
   | 50    | 2        |
   | 30    | 3        |

   ```sql
   SELECT * FROM customer LEFT OUTER JOIN orde ON customer.order_id = orde.order_id;
   ```

   would return:

   | name | order id | price | order id |
   |------|----------|-------|----------|
   |steve | 1        | null  |   null   |
   |mike  | 3        | 30    |   3      |


   Now we add the condition `orde.order_id IS NOT NULL` on it:

   ```sql
   SELECT * FROM customer LEFT OUTER JOIN orde ON customer.order_id = orde.order_id
   WHERE orde.order_id IS NOT NULL;
   ```

   | name | order id | price | order id |
   |------|----------|-------|----------|
   |mike  | 3        | 30    |   3      |

   The result is same as the inner join:

   ```sql
   SELECT * FROM customer JOIN orde ON customer.order_id = orde.order_id
   WHERE order.order_id IS NOT NULL;
   ```

   > More info:
   >
   > * [DataFusion: convert outer join to inner join to improve performance](https://github.com/apache/arrow-datafusion/issues/1585)
   > * [NULL rejection in mysql](https://stackoverflow.com/q/16982845/14092446)


## 16.2.2 Examples of Transformations
## 16.2.3 Join Ordering
## 16.2.4 Enumeration of Equivalent Expressions

# 16.3 Estimating Statistics of Expression Results
# 16.4 Choice of Evaluation Plans (physical plan)
# 16.5 Materialized Views
# 16.6 Advanced Topics in Query Optimization
# 16.7 Summary
