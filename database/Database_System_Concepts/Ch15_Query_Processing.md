> * 15.1 Overview
> * 15.2 Measures of Query Cost
> * 15.3 Selection Operation
>   * 15.3.1 Selections Using File Scans and Indices
>   * 15.3.2 Selections Involving Comparisons
>   * 15.3.3 Implementation of Complex Selections
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

      To fully specify how to evaluate a query, we need to annotate the relational
      algebra expression with instructions on how to evaluate it.

3. What are `evaluation primitive` (node) and `query-execution plan` (tree)

   A relational-algebra expression annotated with instructions on how to
   evaluate it is called an `evaluation primitive`.

   A sequence of `evaluation primitive`s that can be used to evaluate a query
   is called `query-execution plan`.

   > I think I will just call it `query plan` in later notes.

   For SQL:

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

2. Metrics on query plan cost

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

     > Reading from memory happens in units of `cache line` rather than
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
     3. The CPU cost per operator or function

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

   To take it into account, PostgreSQL uses the following assumption:

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

> This is the `selection` in SQL but not the one in relational-algebra.

1. In query processing, the `file scan` is the lowest-level operator to access 
   data. File scans are search algorithms that locate and retrieve records that
   fulfill a selection condition.


   ```sql
   steve=> EXPLAIN SELECT * FROM students WHERE age > 18;
                            QUERY PLAN
   ------------------------------------------------------------
    Seq Scan on students  (cost=0.00..25.88 rows=423 width=36)
      Filter: (age > 18)
   (2 rows)
   ```

   Well, the `EXPLAIN` command will give you a estimated execution cost.

2. `file scan` is the operator that reads the table, `index scan` will use the
   index to read the table.

## 15.3.1 Selections Using File Scans and Index Scans

> Denotations:
>
> * Ts: time of a seek operation
> * Tt: time of transferring a block
> * Nb: the number of blocks to access
> * Hi: height of the index
> * Ne: the number of entries to read (used in case 6)

> NOTE: the text book assumes that every access to an internal node of a B+Tree
> is a random access, you know that most DBMSes don't think so, they believe that
> internal nodes are in memory since they are frequently accessed.

* Case 1: linear search

  Slow, but **will work under ANY cases**, no matter:
   
  * the file type
  * availability of index

  The other approaches are not appliable in all cases.

  To linear search a B+Tree that resides on disk, you have to seek to the root
  node, then follow the pointers stored in internal nodes to arrive at the 
  leftmost leaf node, then do the scan. It is highly possible that leaf nodes
  are not continuous on disk, so more random accesses.

  Cost: If we ignore the random accesses causes by the fact that leaf nodes are
  not continuous on disk, the cost is `Ts + Nb * Tt`.

* Case 2: Linear search, equality on key
 
  > What does "equality on key" mean?
  > 
  > 1. equality means that we want to find tuples where a specific field equals
  >    to some value.
  > 2. `key` means that the values of this field are unique 

  This is similar to linear search except that we can stop the search if the 
  target entry is found.

  Average Cost: `Ts + (Nb/2)*Tt`.

* Case 3: Clustering B+Tree Index, Equality on Key

  > Clustering index requires that the database file is also sorted according
  > to the search key.

  If the clustering index is dense, then the target key is guaranteed to exist
  in the index. Then we:

  1. Read the index: `Hi * (Ts+Tt)`
  2. Following the index pointer to read the entry: `Ts + Tt`

  Cost: `(Hi+1) * (Ts+Tt)`

  If the clustering index is sparse, then if the target key exists in the index,
  then the cost is also `(Hi+1) * (Ts+Tt)`. if the target key does not exist, 
  
  1. then we need to find the key that is less than or equal to the target 
     key: `Hi * (Ts + Tt)`
  2. Read the data file page: `Ts + Tt`
  3. Then sequentially read the entries from this pgae until we find the target
     key: `Nb * Tt`

  Cost: `[(Hi + 1) * (Ts + Tt)] + Nb*Tt`

* Case 4: Clustering B+Tree index, equality on non-key

  > `non-key` means that the values of the target field are not unique

  Since the values of the target field are not unique, we need to read all the
  tuples. Assume reading the the data file needs to access `Nb` blocks, this 
  case needs more cost `(Nb-1) * Tt` than the previous one.

* Case 5: Secondary B+Tree index, equality on key

  > Secondary index must be dense 

  Cost: `(Hi+1) * (Ts + Tt)`

* Case 6: Secondary B+Tree index, equality on non-key

  > B+Tree index is ordered, since the secondary index does not have the same
  > order as the data file, then the data file is guaranteed to be unordered.

  Since the values of this field are not unique, the index entry stores a list
  of pointers pointing to all the tuples with the same search key value.
  first tuple satisfying our request.

  1. Read the index: `Hi * (Ts + Tt)`, it is a list of pointers
  2. Assume the pointers list is ordered, and these tuples store in different
     blocks, then it will be `Ne * (Ts + Tt)` (the worst case)

  Cost: `(Hi + Ne) * (Ts + Tt)`

  If `Ne` is large, then the cost can be pretty high.


## 15.3.2 Selections Involving Comparisons

* Case 7: Clustering index, comparsion

  Say that this index is a clustering B+Tree index, for query:

  ```sql
  SELECT * FROM table WHERE col > A ( >= A)
  ```

  We can traverse down the leaf node that has values >= A, then we follow the
  pointer to the the data storage, find that first tuple that satisfies the
  condition, since this is a clustering index, the data is also ordered, we can
  simply sequentially scan the database file.

  Cost: `Hi * (Ts + Tt) + Ts + (Nb - 1) * Tt`

  For query:

  ```sql
  SELECT * FROM table WHERE col < A ( <= A)
  ```
  We don't need to read the index as we can directly scan the database file
  until we find the first tuple that satisfies the cndition.

  Cost: `Ts + (Nb - 1) * Tt`

  > The above analysis assumes that the clustering index is dense, for a sparse
  > clustering index, we may need to read more blocks.

* Case 8: Secondary index, comparsion

  > Secondary index must be dense

  Secondary index stores pointers to the values satisfying the condition,
  but following a pointer would require a seek + block tranfer, which can
  be quite expensive.

  So it should be ONLY used if:
  1. We are aware of the number of matching tuples
  2. The number is small

  > It might be better to sort the `PageID`s that we got from this index, 
  > which could possibly avoid some random I/O and duplicate reads.
  >
  > PostgreSQL does this, if the number of matching tuples if not known,
  > the query optimizer would create a BitMap with a capacity of the number
  > of blocks used by this table. Then it will traverse the index, if a
  > page is stored in an index entry, the corresponding bit will be set
  > to 1, after index traverse, PostgreSQL will read the pages that having
  > bit set to 1.
  >
  > ```rs
  > let mut map = RoaringBitMap::new();
  > for page_id in secondary_index {
  >     map.insert(page_id);
  > }
  >
  > // access pages whose ID is stored in `map`
  > ```
  >
  > This is called bitmap index scan.
  >
  > Using a bitmap so that we don't need to sort the list, which is cheaper.


## 15.3.3 Implementation of Complex Selections

1. We have covered some selections with simple predicates (column op value), 
   however, the filter can be complex.

2. What are `conjunctive selection` and `disjunctive selection`?

   * A `conjunctive selection` is a selection of the form:

     ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-01-04%2020-27-59.png)

     It is basically SQL queries like:

     ```sql
     SELECT * FROM table WHERE {column} {op} {value} AND {column} {op} {value} AND {column} {op} {value}
     ```

   * A `disjunctive selection` is a selection of the form:

     ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-01-04%2020-28-25.png)
   
     ```sql
     SELECT * FROM table WHERE {column} {op} {value} OR {column} {op} {value} OR {column} {op} {value}
     ```
 3. Cases 

    * Conjunctive selection using one index

       

# 15.4 Sorting
# 15.5 Join Operation
# 15.6 Other Operations
# 15.7 Evaluation of Expressions

> In this section, we examine how to coordinate the execution of multiple 
> operations in a query evaluation plan, in particular, how to use pipelined
> operations to avoid writing intermediate results to disk.

# 15.8 Query Processing in Memory
# 15.9 Summary
