> 15.1 Overview
> 15.2 Measures of Query Cost
> 15.3 Selection Operation
> 15.4 Sorting
> 15.5 Join Operation
> 15.6 Other Operations
> 15.7 Evaluation of Expressions
> 15.8 Query Processing in Memory
> 15.9 Summary

* 15.1 Overview

1. Steps of processing a query:

   1. Parsing and translation

      1. Parse SQL to its AST
      2. translate the AST to a relational-algebra expression

         > This relational-algebra expression is roughly the logical plan

   2. Optimization
   3. Evaluation

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-01-01%2012-54-20.png)

2. For a query, there are generally a variety of methods to process it:

   1. A query can be expressed with several SQL statements
   2. A SQL can be translated into multiple relational-algebra expressions
   3. A relational-algebra expression **ONLY PARTIALLY** describes how to evaluate 
      a query

      To fully specify how to evaluate a query, we need to annotate a relational-
      algebra expression with instructions on how to evaluate it.

3. What are `evaluation primitive` and `query-execution plan`

   A relational-algebra expression annotated with instructions on how to
   evaluate it is called an `evaluation primitive`.

   A sequence of `evaluation primitive`s that can be used to evaluate a query
   is called `query-execution plan`.

   > I think I will just call it `query plan` in later notes.

   For  SQL:

   ```sql
   SELECT salary FROM instrcutors WHERE salary < 75000;
   ```

   Here is one `query-execution plan`, assume that we have an index on column
   `salary`:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-01-01%2014-38-05.png)

4. Not all databases exactly follow the steps we described above, for instance,
   instead of using the relational-algebra representation, several databases use
   an annotated parse-tree representation based on the structure of the given
   SQL query.

5. For a query optimizer, in order to optimize a query plan, it must know the
   cost of each operation. It is hard to get a exact cost but getting a rough
   estimate of execution cost is possible.

* 15.2 Measures of Query Cost

1. There are multiple query-execution plan for a query, we should choose the one
   that has the minimal cost.

   To do so, we have to estimate the cost of individual operation and combine
   them to get the cost of a query

* 15.3 Selection Operation
* 15.4 Sorting
* 15.5 Join Operation
* 15.6 Other Operations
* 15.7 Evaluation of Expressions

> In this section, we examine how to coordinate the execution of multiple 
> operations in a query evaluation plan, in particular, how to use pipelined
> operations to avoid writing intermediate results to disk.

* 15.8 Query Processing in Memory
* 15.9 Summary
