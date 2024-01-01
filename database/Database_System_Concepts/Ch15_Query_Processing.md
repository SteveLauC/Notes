> * 15.1 Overview
> * 15.2 Measures of Query Cost
> * 15.3 Selection Operation
> * 15.4 Sorting
> * 15.5 Join Operation
> * 15.6 Other Operations
> * 15.7 Evaluation of Expressions
> * 15.8 Query Processing in Memory
> * 15.9 Summary

# 15.1 Overview

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

# 15.2 Measures of Query Cost

1. There are multiple query plans for a query, we should choose the one
   that has the minimal cost.

   To do so, we have to estimate the cost of individual operation and combine
   them to get the cost of a query plan.

   > In the next 4 sections, we learn the cost of individual operations.

2. Metrics of query plan cost

   * Disk I/O (the dominating one)

     1. The # of block accesses
   
        For PCIe SSD, transferring a 4k block would take 2 microseconds
        
     2. The # of random I/O

        For PCIe SSD, a random acccess would take 20 to 60 miroseconds

        > time units:
        >
        > 1. second
        > 2. milisecond
        > 3. microsecond
        > 4. nanosecond

     > Reading from memory will happen in units of `cache line` rather than
     > disk block, reading a memory chunk of 4k would take less than 1 us,
     > seeking (latency) would take less hten 100 nanoseconds.

     3. read and write

        On PCIe flash, reading throughput is about twice big than the write one,
        but the difference is almost completely masked by the limited speed of
        SATA interfaces, leading to write throughput matching read throughput.

   * CPU time

     PostgreSQL has some built-in default values for:

     > And these values can be changed via configuration.

     1. The CPU cost per tuple
     2. The CPU cost for processing each index entry
     3. The CPU ost per operator or function

   * Communication cost (for parallel and distributed system)

3. The costs of disk I/O also depends on the main memory size, for example, if
   a big memory that can hold all the data is avilable, then we can read the 
   data from disk, then we no longer need to access disk again.

   When estimating cost, we use the amount of memory available to an operator,
   M, as a parameter.

   > QUES: what is an operator?

   > In PostgreSQL, the total memory available to a query, called the effective
   > cache size, is assumed by default to be 4 gigabytes.

4. Normally, the cost of a disk I/O is smaller than the estimated one due to the
   existence of kernel cache buffer.

   To take it into account, PostgreSQL uses the following hack:

   1. The cost of a random access is assumed to be 1/10th of the actual cost, to
      model the situation that 90% of reads are found to be resident in cache.

   2. For B+Tree nodes access, it assumes that all the internal nodes are in the
      cache, making only accesses to leaf nodes incur random I/O.

5. If a system has multiple disks, and the data are distributed amoung them, the
   performance can be better if we can read them at the same time.

6. `Response time` is the reflection of the above metrics, but `response time`
   is hard to predict, depending on a variety of factors:

   * templature
   * the # of processes
   * cpu frequency
   * ...

   So, query plan optimization is to optimize the resource consumption talked
   above rather than the `response time`.

   > This is pretty silimar to algorithm optimization, we don't optimize the
   > time consumption, but the number of cycle, CPU instructions...

# 15.3 Selection Operation


# 15.4 Sorting
# 15.5 Join Operation
# 15.6 Other Operations
# 15.7 Evaluation of Expressions

> In this section, we examine how to coordinate the execution of multiple 
> operations in a query evaluation plan, in particular, how to use pipelined
> operations to avoid writing intermediate results to disk.

# 15.8 Query Processing in Memory
# 15.9 Summary
