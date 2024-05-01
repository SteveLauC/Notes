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

      $$ \sigma_{\theta_{1} \wedge \theta_{2} } (E) \equiv \sigma_{\theta_{1}} (\sigma_{\theta_{2}} (E)) $$

   2. Selection operations are **commutative**.

      $$ \sigma_{\theta_{1}} (\sigma_{\theta_{2}}) \equiv \sigma_{\theta_{2}} (\sigma_{\theta_{1}}) $$

   3. Only the final operations in a sequence of projectionare needed, the others
      can be omitted, this transformation can also be referred to as a cascade of
      $ \Pi $ 

      $$ \Pi_{L1} (\Pi_{L2} (.. (\Pi_{Ln} (E)))) \equiv \Pi_{L1} (E) $$

      where $ L1 \subseteq L2 \subseteq ...  \subseteq Ln $

   4. Selections can be combined with Cartesian products and theta join

      1. $ \sigma_{\theta} (E1 \times E2) \equiv E1 \Join_{\theta} E2 $

         > This is exactly the definition of Theta join.

      2. $ \sigma_{\theta_{1}} (E1 \Join_{\theta_{2}} E2) \equiv E1 \Join_{\theta_{1} \wedge \theta_{2}} E2 $

         > QUES: can this be applied to outer join?
         >
         > I guess yes.

   5. Theta join operations are commutative.

      > Natural join is a special case of theta join, so it is commutative as well.

      $$ E1 \Join_{\theta} E2 \equiv E2 \Join_{\theta} E1 $$

      > QUES: is this appliable to outer join?
      >
      > I think no, outer join is not commutative.
      >
      > But I believe there are cases where outer join is also commutative.

      > NOTE: this rule holds if we don't care about the order of the attributes.

      > We can choose which table will be used as the outer table while join 2 
      > tables for better performance because of this rule.
      > 
      > See Ch15 for more information.

      > Is cartesian product commutative?
      >
      > No if the order of pairs matter, but yes if it does not matter.

   6. Associative property of join

      > Is cartesian product associative?
      >
      > In set theory, no, assume a = {1}, b = {2}, c = {3}, then
      >
      > $ (a \times b) \times c  = $ {(1, 2), 3} 
      >
      > But 
      >
      > $ a \times (b \times c)  = $ {1, (2, 3)} 
      >
      > {(1, 2), 3} and {1, (2, 3)} are not the same thing.
      >
      > But I think in the relational algebra or SQL world, these 2 sets are the
      > same set, so the answer would be yes. 

      1. Natural join is associative:

         $$ (E1 \Join E2) \Join E3 \equiv E1 \Join (E2 \Join E3) $$

         > QUES: is this appliable to outer join?
         > 
         > A general anawer is no, outer join is not associative, but there
         > should be cases where the associative property is satisfied.

      2. Theta join is associative in the following manner:

         > QUES: I do not quite understand this.

         $$ (E1 \Join_{\theta_{1}} E2) \Join_{\theta_{2} \wedge \theta_{3}} E3 \equiv E1 \Join_{ \theta_{1} \wedge \theta_{3}} (E2 \Join_{\theta_{2}} E3) $$

         where $ \theta_{2} $ only involves attributes from $ E2 $ and $ E3 $

         > Commutative property and associative property are important for join 
         > reordering in query optimization.

   7. Selection distributes over theta-join (inner join)

      > Predicate pushdown in inner join

      > QUES: does this apply to outer join?
      >
      > I highly doubt no.

      1. If all the attributes in $ \theta_{1} $ only involve the attributes of
         one of the expressions, say $ E1 $ in this example, then the following
         equivalence rule holds:

         $$ \sigma_{ \theta_{1}} (E1 \Join_{\theta} E2) \equiv (\sigma_{\theta_{1}} (E1)) \Join_{\theta} E2 $$

      2. Similar to the last rule, if the condition is $ \theta_{1} \wedge \theta_{2} $, 
         and $ \theta_{1} $ only involves the attributes of $ E1 $, $ \theta_{2} $ only 
         involves the attributes of $ E2 $, then:

         $$ \sigma_{\theta_{1} \wedge \theta_{2}} (E1 \Join_{\theta} E2) \equiv (\sigma_{\theta_{1}} E1) \Join_{\theta} (\sigma_{\theta_{2}} E2)) $$

   8. The projection operation distributes over the theta-join operation under the
      following conditions.

      > Projection pushdown in theta join

      > NOTE: This applies to left/right/full outer join as well.

      1. Let $ L1 $ and $ L2 $ be attributes of $ E1 $ and $ E2$, respectively.
         Suppose that the join condition $ \theta $ involves only attributes
         in $ L1 \cup L2 $ (to make the join operation doable after the projection
         pushdown), then:

         $$ \Pi_{ L1 \cup L2 } (E1 \Join_{\theta} E2) \equiv (\Pi_{L1} (E1)) \Join_{\theta} (\Pi_{L2} (E2)) $$

      2. Consider a join $ E1 \Join_{\theta} E2 $, let $ L1 $ and $ L2 $ be sets
         of attributes from $ E1 $ and $ E2 $, respectively. Let $ L3 $ be attributes
         of $ E1 $ that are involved in the join operation $ \theta $ but not in
         $ L1 $, $ L4 $ are attributes that are involved in $ \theta $ as well but
         not in $ L2 $, then:

         $$ \Pi_{ L1 \cup L2 } (E2 \Join_{\theta} E2) \equiv \Pi_{L1 \cup L2} ((\Pi_{L1 \cup L3} (E1)) \Join_{\theta} (\Pi_{L2 \cup L4} (E2)))$$

         We only want attributes $ L1 \cup L2 $, but since the join condition 
         $ \theta $ needs $ L3 $ and $ L4 $ as well, we also need to push them
         down to do the join operation.

   9. Set operations like Union an Intersection are commutative.

      $$ E1 \cup E2 \equiv E2 \cup E1 $$
      $$ E1 \cap E2 \equiv E2 \cap E1 $$

      > Set difference is not commutative.
      >
      > Example, $ a = \{1\}, b = \emptyset $, $ a \setminus b = \{1\} $ , $ b \setminus a = \emptyset $.

   10. Set operations like Union and Intersection are associative.


       $$ (E1 \cup E2) \cup E3 \equiv E1 \cup (E2 \cup E3) $$
       $$ (E1 \cap E2) \cap E3 \equiv E1 \cap (E2 \cap E3) $$

       > set difference is not associative.
       >
       > > If you are interested in proof, check [this][proof] out.
       >
       > [proof]: https://proofwiki.org/wiki/Set_Difference_is_not_Associative
       >
       > Example, $ a = b = c \neq \emptyset $
       >
       > $$ (a \setminus b) \setminus c = \emptyset \setminus c = \emptyset $$
       > $$ a \setminus (b \setminus c) = a \setminus \emptyset = a $$

   11. The selection operation distributes over set operations:

       $$ \sigma_{\theta} ( E1 \cup E2) \equiv (\sigma_{\theta} E1) \cup (\sigma_{\theta} E2) $$
       $$ \sigma_{\theta} ( E1 \cap E2) \equiv (\sigma_{\theta} E1) \cap (\sigma_{\theta} E2) $$
       $$ \sigma_{\theta} ( E1 \setminus E2) \equiv (\sigma_{\theta} E1) \setminus (\sigma_{\theta} E2) $$

       > This should be something like predicate pushdown for sub-queries.

       And if we push the predicate down to only 1 side, then:

       $$ \sigma_{\theta} ( E1 \cap E2) \equiv (\sigma_{\theta} E1) \cap (E2) $$
       $$ \sigma_{\theta} ( E1 \cap E2) \equiv (E1) \cap (\sigma_{\theta} E2) $$
       $$ \sigma_{\theta} ( E1 \setminus E2) \equiv (\sigma_{\theta} E1) \setminus (E2) $$









      

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
