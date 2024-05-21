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
>   * 16.3.1 Catalog information
>   * 16.3.2 Selection (relational algebra) Size Estimation
>   * 16.3.3 Join Size Estimation
>   * 16.3.4 Size Estimation for Other Operations
>   * 16.3.5 Estimation of Number of Distinct Values 
> * 16.4 Choice of Evaluation Plans (physical plan)
>   * 16.4.1 Cost-Based Join-Order Selection
>   * 16.4.2 Cost-Based Optimization with Equivalence Rules
>   * 16.4.3 Heuristics in Optimization
>   * 16.4.4 Optimizing Nested Subqueries
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

3. Query optimizer

   There are 2 categories of query optimizer:

   1. Rule based optimizer
     
      A rule based optimizer will convert a given query into a set of equivalent
      queries through a set equivalence rules, you will see this in section 16.2.1

   2. Cost based optimizer

      After converting the given query to its equivalen alternatives, we need to 
      estimate the cost of those alternatives and choose the one that is least
      costly, such an optimizer is cost-based.

      As you can see, a cost-based optimizer **relies on** rule-based optimizer.

   A database system can only have RBO, i.e., after applying the equivalence rule,
   the new query is guaranteed to be more efficient than the original one, e.g.,
   predicate and projection pushdown.

   DataFusion, a stand-alone query engine, does not have CBO because that would 
   require statistics information. Though they are indeed working towards better
   statistics support: https://github.com/apache/datafusion/issues/8229.


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

      $$ \sigma_{\theta_{1}} (\sigma_{\theta_{2}} E) \equiv \sigma_{\theta_{2}} (\sigma_{\theta_{1} } E) $$

   3. Only the final operations in a sequence of projection operation needed, the 
      others can be omitted, this transformation can also be referred to as a 
      cascade of $ \Pi $ 

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
      >
      > Future steve: full outer join is commutative, see rule 14.

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
      > In set theory, no, assume $ a = \{1\}, b = \{2\}, c = \{3\} $, then
      >
      > $ (a \times b) \times c  = \{(1, 2), 3\} $
      >
      > But 
      >
      > $ a \times (b \times c)  = \{1, (2, 3)\} $
      >
      > $ \{(1, 2), 3\} $ and $ \{1, (2, 3)\} $ are not the same thing.
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
      > See rule 15.

      1. If all the attributes in $ \theta_{1} $ only involve the attributes of
         one of the expressions, say $ E1 $ in this example, then the following
         equivalence rule holds:

         $$ \sigma_{ \theta_{1}} (E1 \Join_{\theta} E2) \equiv (\sigma_{\theta_{1}} (E1)) \Join_{\theta} E2 $$

      2. Similar to the last rule, if the condition is $ \theta_{1} \wedge \theta_{2} $, 
         and $ \theta_{1} $ only involves the attributes of $ E1 $, $ \theta_{2} $ only 
         involves the attributes of $ E2 $, then:

         $$ \sigma_{\theta_{1} \wedge \theta_{2}} (E1 \Join_{\theta} E2) \equiv (\sigma_{\theta_{1}} E1) \Join_{\theta} (\sigma_{\theta_{2}} E2)) $$

         > This equivalence can be derived from rule 1 and 7.1

   8. The projection operation distributes over the theta-join operation under the
      following conditions.

      > Projection pushdown in theta join

      > NOTE: This **applies** to left/right/full outer join as well.

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

       > For set operations like union/intersection/set-difference, the operatee
       > should have have the same schema, so if $ \theta $ involves attributes 
       > of $ E1 $, the it also involves attributes from $ E2 $.
       >
       > This is the reason why you don't see things like
       >
       > $$ \sigma_{\theta} (E1 \cup E2 ) \equiv (\sigma_{\theta} (E1)) \cup E2 $$
       >
       > if $ \theta $ involves only the attributes from $ E1 $, which is impossible.

       > This should be something like predicate pushdown for sub-queries.

       And if we push the predicate down to only 1 side, then:

       $$ \sigma_{\theta} ( E1 \cap E2) \equiv (\sigma_{\theta} E1) \cap (E2) \equiv  (E1) \cap (\sigma_{\theta} E2)  $$
       $$ \sigma_{\theta} ( E1 \setminus E2) \equiv (\sigma_{\theta} E1) \setminus (E2) $$

   12. The projection operation distributes over the union operation

       $$ \Pi_{L} (E1 \cup E2) \equiv (\Pi_{L} E1) \cup (\Pi_{L} E2) $$

       This does not apply to intersection, example

       Assume $ E1 $ is:

       | name | age | 
       |------|-----|
       | steve| 1   |
       
       Assume $ E2 $ is:

       | name | age | 
       |------|-----|
       | steve| 2   |

       then $ \Pi_{name} (E1 \cap E2) = \emptyset $ 

       But $ (\Pi_{name} E1) \cap (\Pi_{name} E2) = \{ steve \} $

       The same example applies to set-difference as well:

       $ \Pi_{name} (E1 \setminus E2) = \{ steve \}$, but $ (\Pi_{name} E1) \setminus (\Pi_{name} E2) = \emptyset $.

   13. Selection distrbutes over aggregation under the following conditions, let
       $ G $ be a set of group by attributes, and $ A $ a set of aggregate 
       expressions. When $ \theta $ only involves attributes in $ G $, the following
       equivalence holds:

       $$ \sigma_{\theta} (G gA(E) ) \equiv GgA (\sigma_{\theta}E) $$

   14. Commutative property of outer join

       1. Full outer join is commutative
       2. Left and right outer join are not commutative.

   15. Selection distributes over **left/right** outer join is the condition $\theta$
       only involves the attributes from one side, and that side is the outer one

       $$ \sigma_{\theta} (E1 left-outer-join E2 ) \equiv (\sigma_{\theta} E1) left-outer-join E2 $$
       $$ \sigma_{\theta} (E2 left-outer-join E1 ) \equiv (\sigma_{\theta} E2) left-outer-join E1 $$

       > What about full outer join?
       >
       > I think predicate pushdown cannot be done with full outer join.


   16. Null rejecting or null rejection

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

1. A set of equivalence rules is said to be **minimal** if no rule can be derived
   from any combination of others.

   Since the rule 7.2 can be derived from the rule 1 and 7.1, the rule set introduced 
   in section 16.2.1 is NOT minimal.

   The number of difference ways of generating an expression increases when we use
   a nonminimal set of equivalence rules, so query optimizer should use minimal
   sets of equivalence rules.

## 16.2.3 Join Ordering

1. What is join ordering and why we need to optimize it

   Join is a binary operation, which means you have to join 2 tables, then another
   2 tables. In real world scenarios, it is common to join over multiple tables,

   ```sql
   SELECT * FROM a JOIN b JOIN c;
   ```

   The above query wants to join over 3 tables a, b and c, but we cannot join over
   3 tables, only 2 tables are allowed in the join operation, and since natural 
   inner join is commutative and associative, so we can do

   > Why is `a JOIN b JOIN c` equivalent to
   >
   > `(a JOIN b) JOIN c`
   > `a JOIN (b JOIN c)`
   >
   > see [`Why is 1+2+3 = (1+2)+3`][link]
   >
   > [link]: https://math.stackexchange.com/q/4901826/1313487

   ```sql
   SELECT * FROM (a JOIN b) JOIN c;
   SELECT * FROM a JOIN (b JOIN c);
   SELECT * FROM (a JOIN c) JOIN b;
   SELECT * FROM a JOIN (c JOIN b);
   SELECT * FROM (b JOIN a) JOIN c;
   SELECT * FROM b JOIN (a JOIN c);
   SELECT * FROM (b JOIN c) JOIN a;
   SELECT * FROM b JOIN (c JOIN a);
   SELECT * FROM (c JOIN a) JOIN b;
   SELECT * FROM c JOIN (a JOIN b);
   SELECT * FROM (c JOIN b) JOIN a;
   SELECT * FROM c JOIN (b JOIN a);
   ```

   The above 12 SQLs are equivalent, but they can be different when it comes 
   to execution efficiency. We want to choose the on with the most minimal cost.

   > For 3 tables, there are 12 ways to do the join operation, for 4 tables,
   > there are 120 ways, for 5 tables, there are 1680 ways.


## 16.2.4 Enumeration of Equivalent Expressions

1. Pseudo code for the procedure to generate all equivalent expressions

   ```rs
   #[derive(Clone, PartialEq, Eq)]
   struct RealtionalAlgebraExpr;

   struct Rule;

   impl Rule {
      /// Return `Some(new_expr)` if `self` can be applied on `old_expr`.
      fn apply(&self, old_expr: &RealtionalAlgebraExpr) -> Option<RealtionalAlgebraExpr> {
         unimplemented!()
      }
   }

   static EQUIVALENE_RULES: Vec<Rule> = Vec::new();

   fn generate_equivalent_expr(init_expr: &RealtionalAlgebraExpr) -> Vec<RealtionalAlgebraExpr> {
      let mut ret: Vec<RealtionalAlgebraExpr> = vec![init_expr.clone()];

      for expr in ret.iter() {
         for rule in EQUIVALENE_RULES.iter() {
               if let Some(new_expr) = rule.apply(&expr) {
                  if !ret.contains(&new_expr) {
                     ret.push(new_expr);
                  }
               }
         }
      }

      ret
   }
   ```

   The above code is problematic because it changes the `ret` while iterating it...

   ```sh
   $ cargo check -q
   error[E0502]: cannot borrow `ret` as mutable because it is also borrowed as immutable
   --> src/main.rs:22:21
      |
   18 |     for expr in ret.iter() {
      |                 ----------
      |                 |
      |                 immutable borrow occurs here
      |                 immutable borrow later used here
   ...
   22 |                     ret.push(new_expr);
      |                     ^^^^^^^^^^^^^^^^^^ mutable borrow occurs here

   For more information about this error, try `rustc --explain E0502`.
   warning: `rust` (bin "rust") generated 1 warning
   error: could not compile `rust` (bin "rust") due to 1 previous error; 1 warning emitted
   ```

2. The procedure listed in the last note is extremely costly in both space and 
   in time.

   There are 2 ways that a query optimizer can reduce the cost, I am gonna cover
   it here since I don't quite understand the whole process.

   TODO: revisit this in the future.

3. Among the equivalent expressions, a cost-based query optimizer will analyze
   their cost and choose the one that is least costly.

   > Cost analysis will be covered in section 16.3
   
   > For the part that how the query optimizer will choose the least costly one, 
   > see 16.4.

# 16.3 Estimating Statistics of Expression Results

1. How to do cost analysis of an expression

   In chapter 15, we covered the implementations of different operators and their
   costs, that is for single operator.

   An expression consists of a lot of operators, it is a tree of operators.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-05-02%20at%2020.36.36.png)

   We can start from the bottom-level operations, and estimate their statistics,
   and continue the process on higher-level operations till we reach the root of
   the tree.

2. One thing to note is that the estimates are not very accurate, since they are
   based on assumptions that may not hold exactly.

   A query-evaluation plan that has the lowest estimated execution cost may 
   therefore not actually have the lowest actual execution cost.

   However, real-world expreience has shown that even if the estimates are not 
   precise, the plan with the lowest estimated costs usually have actual execution
   costs that are either the lowest actual execution costs or are close to the
   lowest actual execution costs.

## 16.3.1 Catalog information

1. For a cost-based optimizer, the database needs to maintain some metadata
   in its catalog, e.g.

   1. the # of tuples in a relation
   2. the # of blocks in a relation
   3. the blocking factor of a relation, i.e., the # of tuples that fit into one
      block
   4. the # of distinct values of an attribute of a relation

      > Columnar formats store this value directly in the file

   Maintaining these metadata can be costly and can enforce a limit on concurrency,
   so most systems do not update the statistics on every modification. Instead, 
   they update the statistics during periods of light system load.

2. If no catalog is available, then the query optimizer would assume the data
   is uniformly distributed.

   > Well, you should recall that you have done this when doing algorithm 
   > complexity analysis.
   
   This can be inaccurate, but it is the best thing we can do when there is no
   statistics.

3. How to create statistics

   The most straightforward way is to scan the whole relation, then build the
   statistics, but this is inpractical for AP engines with millions rows, the
   more common way is to build the statistics for sample data.

   NOTE: the sample must be random.

3. Update strategy of statistics

   > Updating statistics can be costly.

   1. No automatic update, they need the DBA to do it manuall.
   2. Automatically update after a period of time
   3. Automatically update when the optimizer realized that the statistics are
      kinda outdated.

   > In PostgreSQL, one can update the statistics manually or automatically.
   >
   > 1. Use the [`analyze`][link1] command to update it manually
   >
   > [link1]: https://www.postgresql.org/docs/current/sql-analyze.html
   >
   > 2. Enable the automatic refresh with [feature `autovacuum`][link2]
   >
   > [link2]: https://www.postgresql.org/docs/current/routine-vacuuming.html#AUTOVACUUM

## 16.3.2 Selection (in relational algebra) Size Estimation

> The size estimate of the result of a selection operation depends on the selection
> predicate, we first consider a single equality predicate (field = value), then
> a gingle comparsion predicate, and finally combinations of predicates.

> Nonations
>
> * $ n_r $: the # of tuples in the relation `r`
> * $ b_r $: the # of blocks in the relation `r`
> * $ V(A, r) $: the # of distinct values that appear in the relation `r`


1. Selection with single equality predicate 

   $$ \sigma_{A=a} $$

   Some systems maintain statistics for values that are frequently accessed, if
   `a` is one of them, we can directly use that value directly.

   If there is no statistic for this value, then we assume

   1. The data of attribute `A` is uniformly distributed
      
      > This is not realistic

   2. Value `a` exists in the data

      > This is typically true.

   The row count is $ n_r / V(A, r) $

2. Selection with single comparsion predicate

   $$ \sigma_{A \le v} (r) $$

   Suppose data is uniformly distributed and we know the maximum and minimum value
   of attribute A, then the size estimate is:

   * 0 if $ v < min(A) $
   * $ n_r $ if $ v \ge max(A) $
   * $ n_r \times ( \frac{v - min(A)}{max(A) - min(A)}) $, otherwise

   If the value `v` is unknown during the plan stage, for example:

   > In DudkDB, `random()` return a random number between 0 and 1.

   ```sql
   SELECT * FROM table1 WHERE A <= random() * 100;
   ```

   For this case, we have no idea what the value of `v` is, we simply assume that
   this predicate would return half tuples, so the row count is $ n_r / 2 $.

   > This is quite stupid, but it is the best thing we can do.
   

3. Selection with combinations of predicates

   1. Conjunction

      $$ \sigma_{ \theta_{1} \land \theta_{2} \land \cdot \cdot \cdot \land \theta_{n}} (r) $$

      For each predicate $ \theta_i $, assume the size of $ \sigma_{\theta_{i}} (r) $
      is $ s_i $, so the probability of $ \theta_{i} $ is $ s_i / n_r $, assume 
      that those predicates are **independent**, 

      > again, this is not realistic

      then the probability of $ \theta_1 \land \theta_2 \land \cdot \cdot \cdot \land \theta_n $
      is $ \frac{s_1 \times s_2 \times s_3 \times \cdot \cdot \cdot s_n}{n_r^n} $, 
      so the row count is $ n_r \times  \frac{s_1 \times s_2 \times s_3 \times \cdot \cdot \cdot s_n}{n_r^n} $ 

   2. Disjunction

      $$ \sigma_{ \theta_{1} \lor \theta_{2} \lor \cdot \cdot \cdot \lor \theta_{n}} (r) $$

      Use $ P(E) $ to denote the probability of $E$, then 

      > we still assume that those predicates are independent.

      $$ P(\sigma_{ \theta_{1} \lor \theta_{2} \lor \cdot \cdot \cdot \lor \theta_{n}} (r)) = 1 - P(\sigma_{\lnot ( \theta_{1} \lor \theta_{2} \lor \cdot \cdot \cdot \lor \theta_{n})} (r)) $$
      $$ = 1 - P(\sigma_{(\lnot \theta_1) \land (\lnot \theta_2) \land \cdot \cdot \cdot \land (\lnot \theta_n)} (r)) $$
      $$ = 1 - (1-\frac{s_1}{n_r}) \times (1-\frac{s_2}{n_r}) \times \cdot \cdot \cdot \times (1 - \frac{s_n}{n_r})  $$

   3. Negation

      $$ \sigma_{\lnot \theta} (r) $$

      $$ P(\sigma_{\lnot \theta} (r) ) = 1 - P(\sigma_{\theta} (r)) $$

      For how to calculate $ P(\sigma_{\theta} (r)) $, please refer to the previous sections. 

## 16.3.3 Join Size Estimation

1. Estimate the size of Cartesian Product

   > Use $ l_r $ to denote the size of the relation $ r $.

   $ r \times s $ contains $ n_r \times n_s $ tuples, each tuple from the result would occupy $ l_r + l_s $ bytes.

2. Estimate the size of natural join $ r(R) \Join s(S) $

   1. If $ R \cap S = \emptyset $, then this natural join would be a Cartesian 
      product, i.e., $ n_r \times n_s $ tuples.
      
   2. If $ R \cap S $ is a key (unique) for $ R $, then we know that a tuple in
      $s$ will join with at most 1 tuple from $r$, so that the # of tuples will
      be no greater than $ n_s $.

      > If unmatched, it will join with 0 tuples. Otherwise, it is guaranteed to
      > be 1.

      A more special case, if $ R \cap S $ is a primary key on $R$ and a foreign
      key on $S$, then there won't be any unmatched tuples from $S$, so the row
      count will be exactly $n_s$.

      Examples:

      | ID | name |
      |----|------|
      | 1  | steve|

      | ID | event |
      |----|-------|
      | 1  | yes   |
      | 1  | no    |
      | 2  | ye    |

      The row count won't be greater than $n_s$ (3), actually, the row count is 2.

   3. If $ R \cap S $ is neither a key of relation $r$, nor a key of table $s$

      Without any further statistics information, we simply assume:

      1. the data in both relations are uniformly distributed
      2. for relation $r$, every value of attribute $R \cap S$ exists in relation
         $s$, and vice versa

      then for every tuple in relation $R$, the # of tuples in $r \Join s$ produced
      by it should be $ \frac{n_s}{V(R \cap S,s)} $, then considering all the tuples
      from relation $r$, the # of tuples will be $ n_r \times  \frac{n_s}{V(R \cap S,s)} $

      Reverse the roles of $r$ and $s$, we will get an estimate 
      $ \frac{n_r \times n_s}{V(R \cap S, r)} $.

      These 2 estimates can be different since $V(R \cap S, r)$ and $V(R \cap S, s)$
      can be different. 
      
      > QUES: The textbook says that we should use the lower one as the estimate,
      > I don't quite get it.

3. Estimate the size of theta join

   Consider the definition of theta join

   $$ r \Join_{\theta} s = \sigma_{\theta} (r \times s) $$

   We estimate the size of a theta join by:

   1. estimate the size of the Cartesian Product
   2. estimate the size of the selection operation

## 16.3.4 Size Estimation for Other Operationo

1. Projection

   The # of tuples of a projection $ \Pi_{A}(r)$ is the # of tuples of $r$.

2. Aggregation
  
   The size estimation is the # of distinct value of those `GROUP BY` attributes.
   
3. Set operations

   1. If a set operation is done against the same relation, then it can be 
      rewritten as selection

      * Union

        $$ (\sigma_{\theta_1} r) \cup (\sigma_{\theta_2} r) \equiv \sigma_{\theta_1 \lor \theta_2} (r) $$

      * Intersection

        $$ (\sigma_{\theta_1} r) \cap (\sigma_{\theta_2} r) \equiv \sigma_{\theta_1 \land \theta_2} (r) $$

      * Set difference
        
        $$ (\sigma_{\theta_1} r) \setminus (\sigma_{\theta_2} r) \equiv \sigma_{(\theta_1) \land (\lnot \theta_2) } (r)$$

      then we can do size estimation to the selection operation.

   2. Otherwise

      * Union: the # of tuples in $r$ + the # of tuples in $s$
      * Intersection: min(the # of tuples in $r$, the # of tuples in $s$)
      * Set difference ($r - s$): the # of tuples in $r$

4. Outer join
   
   * left outer join: the # of tuples in $r \Join s$ + the # of tuples in $r$ 
   * right outer join: the # of tuples in $r \Join s$ + the # of tuples in $s$ 
   * full outer join:the # of tuples in $r \Join s$ + the # of tuples in $r$ + the # of tuples in $s$

## 16.3.5 Estimation of Number of Distinct Values 

> In the previous sections, $V(A, r)$ is frequently used to estimate the size, so
> it is quite important. If $r$ is a stored table, then it is hightly possible
> that such information is stored in the catalog as we have seen in section 16.3.1
>
> However, if $r$ is a relation that needs to be computed during runtime, then
> we need to estimate it!

1. If $r$ is a relation generated from a selection operation with simple predicates

   1. If the predicate is something like $ A = a $, then $V(A, \sigma_{\theta} (r))$
      will just be 1.

   2. If the predicate is something like $ A = a \lor A = b \lor \cdot \cdot \cdot \lor A = z $, 
      then $V(A, \sigma_{\theta} (r))$ will just be the # of specified values.

   3. In all other cases, we just estimate it with

      $$ min(V(A, r), n_{\sigma_{\theta} (r)}) $$

2. If $r$ is a relation generated from a selection operation with join

   > steve: I don't think this will help/work in real world systems, so I just 
   > skip the last few paragraphs...

3. If $r$ is a relation generated from a selection operation with projection
4. If $r$ is a relation generated from a selection operation with aggregation

# 16.4 Choice of Evaluation Plans (physical plan)

> We first start by covering simple join-order and join algorithm selection in
> section 16.4.1, then we introduce how to build a general-purpose optimizer
> that is based on equivalence rules in section 16.4.2.
>
> Exploring the space of all possible plans may be too expensive for complex
> queries, most optimizers use heuristics to reduce the cost of query optimization,
> at the cost of the possibility of not finding the optimal plan, this will be
> explained in section 16.4.3.

## 16.4.1 Cost-Based Join-Order Selection

1. What is dynamic programming

   It is a way of finding optimal solutions to a problem by:

   1. Splitting the problem into over-lapping sub-problems
   2. Recursively find the optimal solution to the sub-problems

   A problem can be sovled via DP if:

   1. It can be splitted into sub-problems
   2. The splitted sub-problems are over-lapping

      If they are not over-lapping, then we use the divide-and-conquer algorithm.

   3. If we get the optimal solution for the sub-problems, then we get the optimal
      solution for the problem.

2. Why is DP good

   To find the optimal solution of a problem, one way to do it is to enumerate
   all the possible solutions and use the best one.

   This is not pratical if the the problem can have tons of solutions, enumerating
   all the solutions will be a problem as well.

   DP **won't** enumerate all the solutions, instead, since the problem can be
   splitted into overlapping sub-problems, it will record the best solutions for
   those sub-problems because these solutions can be **reused** in the future
   given that the sub-problems are overlapping. By reusing the solutions, the
   whole search space can be enormously shrunk.

   For example, to join table a and b, if I alreay know the best way to do it is
   $ a \Join b $, then I won't enumerate all the possible approaches when I want
   to join them again in the future.
  
3. With $n$ tables being joined, there can be $\frac{(2(n-1))!}{(n-1)!}$ join
   orders.

   > TODO: Prove this
   >
   >
   > A wrong proof (just record it here so that future steve can understand the 
   > mistake that I made)
   >
   > $$ A_{n}^{2} \times A_{n-1}^{2} \times ... \times A_{2}^{2} $$
   >
   > When $n=4$, this formula will be 144, so there are some duplciates cases.

   | n | the # of join orders |
   |---|----------------------|
   | 3 | 12                   | 
   | 4 | 120                  |
   | 5 | 1680                 |
   | 7 |665280                |
   | 10|17.6 billion          |

4. Using DP to do join order selection

   Pseudocode:

   For the following impl, we:

   1. Access every table through `TableScan`
   2. Join `a` and `b` via: `a NestedLoopJoin b`
   3. This chooses the physical plan for the join operation since it checks
      the join algorithm, the table access path.

   ```rs
   use once_cell::sync::Lazy;
   use std::sync::RwLock;
   use std::{collections::HashMap, ops::BitAnd};

   #[easy_ext::ext(SubSet)]
   impl &[String] {
       /// Return all the non-empty, proper subsets.
       pub fn non_empty_proper_subsets(&self) -> impl Iterator<Item = Vec<String>> {
           let len = self.len();
           let n_subset = 2_usize.pow(len as u32);
           let mut ret = Vec::with_capacity(n_subset - 2);

           for idx in 1..(n_subset - 1) {
               let mut subset = Vec::new();
               for bit_idx in 0..len {
                   if idx.bitand(2_usize.pow(bit_idx as u32)) != 0 {
                       subset.push(self[bit_idx].clone());
                   }
               }
               ret.push(subset);
           }

           ret.into_iter()
       }

       /// Set difference operation: `self - other`.
       //
       // We can iterate `self` and do binary search to search if an item exists
       // in `other`, or we can use the approach introduced in chapter 15 that is
       // used to implement set difference.
       pub fn difference(&self, other: &Self) -> Vec<String> {
           let mut ret = Vec::new();
           for item in self.iter() {
               if other.binary_search(item).is_err() {
                    ret.push(item.clone());
               }
           }
           ret
       }
   }

   #[derive(Debug, Clone)]
   struct PlanWithCost {
       /// The cost of a plan.
       cost: f64,
       /// The actual plan, just use an unit type for it.
       plan: Plan,
   }

   #[derive(Debug, Clone)]
   enum AccessPath {
       TableScan,
       IndexScan,
   }

   #[derive(Debug, Clone)]
   enum Plan {
       TableScan {
           table: String,
           path: AccessPath,
       },
       Join {
           left: Box<PlanWithCost>,
           algorithm: JoinAlgorithm,
           right: Box<PlanWithCost>,
       },
   }

   #[derive(Debug, Clone)]
   enum JoinAlgorithm {
       NestedLoopJoin,
       BlockNestedLoopJoin,
       IndexedNestedLoopJoin,
       MergeJoin,
       BasicHashJoin,
       PartitionedHashJoin,
   }

   /// A global variable used to stored the best plans we have investiaged.
   ///
   /// This is how dynamic programming typically works, store finds so that
   /// we don't need to reinvestigage.
   static BEST_PLAN: Lazy<RwLock<HashMap<Vec<String>, PlanWithCost>>> =
       Lazy::new(|| RwLock::new(HashMap::new()));

   /// Find the best plan for joining the tables specified in `table_set`.
   ///
   /// # Assumption
   ///
   /// Since `HashSet<String>` is not `Hash`, so we use a `Vec<String>` instead, and
   /// we assume that the table names stored in the `Vec` is sorted.
   fn find_best_plan(table_set: &[String]) -> PlanWithCost {
       // this has already been found
       if let Some(plan) = BEST_PLAN.read().unwrap().get(table_set) {
           return plan.clone();
       }

       if table_set.len() == 1 {
           // find the best access path for this table
           //
           // We just use a TableScan here.
           let best_plan_for_accessing_this_table = PlanWithCost {
               cost: 1.0,
               plan: Plan::TableScan {
                   table: table_set[0].to_string(),
                   path: AccessPath::TableScan,
               },
           };
           // store it
           BEST_PLAN.write().unwrap().insert(
               table_set.to_vec(),
               best_plan_for_accessing_this_table.clone(),
           );
           return best_plan_for_accessing_this_table;
       }

       let subsets = table_set
           .non_empty_proper_subsets()
           .collect::<Vec<_>>();
       let subsets_len = subsets.len();
       let (left_part, right_part) = subsets.split_at(subsets_len / 2);
       let paired_iter = std::iter::zip(left_part, right_part.iter().rev());

       let mut plan_for_table: Option<PlanWithCost> = None;
       for (left, right) in paired_iter {
           let best_plan_for_left = find_best_plan(left);
           let best_plan_for_right = find_best_plan(right);
           let mut write_gaurd = BEST_PLAN.write().unwrap();
           write_gaurd.insert(left.to_vec(), best_plan_for_left.clone());
           write_gaurd.insert(right.to_vec(), best_plan_for_right.clone());
           drop(write_gaurd);

           // Choose the algorithm to join left and right
           //
           // We just do it like: left NestedLoopJoin right
           let plan = Plan::Join {
               left: Box::new(best_plan_for_left),
               algorithm: JoinAlgorithm::NestedLoopJoin,
               right: Box::new(best_plan_for_right),
           };
           let plan = PlanWithCost { cost: 1.0, plan };

           match plan_for_table {
               Some(ref mut prev_plan) => {
                   if plan.cost < prev_plan.cost {
                       *prev_plan = plan;
                   }
               }
               None => plan_for_table = Some(plan),
           }
       }
       plan_for_table.unwrap()
   }

   fn main() {
       let best_plan = find_best_plan(&["a".into(), "b".into(), "c".into()]);
       println!("{:#?}", best_plan);
   }
   ```

5. 


## 16.4.2 Cost-Based Optimization with Equivalence Rules
## 16.4.3 Heuristics in Optimization
## 16.4.4 Optimizing Nested Subqueries
# 16.5 Materialized Views
# 16.6 Advanced Topics in Query Optimization
# 16.7 Summary
