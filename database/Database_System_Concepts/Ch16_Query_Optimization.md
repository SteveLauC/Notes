> This chapter covers:
>
> 1. Equivalent rules in relational algebra (16.2)
>
>    > Review Relation Algebra (Ch2) if you cannot recall them clearly.
>
> 2. For a generated unoptimized logical plan, we know how to convert it to 
>    an equivalent one using equivalent rules. Then we need to choose the least
>    costly one by estimating their cost (16.3)
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
>   * 16.4.4 Optimizing Nested Subqueries (correlated subqueries/semi-join/anti-join)
> * 16.5 Materialized Views
>   > 1. How to maintain (keep it update-to-date) it
>   > 2. How to do queries on materialized views
>
>   * 16.5.1 View Maintenance
>   * 16.5.2 Incremental View Maintenance
>     * 16.5.2.1 Join Operation
>     * 16.5.2.2 Selection and Projection Operations
>     * 16.5.2.3 Aggregation Operations
>     * 16.5.2.4 Other Operations
>     * 16.5.2.5 Handling Expressions (complex queries)
> * 16.6 Advanced Topics in Query Optimization
>   * 16.6.1 Top-K Optimization
>   * 16.6.2 Join Minimization
>   * 16.6.3 Optimization of Updates
>   * 16.6.4 Multiquery Optimization and Shared Scans
>   * 16.6.5 Parametric Query Optimization
>   * 16.6.6 Adaptive Query Processing
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

      After converting the given query to its equivalent alternatives, we need to 
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

      > QUES: is this applicable to outer join?
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
   statistics, but this is impractical for AP engines with millions rows, the
   more common way is to build the statistics for sample data.

   NOTE: the sample must be random.

3. Update strategy of statistics

   > Updating statistics can be costly.

   1. No automatic update, they need the DBA to do it manually.
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

   A problem can be solved via DP if:

   1. It can be split into sub-problems
   2. The split sub-problems are over-lapping

      If they are not over-lapping, then we use the divide-and-conquer algorithm.

   3. If we get the optimal solution for the sub-problems, then we get the optimal
      solution for the problem.

2. Why is DP good

   To find the optimal solution of a problem, one way to do it is to enumerate
   all the possible solutions and use the best one.

   This is not practical if the the problem can have tons of solutions, enumerating
   all the solutions will be a problem as well.

   DP **won't** enumerate all the solutions, instead, since the problem can be
   split into overlapping sub-problems, it will record the best solutions for
   those sub-problems because these solutions can be **reused** in the future
   given that the sub-problems are overlapping. By reusing the solutions, the
   whole search space can be enormously shrunk.

   For example, to join table a and b, if I already know the best way to do it is
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
   > When $n=4$, this formula will be 144, so there are some duplicates cases.

   | n | the # of join orders |
   |---|----------------------|
   | 3 | 12                   | 
   | 4 | 120                  |
   | 5 | 1680                 |
   | 7 |665280                |
   | 10|17.6 billion          |

4. Using DP to do join order selection

   Pseudo-code:

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

5. The textbook says:

   > The join cost formulae that we saw in Chapter 15 can be used with appropriate
   > modifications to ignore the cost of reading the input relations.

   I don't quite understand because the cost analysis introduced in Chapter 15
   primarily and almost only covers the cost of the I/O (block transfer + seek)

6. Time complexity $ O(3^n) $

7. The order in which tuples are generated by the join of a set of relations is
   important for finding the best **overall** join order since it can affect
   the further operations.

   For example, `SELECT * FROM a NATURAL JOIN b ON a.id = b.id ORDER BY id` even
   though the plan for joining a and b with the least cost probably uses hash join,
   we may still want to use sort merge join given that the `SELECT` phrase requires
   that the output to be ordered.

   **Interesting sort order**: If an order of tuples can be useful for later 
   operation, then it is said to be an interesting sort order.

8. To take interesting sort order into account, the pseudocode has to be modified.
   We no longer store the best plan only for the subsets, instead, we store the
   best plan for each subset, for each interesting sort order of the join result
   for that subset. Then the global variable would be changed to: 
   `HashMap<(Subset, Interesting sort order), PlanWithCost>`

   The number of interesting sort order is found to be small, so the time 
   complexity still remains at $ O(3^n) $.

   When n is 10, this number is around 59049. Enumerating join orders for 10 relations
   would result in 17.6 billion options.

9. Another optimization that can be made is that don't do cartesian product, if
   2 relations don't have any join condition linking the 2 relations, then we 
   should not join them while searching for the best plan.

## 16.4.2 Cost-Based Optimization with Equivalence Rules

> The join order optimization introduced in the last section is the most common
> one in real-world scenarios. It uses equivalence rules:
>
> 1. Theta inner join is commutative
> 2. Natural inner join is associative
>
> There are tons of queries that use other features, this section covers how to
> do general cost based query optimization with equivalence rules.
>
> In section 16.2.4, we saw how to enumerate all the equivalent expressions to
> a given query, that is for logical plan. The case for physical plan generation
> is pretty > similar to that process, where we add a new class of equivalence
> rules **physical equivalence rules**, which can be used to convert a logical
> operator into the corresponding physical one, e.g., join -> hash join.

> QUES: for the physical equivalence rules described above, haven't we seen it
> in the last section?

It turns out that this section won't cover the details on how to implement a
cost-based optimizer using equivalence rules. IMO, that is defined quite
complicated.

The textbook says that to implement it based on the pseudo-code introduced in
section 16.2.4, we should:

1. Make an space-efficient representation of expressions that avoids making multiple
   copies of the same sub-expressions when equivalence rules are applied.
   
   > steve: can we do this `Rc` or `Arc`?

2. Efficient techniques for detecting duplicate derivations of the same 
   expression, i.e., when deriving a new expression using one equivalence
   rule, we can efficiently check if this expression exists in our expression
   collection.

   > The most stupid way to do is to iterate over all the existing expressions,
   > then compare them.

3. Dynamic programming

   We don't want to enumerate all the expressions, for the best plan of a 
   sub-expression, we need to store it so that we can reuse it in the future.

## 16.4.3 Heuristics in Optimization

> Use heuristics to limit the search space, DP is good, but it is not sufficient.

1. What is heuristic query optimization?

   The query optimization approaches we talked about generally consist of 2 
   categorizes:

   1. Rule-based
   2. Cost-based

   Heuristic approach is kinda similar to the rule-based one, we will have some
   predefined rules, but with heuristic approach, we **blindly** follow the defined
   rules and apply them during query optimization **without considering their actual 
   cost** because we, as human, **trust** these rules that they will reduce the cost.

   Some heuristic rules:

   1. Perform selection operations as early as possible 
   2. Perform projections early

   > It is usually better to perform selections earlier than projections, since
   > selections have the potential to reduce the sizes of relations greatly.

   Yeah, they "should" speed the query since the the amount of data that needs to
   be transfered is shrank.

   And for a single-table scan, there rules are definitely correct and effective.
   But for operations that involve multiple tables, it can make things worse.

   But heuristic optimizer is used wiedly in real world engines since:

   1. Find the best plan with equivalence rules is NP hard
   2. Cost estimation can be literally a guess
   3. Heuristic optimizer can make things worse, but it is intuitive

2. Query optimization is hard, most systems don't do cost-based optimization for
   all the operations, they

   1. Use heuristic approaches
   2. Use cost-based approach for join order selection

   > This is confirmed by the comment from Andrew Lamb, see the summary section.

3. heuristic approach in join order selection

   > This is used by a lot of query optimizers, including the System R optimizer.

   When choosing join order, they don't consider all the options, instead, only
   those **left-deep join orders** will be considered.

   > Left-deep join order is an order where teh right operand of each join is
   > one of the initial relations.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-05-26%20at%209.18.21%20AM.png)

   They do this because left-deep join tree will be easy to implement, i.e., it
   fits the pipeline model pretty well.

4. optimization cost budget

   For those cost-based optimizers, they typically allow a cost budget to be 
   specified for query optimization, the search for the optimal plan is terminated
   when the budget is exceeded, and the best plan that is found up to this point
   is returned.

5. Plan caching

   Finding the optimal plan for a given query is hard, why not cache the best
   plan we have found for the queries.

   PostgreSQL does this for the query given by `PREPARE` statement.

## 16.4.4 Optimizing Nested Sub-queries

> https://ericfu.me/subquery-optimization/ 

> https://duckdb.org/2023/05/26/correlated-subqueries-in-sql.html

1. There are 2 kinds of subqueries in SQL

   1. Non-correlated subquery (非关联子查询)
      
      This means that the subquery does not involve anything from the outer 
      relation, i.e., they are totally indepedent.

      This kinda of subquery needs no special optimization, just execute the
      subquery, materialize it and pass it to the upper executors.

      We care more obout correlated subquery.
     
   2. Correlated subquery (关联子查询)

      Different from non-correlated subquery, a correlated subquery itself is
      incomplete and needs some information from the relation of the outer query.

      One can basically treat it as a function that takes a argument (the 
      correlated field) from the outer relation.

      Let's see an example:

      ```sql
      D SHOW TABLES;
      ┌────────────┐
      │    name    │
      │  varchar   │
      ├────────────┤
      │ instructor │
      │ teaches    │
      └────────────┘
      D SELECT * FROM instructor;
      ┌───────┬─────────┐
      │  id   │  name   │
      │ int32 │ varchar │
      ├───────┼─────────┤
      │     0 │ steve   │
      │     1 │ mike    │
      └───────┴─────────┘
      D SELECT * FROM teaches;
      ┌───────┬───────┬─────────┐
      │  id   │ year  │ course  │
      │ int32 │ int32 │ varchar │
      ├───────┼───────┼─────────┤
      │     0 │  2019 │ math    │
      │     0 │  2019 │ english │
      └───────┴───────┴─────────┘
      ```

      Let's find the instructors that have ever teached a class:

      ```sql
      D SELECT name
        FROM instructor
        WHERE EXISTS
            (SELECT *
             FROM teaches
             WHERE teaches.id = instructor.id);
      ┌─────────┐
      │  name   │
      │ varchar │
      ├─────────┤
      │ steve   │
      └─────────┘
      ```

      The above subquery, `SELECT * FROM teaches WHERE teaches.id = instructor.id`
      itself is not a valid SQL due to that extra `instructor.id` field, this field
      needs to be provided by the outer relation `instructor`.

      Let's take a look at the unoptimized query plan of this query:

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-05-26%20at%204.12.08%20PM.png)

      You can see that the subquery is under a `Filter` node, i.e., for every tuple
      from the `instructor` table, we would scan the whole `teaches` table, this is
      basically equivalent to a nested-loop join, which is quite expensive and 
      inefficient.

      Then how can we optimize it? Rewrite it using join.

      > Decorrelation: the process of replacing a nested query by a query with 
      > a join, semi-join, or anti-semijoin is called "decorelation".
      
      At the first glance, we
      should be able to do it by simply 
      `instructor JOIN teaches ON instructor.id=teaches.id`, but:

      ```sql
      D SELECT name 
        FROM instructor JOIN teaches 
        ON instructor.id = teaches.id;
      ┌─────────┐
      │  name   │
      │ varchar │
      ├─────────┤
      │ steve   │
      │ steve   │
      └─────────┘
      ```

      As you can see, it returns 2 "steve" since instructor steve has teached 
      2 courses. Then, how can I optimize this correctly, using semi join.

      ```sql
      D SELECT name
        FROM instructor SEMI JOIN teaches
        ON instructor.id = teaches.id;
      ┌─────────┐
      │  name   │
      │ varchar │
      ├─────────┤
      │ steve   │
      └─────────┘
      ```

2. What are semi join and anti join

   > Not sure where these 2 joins come from

   Semi join: By joining $r$ and $s$, if a tuple $r_i$ appears n times in $r$, 
   then it will appear n times in the join result if there is a tuple $s_j$ 
   such that $ r_i $ and $s_j$ together satisfy the join condition.
   
   Semi join consists of:

   * left semi join
   * right semi join

   Different from other join we have seen before, semi join only return the half
   part data. Left semi join would return tuples from the left table, right semi
   join would return tuples from the right table.

   Anti join is in the reverse direction of semi join, a tuple will be returned
   only if it does satisfy the join condition.

3. Decorrelation is hard, many optimizers do only a limited amount of decorrelation.

4. For non-correlated subqueries, there are 3 kinds of them:

   1. Scalar subquery

      Scalar subquery would return a single value, i.e., a table with only 1 
      column and 1 row.

      If it returns more than 1 line of data, then a runtime error would occur.
      
   2. `EXISTS`

      This returns a boolean value, one can basically treat it as something like
      "set is not empty".

   3. `IN`/`ANY`/`ALL`

      returns a boolean value.

      `IN (list)` is equivalent to `= ANY (list)` and `<> ALL(list)`.

# 16.5 Materialized Views

1. When you define a view in database, the system will only store the query, the
   data that will be returned by the query won't be stored.

   Materialized view is different, it is view but materialized!

   > QUES: What is materialization?
   >
   > We have
   >
   > 1. late/early materialization
   >    Materialization here refers to the process of converting the data from 
   >    a format to the data format that can be consumed by the upper execution 
   >    node/operator.
   >
   > 2. materialized view
   >
   >    Seemingly, materialization means the processing of storing data.

2. When to need materialized views?

   * The same **complex** query needs to be repeatedly recalculated over a large 
     amount of data.
   * Low end-to-end latency is required but access to up-to-the-moment data 
     is not a critical requirement
   * Storage space is not a major concern. 
   * You data won't be updated frequently (or the cost of updating materialized 
     view can be large, thus the write performance can be reduced)

## 16.5.1 View Maintenance

1. A problem with materialized views is they have to be updated when the data
   used in the view definition changes.

   The procedure of keeping a materialzed view up-to-date with the underlying
   data is called **view maintenance**.

   > QUES: I am interested in why it isn't called materialized view maintenance.

2. There are generally 3 ways on how to maintain materialized view

   1. Update it manually, i.e., whenever there are changes to the data, update
      the materialized view. This is tedious and error-prone.

   2. Define triggers

   3. The DBMS can automatically keep your materialized view update-to-date

      1. Immediate view maintenance

      2. Deferred view maintenance

      > Well, PostgreSQL does not have this, users need to manually 
      > `REFRESH MATERIALIZED VIEW`, or users can use triggers.

3. How to update materialized view

   1. Rebuild a completely new one on modification
   2. Do incremental update (obviously, this is a better option)

4. When it comes to view maintenance, materialized view is kinda similar to
   index, index of the most databases will be updated automatically, and
   should be incremental update.

## 16.5.2 Incremental View Maintenance

> Before reading the next few sections, think about the **query**(R) operations 
> we have in SQL:
>
> > I don't think there would be people that will create materialized views for 
> > insert/update/delete.
> >
> > PostgreSQL does not allow materialized views with `INSERT/UPDATE/DELETE`.
>
> 1. Projection
> 2. Selection
> 3. Aggregation
> 4. Join and subquery
> 5. Set operation

> For incremental update, you should be able to update the materialied view by
> updating view only for the **diff** part.

1. In the following sections, we introduce how to update the materialized view
   if they are defined as the following operations.

   Modifications to the table can be `INSERT/UPDATE/DELETE`, We only cover `INSERT`
   and `DELETE` since `UPDATE` can be achieved by `INSERT` and `DELETE`.

### 16.5.2.1 Join Operation

$ mv = r \Join s $

* insert $i$ to $r$
     
  > Applied Property: cartesian product distributes over set union.

  $$ mv_{new} = $$
  $$ (r \cup i) \Join s  = $$
  $$ (r \Join s) \cup (i \Join s) =  $$
  $$ (mv_{old}) \cup (i \Join s) =  $$

* delete $i$ from $r$

  > Applied property: cartesian product distributes over set difference.

  $$ mv_{new} = $$
  $$ (r \setminus i ) \Join s = $$
  $$ (r \Join s) \setminus (i \Join s) = $$
  $$ mv_{old} \setminus (i \Join s) = $$

> Modifiction to $r$ will be handled in exactly symmetric fashion.

### 16.5.2.2 Selection and Projection Operations

> I assume that duplicate tuples are allowed in relation, which is different from
> the procedure descibed in the textbook.
>
> This is closer to the case in SQL.

1. Selection $ mv = \sigma_{\theta} (r) $

   * Insert $i$ to $r$

     > Applied property: selection distributes over set union (introduced in 
     > section 16.2.1)

     $$ mv_{new} = \sigma_{\theta} (r \cup i) \equiv $$
     $$ mv_{new} = (\sigma_{\theta} (r)) \cup (\sigma_{\theta} (i)) = $$
     $$ mv_{new} = (mv_{old}) \cup (\sigma_{\theta} (i)) $$

     We do the selection operation on $i$, if a tuple satisfies the condition, 
     append it to the materialized view (no matter if there is a tuple with the
     same value exists or not).

     > So technically it is not a `UNION` operation, it is a `UNION ALL`.

   * Delete $i$ from $r$

     > Applied property: selection distributes over set difference (introduced
     > in section 16.2.1)

     $$ mv_{new} = \sigma_{\theta} (r \setminus i) \equiv $$
     $$ mv_{new} = \sigma_{\theta} (r) \setminus \sigma_{\theta} (i) = $$
     $$ mv_{new} = mv_{old} \setminus \sigma_{\theta} (i) $$

     Traverse the tuples in $i$, if one satisfies the condition, remove it from
     the materialized view.

     If there is only one tuple with this value, then just remove it, otherwise,
     just pick one from them and remove it.

2. Projection $ mv = \Pi_{A} (r) $ 

   1. Insert $i$ to $r$

      > Applied property: Projection distributes over set union
      
      $$ mv_{new} = \Pi_{A} ( r \cup i) \equiv $$
      $$ mv_{new} = \Pi_{A} (r) \cup \Pi_{A} (i) = $$
      $$ mv_{new} = mv_{old} \cup \Pi_{A} (i) $$
      
      Still, it should be `UNION ALL` rather than `UNION`. 

   2. Delete $i$ from $r$

      Projection DOES NOT distribute over set difference(see section 16.2.1), but
      I think we can simply do the projection to $i$ and removee the value from 
      the materialized view given that:

      1. We allow duplicate values in "set"s.
      2. $r$ is a super-set of $i$

### 16.5.2.3 Aggregation Operations

> They can have `GROUP BY` statements.

1. Count

   1. insert a tuple
      
      Check the `GROUP BY` value in the materialized view, if it does not exist,
      add a new tuple `value, 1` to the materialized view, otherwise, increment
      the corresponding counter by 1.

   2. Delete a tuple
      
      Check the `GROUP BY` value in the materialized view (it should exist)

      Decrease the counter by 1, and if the counter becomes 0, remove this tuple.

2. Sum

   1. insert a tuple

      Check the `GROUP BY` value in the materialized view, if it does not exist,
      then, add a new tuple `value, value` to the view, otherwise, increment the
      sum value.

   2. Delete a tuple
      
      Check the `GROUP BY` value in the materialized view (it should exist), 
      decrease the value, if it reaches 0, then remove the whole tuple from the
      view.

3. Avg

   > To allow updates, `Avg` is implemented using `Sum` and `Count`.

   1. insert 

      Just do the procedures of how to handle `Sum` and `Count`

   2. Delete

      Just do the procedures of how to handle `Sum` and `Count`

      > When `Sum` becomes 0, `Count` should become 0 as well, one can use any
      > of them to decide if the tuple should be removed from the view.

4. Min

   1. insert 

      Check the `GROUP BY` value, if it exists, then compre the new value with
      the stored min value, if the new value is smaller, then update the min
      value.

      If it does not exist, simlply add a new tuple.

   2. Delete

      If the delete tuple's value is not the minimum one, then we don't need to
      do anything. Otherwise, we have to iterate over all the values to see if
      the stored min value is still valid. 

      > The deletion can be expensive.

5. Max

   Same as the Min aggregation.

### 16.5.2.4 Other Operations

What are not covered by the above sections

1. outer join

   1. Left outer join (r left outer join s)
      
      1. insert i to r
         
         $ mv_{old} \cup (i \Join s) $ 

         > NOTE: the above $\Join$ is left outer join

      2. remove i from r
        
         $ mv_{old} \setminus (i \Join s) $

         > NOTE: the above $\Join$ is left outer join
         
      3. insert i to s

         1. Do inner join $ i \Join r $, and append it the view
         2. Traverse the view and find the tuples generated from unmatched data,
            check if they are still unmatched after insertion of $i$, if not, 
            remove the tuple

      4. remove i from s

         1. Do inner join $ i \Join j $, and remove it the view
         2. Traverse the tuples of r, check if they become unmatched due to the
            removal of i, if so, add an unmatched tuple to the view.
     
   2. Right outer join (r left outer join s)

      Mirror operation of the procedure introduced in the last section.

   3. Full outer join
      
      1. Insert i to r
         
         1. Do the left outer join, and append it to the view
         2. Traverse the unmatched data in s, check if it is still unmatched, 
            if so, do nothing, otherwise, remove it from the view.

      2. Remove i from r

         1. Do the left outer join, and remove it to the view
         2. Traverse the tuples of s, check if it is becomes unmatched, if so,
            add it to the view.

      3. Insert i to s

         Mirror operation of the precedure for `Insert i to r`.

      4. Remove i from s

         Mirror operation of the precedure for `Remove i from r`.
   
2. set operation

   1. Union 

      * UNION $ mv = r \cup s $
        
        1. Insert i to r 
           
           For every tuple in i, check if it already exists in the view, if not,
           add it.

        2. Remove i from r

           For every tuple from i, it should exist in the view, we cannot remove
           it from the view unless we are sure that it is the tuple in `r union
           all s`.

           To do so, we have to maintain an extra counter for every tuple in the
           view.

      * UNION ALL

        Handle it in the same way as the projection.
   
   2. Intersection $mv = r \cap s$

      1. Insert i to r
         
         For every tuple in i, chece if it exists in s, if so, and it does not 
         exist in the view, add it to the view.
         
      2. Remove i from r

         > We have to maintain 2 counter (one for r, one for s) for the tuples
         > stored in the view.

         For every tuple in i, if it exists in the view, decrease the counter for
         r by 1, if either counter becomes 0(for case "remove i from r", it should
         be the counter for r), remove it from the view.

   3. Set difference $mv = r \setminus s$

      1. Insert i to r
         
         For every tuple in i, if it does not exist in s and the view, add it 
         to the view.

      2. Remove i from r

         > We have to maintain a counter for the tuples that are shown in the 
         > view, recording their amount in r.
         
         For every tuple in i, if it exists in the view (the counter should be 
         at least 2), decrease the counter by 1, if the counter becomes 1, then
         remove it from the view.

### 16.5.2.5 Handling Expressions (Complex queries)

A complex query would be a tree, to maintain materialized view of complex queies,
we should start with the smallest subexpressions.

## 16.5.3 Query Optimization and Materialized Views

1. Sometimes, we can rewrite the query plan to use materialized view to speed up 
   query, e.g., assume we have a materialized view $ v = r \Join s $, a query like
   $ r \Join s \Join t $ can be written as $ v \Join t $ to speed it up.  

2. But sometimes, blindly using materialized views can make the performance worse,
   say we have a materialized view $ v = (r \Join s) $, and column $A$ is only 
   present in relation $r$, and an index is available on $A$, there is also an 
   index for common column $B$ so that we can use hash join. For a query like 
   $\sigma_{A=10} (v)$, the optimial plan will never be using the materialized
   view, instead, we should rewrite it to:

   $$ \sigma_{A=10} (r \Join s) \equiv $$
   $$ (\sigma_{A=10} (r) ) \Join s \equiv $$

3. It is up to the query optimizer to decide if a query should be optimized using
   materialized view.

## 16.5.4 Materialized View and Index Selection

1. Index is good, it can speed up query, but what fields should I build index 
   for? This problem is called index selection. Materialized view has a similar
   problem, materialized view selection.

2. Materialized view and index selection should be done according to the typical
   workload of the system. Modern DBMSes would provide tools to help adminisrator
   to make decisions.

# 16.6 Advanced Topics in Query Optimization
## 16.6.1 Top-K Optimization

For Top-K queries, **if `K` is quite small**, it is rather inefficient to sort all the
tuples, then take `K` values from it.

The typical approach for this is to use a priority queue with capacity set to `K`, 
insert every data pipelined from the bottom operators, after traversing all the 
data, the data remained in the priority queue would be the Top `K` data.

## 16.6.2 Join Minimization

Join minimization is for cases where:

1. A query would use a materialied view that is defined using join
2. The join operation is done over multiple tables
3. A query may not need all the tables joined in the materialized view

In such case, we can "drop" some relations from the join.

> QUES:
>
> I don't quite understand this, how can we drop a relation from the result of
> join operation.

## 16.6.3 Optimization of Updates

1. The Halloween problem

   > We have seen this in CMU 15-445 Lecture 12.

   When updating an ordered data structure (e.g., B+Tree), if the update is done
   while the selection is being evaluated by an scan, an updated tuple may be
   reinserted in the data structure ahead of the scan and seen **again** by the
   scan, and thus will be updated twice (or even more).

   This is called Halloween problem.

2. How to solve the Halloween problem

   1. One can do the update in 2 phases
      
      1. Selection, find out the tuples that satisfy the condition and collect
         their RecordIDs.

      2. Apply the update to the tuples specified by the collected RecordIDs.

   2. There are cases where this problem will never happen:

      1. If the update statement will update field `A`, and the ordered structure
         is sorted on field `B`, then this problem will never happen.

         > Well, that's true

      2. Even though the update statement and the ordered structure will all use
         the same column, if the update will decrease the value, and we are 
         scanning the table in an increasing order, then this problem won't happen.

3. Other optimizations that can be made to update

   1. Do it in a batch
   2. When updating the underlying sorted structure, e.g., B+Tree, sort the modifications,
      then do them sequentially to avoid random I/O.

      Update a B+Tree would:

      1. Remove the old value from it
      2. Insert the new value to it

      One should do the above 2 steps sequentially.

## 16.6.4 Multiquery Optimization and Shared Scans

1. What is multiquery optimization

   When a batch of queries are submitted together, those queries would typically
   share something in common, so theoretically the query optimizer can reuse the
   things that they share to optimize them.

2. Common multiquery optimization

   1. Common subexpression elimination
     
      > This is [a wildly applied optimization in programming language compilers][link].
      >
      > [link]: https://en.wikipedia.org/wiki/Common_subexpression_elimination

      For those common sub-expressions, we would compute them once and get them
      stored so that we can reuse them later.

   2. Shared scan
      
      When multiple queries are done against the same relation, we can read it once
      and pipeline the tuples to all the above operators.

## 16.6.5 Parametric Query Optimization

QUES: Don't quite understand what this is.

## 16.6.6 Adaptive Query Processing

Adjusts the query plan during execution time.

# 16.7 Summary

I would like to quote a comment from the Andrew Lamb:

> https://github.com/apache/datafusion/issues/1972#issuecomment-1156308944


I would second your assertion that almost **all successful real world (e.g. 
commerical) query optimizers are not implemented with a cascades like framework**,
but instead are **some combination of heuristics and cost models**.

I also think the point that cost models have unsolved error propagation issues,
my experience was that after about 2-3 joins, the output cardinality estimation
is basically a guess, even with advanced statistics like histograms.

What I would like to see in DataFusion is:

* A solid "classic" heuristic optimizer as a default
* Sufficient extension points that anyone who wants to experiment / create / use
  a different optimizer strategy can easily do so.
