> This chapter covers:
>
> 1. Equivalent rules in relational algebra (16.2)
>
>    > Review Relation Algebra (Ch2) if you cannot recall them clearly.
>
> 2. For a generated unoptimized logical plan, we know how to convert it to 
>    an equivalent one using equivalent rules. Then we need to choose the least
>    costly one by estimnating their cost (16.3)
>
> 3. How to generate the physical plan (16.4)
> 4. Introduce materialized views, which can be used to speed up certain queries 
>    (16.5)

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

   > The order of tuples does not matter since a relation is a set, and set does
   > not have order.

## 16.2.1 Equivalence Rules

1. What is an equivalence rule

   An equivalence rule says that expressions of two forms are equivalent.

   The query optimizer uses equivalence rules to transform expressions into other
   logically equivalent expression.

2. Rules

   For the following rules, we use $ \theta, \theta_{1}, \theta_{2} $ and so on
   to denote predicates. $ L_{1}, L_{2}, L_{3} $ and so on to denote lists of
   attributes, and $ E, E_{1}, E_{2} $ and so on to denote relational algebra
   expressions.

   1. Conjunctive selection operations can be deconstructed into a sequence of
      individual selections, this transformation is referred to as a cascade
      of $ \sigma $.

      $$ \sigma_{\theta_{1} \wedge \theta_{2} } (E) = \sigma_{\theta_{1}} (\sigma_{\theta_{2}} (E)) $$

   2. Selection operations are **commutative**.

      $$ \sigma_{\theta_{1}} (\sigma_{\theta_{2}}) = \sigma_{\theta_{2}} (\sigma_{\theta_{1}}) $$

   3. 

2. Null rejecting or null rejection

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
