> This chaper covers how to do:
>
> 1. Full file scan
> 2. Selections with equality condition
> 3. Selections with comparsion
> 4. Selections with complex conditions (conjunction/disjunction)
> 5. Sort

> * 15.1 Overview
> * 15.2 Measures of Query Cost
> * 15.3 Selection Operation
>   * 15.3.1 Equality/full file Selections Using File Scans and Indices
>   * 15.3.2 Selections Involving Comparisons
>   * 15.3.3 Implementation of Complex Selections
> * 15.4 Sorting
>   * 15.4.1 External Sort-Merge Algorithm
>   * 15.4.2 Cost Analysis of External Sort-Merge
> * 15.5 Join Operation
>   * 15.5.1 Netsted-Loop Join 
>   * 15.5.2 Block Netsted-Loop Join 
>   * 15.5.3 Indexed Nested-Loop Join
>   * 15.5.4 Merge Join
>     * 15.5.4.1 Merge Join Algorithm
>     * 15.5.4.2 Cost Analysis
>     * 15.5.4.3 Hyprid Merge Join
>   * 15.5.5 Hash Join
>     * 15.5.5.1 Basics
>     * 15.5.5.2 Recursive Partitioning
>     * 15.5.5.3 Handling of Overflows
>     * 15.5.5.4 Cost of Hash Join
>     * 15.5.5.5 Hybrid Hash Join
>   * 15.5.6 Complex Joins
>   * 15.5.7 Joins over Spatial Data
> * 15.6 Other Operations
>   * 15.6.1 Duplicate Elimination
>   * 15.6.2 Projection
>   * 15.6.3 Set operations
>   * 15.6.4 Outer Join
>   * 15.6.5 Aggregation
> * 15.7 Evaluation of Expressions
>   * 15.7.1 Materialization
>   * 15.7.2 Pipelining
>     * 15.7.2.1 Implementation of Pipelining
>     * 15.7.2.2 Evaluation Algorithms for Pipelining
>   * 15.7.3 Pipelines for Continuous-Stream Data
> * 15.8 Query Processing in Memory
>   * 15.8.1 Cache-Conscious Algorithms
>   * 15.8.2 Query Compilation
>   * 15.8.3 Column-Oriented Storage
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
   3. A relational-algebra expression (logical plan) **ONLY PARTIALLY** describes
      how to evaluate a query

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

   > This is cost-based optimization.
   >
   > There are generally 2 kinds of optimizers:
   >
   > 1. Rule-based (RBO)
   > 2. Cost-based (CBO)

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
     
     1. Serialization and deserialization

3. The costs of disk I/O also depends on the main memory size, for example, if
   a big memory that can hold all the data is avilable, then we can read the 
   data from memory, then we no longer need to access disk again.

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

> This selection covers how to do selections under different conditions and their
> estimated cost (cost made by disk I/O).

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

   Well, the `EXPLAIN` command will give you an estimated execution cost.

2. `file scan` is the operator that reads the table, `index scan` will use the
   index to read the table.

   > Selection predicate guides us to choose the right index to use.

## 15.3.1 Equality/full file selections Using File Scans and Index Scans

> Denotations:
>
> * Ts: time of a seek operation
> * Tt: time of transferring a block
> * Nb: the number of blocks to access
> * Hi: height of the index
> * Ntuple: the number of tuples to read (used in case 6)

> NOTE: the text book assumes that every access to an internal node of a B+Tree
> is a random access, this is true if they are not present in the cache, and 
> you know that most DBMSes don't think so, they believe that internal nodes are
> in memory since they are frequently accessed.

* Case 1: linear search

  Slow, but **will work under ANY cases**, no matter:
   
  * the file type
  * availability of index

  The other approaches are not appliable in all cases.

  To linear search a B+Tree that resides on disk, you have to seek to the root
  node, then follow the pointers stored in internal nodes to arrive at the 
  leftmost leaf node, then do the scan. It is highly possible that leaf nodes
  are not continuous on disk, so more random accesses.

  > If this is a B-Tree, you can simply read all the pages of this file without
  > accessing the pointers stored in internal nodes.

  Cost: If we ignore the random accesses causes by the fact that leaf nodes are
  not continuous on disk, the cost is `Hi * (Ts + Tt) + Ts + Nb * Tt`.

  > QUES: Are leaf nodes continuous on disk? I think this depends on implementation.

* Case 2: Linear search, equality on key
 
  > What does "equality on key" mean?
  > 
  > 1. equality means that we want to find tuples where a specific field equals
  >    to some value.
  > 2. `key` means that the values of this field are unique 

  This is similar to linear search except that we can stop the search if the 
  target entry is found.

  Average Cost: we need to access `Nb/2` blocks under the average case `Ts + (Nb/2)*Tt`.

* Case 3: Clustering B+Tree Index, Equality on Key

  > Clustering index requires that the database file is also sorted according
  > to the search key.

  If the clustering index is dense, then the target key is guaranteed to exist
  in the index. Then we:

  1. Read the index: `Hi * (Ts + Tt)`
  2. Following the index pointer to read the entry: `Ts + Tt`

  Cost: `(Hi+1) * (Ts+Tt)`

  If the clustering index is sparse, then if the target key exists in the index,
  then the cost is also `(Hi+1) * (Ts+Tt)`. if the target key does not exist, 
  
  1. then we need to find the key that is less than or equal to the target 
     key: `Hi * (Ts + Tt)`
  2. Read the data file page: `Ts + Tt`
  3. Then sequentially read the entries from this pgae until we find the target
     key, assume we will find the target key after reading `Nb` pages: `Nb * Tt`

  Cost: `[(Hi + 1) * (Ts + Tt)] + Nb*Tt`

* Case 4: Clustering B+Tree index, equality on non-key

  > `non-key` means that the values of the target field are not unique

  Since the values of the target field are not unique, and this is a clustering
  index, then the pointer stored in index points to the first tuple that has a 
  field of that value. We need to read all the tuples. Assume reading the the 
  data file needs to access `Nb` blocks, this case needs more cost `(Nb-1) * Tt` 
  than the previous one in both cases.

* Case 5: Secondary B+Tree index, equality on key

  > Secondary index must be dense 

  Cost: `(Hi+1) * (Ts + Tt)`

* Case 6: Secondary B+Tree index, equality on non-key

  > B+Tree index is ordered, since the secondary index does not have the same
  > order as the data file, then the data file is guaranteed to be unordered.
  >
  > Future steve: WTF? the data file can be ordered, it can be ordered by a key
  > that is different from the one the index is built against.

  Since the values of this field are not unique, and this is a secondary index,
  then the index entry stores a list of pointers pointing to all the tuples 
  with the same search key value.

  1. Read the index: `Hi * (Ts + Tt)`, it is a list of pointers
  2. Assume the tuples pointed by these pointers are storesd in different pages, 
     then we need to access `Ntuple` pages, in the worst case the cost will be 
     `Ntuple * (Ts + Tt)`

  Cost: `(Hi + Ntuple) * (Ts + Tt)`

  > If `Ntuple` is large, then the cost can be pretty high.


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

  Cost: `Hi * (Ts + Tt) + Ts + Nb * Tt`

  For query:

  ```sql
  SELECT * FROM table WHERE col < A ( <= A)
  ```
  
  Since there is a clustering index on field `col`, then the database file itself
  is ordered by `col` so that We don't need to read the index as we can directly 
  scan the database file until we find the first tuple that satisfies the cndition.

  Cost: `Ts + Nb * Tt`

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

1. We have covered some selections with simple predicates (`column op value`), 
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

    * Case 9: Conjunctive selection using one index

      1. For all the conditions of a conjunctive selection, if any one of it 
         whose field has an index, then we can retrieve the records satisfying
         the condition through that index

         > The records satisfying all the conditions is guaranteed to be a subset
         > of the records retrieved from this index.

      2. Then for the records, we check if it satisfies the other conditions.
         
      Cost: it depends on the index you choose
    
    * Case 10: Conjunctive selection using a composite index

      If the conjunctive selection specifies an equality condition on two or more
      attributes, and an **appropriate** composite index exisets on these combined
      fields, then the index can be used.

    * Case 11: Conjunctive selection by intersection of identifiers

      > Identifier means the pointer stored in index.

      If multiple fields of the conjunctive selection have indexes, then we can
      retrieve all the pointers stored by these indexes, and intersect them, the
      records stored in the pages pointed by these pointers should satisfy all
      the conditions.

      If these are conditions that whose fields do not have an index, we can iterate
      over the records to see if the remaining conditions are met.

    * Case 12: disjunctive selection by union of identifiers

      If all the fields used by a disjunctive selection have an index, then we can
      union the identifiers stored by all the indexes. 

      However, if tere is any field that does not have an index, then we have to
      do a linear search.

# 15.4 Sorting

1. Sorting is important as
 
   1. SQL supports specifying the order with the `ORDER BY` clause
   2. Several relational operations, such as joins, can be implemented efficiently
      if the input relatsions are first sorted.

      > This is the reason why we cover sort beforing diving into joins.

2. Sort records logically

   To sort reocrds logically according to a field, we can build an ordered index
   on that field, then accessing the records through that index is ordered.

   > This will be a non-clustering index.

   But since reocrds are ONLY logically ordered but not physically sorted, it 
   could result in massive random I/Os. 

   > We want records to be physically ordered.

3. If the data can fit entirely in the memory, then quick-sort can be used. In the
   next section, we discuss how to handle the case where data is too big to fit 
   into the memory.

## 15.4.1 External Sort-Merge Algorithm

1. Sorting relations that do not fit in memory is called `external sorting`, amoung
   which the most common one is `external sort-merge` algorithm.

2. External sort-merge algorithm

   Generally, external merge sort happens in 2 phases:

   1. Sort
   2. Merge

   > merge sort is a divide-and-conquer algorithm

   Let's dive into the details.

   > We assume that:
   >
   > * There are `M` blocks of memory that is available for sorting
   > * There are `Nb` blocks of data to sort
   > * `N = Nb/M`

   1. Repeatedly, until all blocks are read:
      1. read `M` blocks of data into memory
      2. sort these blocks
      3. write them to a disk file
         
      After this step, we will have `N` sorted chunks, every sorted chunk is 
      called a `run`, then we want to merge them.

   2. If `N + 1 <= M`, then read the first block of these `N` runs into memory,
      use 1 block to store the sorted result, let name this block as the destination
      block.

      1. Iterate over the first tuple of these `N` blocks, **move** the smallest
         tuple to the destination block.
      2. Repeat the last step until all the `N` blocks are empty.
      3. All these `N` blocks are sorted, write the sorted result to disk

      > With the in-memory merge sort, we will merge 2 sorted array, this is called
      > 2-way merge.
      >
      > ```rs
      > fn top_down_merge_sort<T: Copy + Ord>(a: &mut [T]) {
      >    // already ordered
      >    if a.len() <= 1 {
      >        return;
      >    }
      >
      >    let mid: usize = a.len() / 2;
      >
      >    merge_sort(&mut a[..mid]);
      >    merge_sort(&mut a[mid..]);
      >    // merge 2 sorted slices, 2-way merge
      >    merge(a, mid);
      > }
      > ```
      >
      > The above step merges `N` sorted arrays, this is called N-way merge.

      4. Repeat the last 3 steps until all these `N` runs are empty.

   3. If `N + 1 > M`, then we merge `M - 1` files first, this will be the input
      of the next merge, then we merge anther `M - 1` files until all files are
      processed.

      > Every round is called a `pass`.

      > TODO: figure out the approach that has the best performance.
      >
      > The approach described above is one strategy we can use, different
      > strategies have performance impacts, need to figure out the best way
      > to do it.

      At this point, the number of files have been reduced by a factor of `M-1`,
      then we do another round on these files, until we merge all the files into
      one file.

      > The last round, the number of input files will be `< M`.

3. diagram

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-01-07%2017-40-03.png)

   In the example depicted by the above diagram, we assume that 3 blocks of memory
   are available for sorting, and one block would contain 1 record.

   1. First, we repeatedly read 3 blocks, and sort them.
   2. After sorting, we have 4 files, so we cannot merge them all in a single 
      round, so in this pass, we repeated merge 2 files.
   3. After the last round, we have 2 files left to merge, 2 < 3, so this is
      the last round, then we merge them.

4. Merge strategy

   The text book seems to recommand that every merge in a round should be 
   balanced, "balanced" means that all the input files should try to have the
   same number of blocks.

   > QUES: I am not sure about this

## 15.4.2 Cost Analysis of External Sort-Merge

> NOTE: revisit ths in the future.

Cost:

1. The # of block transfer

   > Assume we are using the merge strategy demostracted in the above diagram.

   1. The first stage, sorting, we read `Nb` blocks, sort them, and write them 
      back, which would give us `2Nb` blocks transfers.

   2. We have `Nb/M` runs after the sorting phrase, every pass would decrease the
      number of runs by a factor of `M-1`, so that the # of pass will be:

      $$ \log _{M-1} (Nb/M) $$

   3. So the # of block transfer is:

      $$ \left [ \left ( 1 + \log _{M-1} (Nb/M) \right ) \right ] * 2Nb $$

2. The # of seeks

# 15.5 Join Operation

1. If we categorize joins by how they handle **unmatched** data, joins can be 
   separated into 4 kinds:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-20%2019-25-36.png)

   > This above diagram is great, the result of an inner join will be a subset 
   > of the result of the corresponding left/right/full join.

   > If you forget about the differences between them, 
   > see [What's the difference between INNER JOIN, LEFT JOIN, RIGHT JOIN and FULL JOIN?][so]
   >
   > so: https://stackoverflow.com/q/5706437/14092446

   > TODO
   >
   > 1. left semi
   > 2. right semi
   > 3. left anti
   > 4. right anti

2. In relational algebra, we have 3 kinds of joins:

   1. theta (θ, the eighth letter of the Greek alphabet) join
      
      This is the join cencept we use in the SQL world, `θ` denotes the condition.

      > If you come from the SQL world, you want to just call it join rather than
      > giving it a weird name.
      >
      > So I won't say it is a real category.
     
   2. equi join

      For a (theta) join, if the condition expression is `=`, then it is an equi
      join.

   3. natural join

      natural join is a special kinds of join, the fields (on different tables) 
      it uses should have the same name.

   > All the above joins, equi join and natural join, can be 
   > inner/left outer/right outer/full outer.

   4. non-equi join

## 15.5.1 Netsted-Loop Join 

1. Remeber that for selection operations, linear file scan will work under whatever
   circumstances.

   Like the linear file scan, this Nested-Loop Join alrogithm will work regardless
   of what the join condition is. And it does not require an index, as if there is
   no index, we can do linear file-scan. (it can be speeded up if there is an index
   appliable to the join condition).

2. Algorithm procedure

   > Algorithms will be expressed in Rust pseduo-code
   >
   > ```rs
   > /// Return true if `cond` is satisfied by `lhs` and `rhs`
   > fn condition(lhs: Tuple, rhs: Tuple, condi: Expr) -> bool;
   >
   > /// Join tuples `lhs` and `rhs`.
   > fn join(lgs: Tuple, rhs: Tuple) -> Tuple;
   > ```

   ```rs
   let mut result: Vec<Tuple> = Vec::new();

   for outer_tuple in outer {
       for inner_tuple in realtion inner {
           let satisfied: bool = condition(outer_tuple, inner_tuple, cond);
           if satisfied {
               result.push(join(outer_tupel, inner_tuple))             
           } 
       }
   }
   ```

   You can see that it is basicaly a nested-loop, the pseudo-code assumes there are
   2 relations to be operated on, increase the # of layers if more relations are
   added.

3. If we denote the # of tuples in a relation with `n_tuple(relation)`, and the
   \# of blocks with `n_block(relation)`

   With this algorithm, the inner relation will be accessed `n_tuple(outer)` times,
   and 1 time of access to the outer relation. 

   If the memory is sufficient to hold all the pages of these 2 relations, then we
   can load and access them in memory, which means the # of block transfer will be
   `n_block(outer) + n_block(inner)`. This is the best case.

   If there is ONLY one block for each relation, then every time we access a page,
   we have to load it from disk and evict the previous one, leading one block 
   transfer, then the # of block accesses will be the # of block transfer, i.e.,
   `n_tuple(outer) * n_block(inner) + n_block(outer)`, this is the worst case.

   You may find that the inner relation will be accessed multiple times, it will
   be benefical if we make the smaller relation the inner one. And if the inner
   relation can fit in the memory, then the algorithm will also need only 
   `n_block(outer) + n_block(inner)` block transfers, same as the case where both
   relations can fit in the memory.

   > For relations that will be accessed for multiple times, you want to pin them
   > in memory as much as you can to avoid redundant/duplicate disk reads.

   For seeks, if both relations can fit in the memory, then it requires 2 seeks.
   In the worst case, every access of the inner relation requires a seek, i.e., 
   `n_tuple(outer)` seeks, if we are on a HDD where only one read/write can be 
   handled at a time, then every block access of `outer` requires a seek, so 
   the # of seeks will be `n_tuple(outer) + n_block(outer)`.

## 15.5.2 Block Netsted-Loop Join 

1. The # of block accesses of nested-loop join is:

   `n_tuple(outer) * n_block(inner) + n_block(outer)`

   WITH ONLY ONE BLOCK OF MEMORY FOR EACH RELATION, the # of block transfers for
   the inner relation will be `n_tuple(outer) * n_block(inner)`, this can be
   improved if we **process the outer relation on a per-block basis**.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-24%2020-06-03.png)

   > In the above diagram, the arrow indicates the process of the accessing.

   With nested-loop join, we will remove the first block of the inner relation 
   and load the second block, this wasted the first block because it is still
   needed in the future.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-24%2020-09-55.png)

   With block nested-loop join, we don't kick the first block out, we do the 
   process of the `t2 of outer` and the tuples in the first block of inner.
   After doing this, processing of the first block of outer is complete, then
   we can drop the first block of inner and proceed to the second one..

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-24%2020-10-56.png)

   This will decrease our # of block transfers to 
   `n_block(outer) * n_block(inner) + n_block(outer)`.

2. Like nested-loop join, it can support any joins regardless of their join 
   conditions.

2. Algorithm procedure

   ```rs
   let mut result: Vec<Tuple> = Vec::new();

   for outer_block in outer {
       for inner_block in inner {
           for outer_tuple in outer_block {
               for inner_tuple_in inner_block {
                   let satisfied: bool = condition(outer_tuple, inner_tuple, cond);
                   if satisfied {
                       result.push(join(outer_tupel, inner_tuple))             
                   } 
               }
           }
       }
   }
   ```

3. With block nested-loop join, every block in the inner relation will be paired 
   with a block rather of a tuple of the outer relation.

   This will decrease the # of block transfers of the inner relation by the factor
   of `n_tuple(outer)/n_block(outer)` (in the worst case), leading to 
   `n_block(outer) * n_block(inner) + n_block(outer)` block transfers.

   > It is more efficient to use the smaller relation as the outer relation to
   > decrease the last number.
   >
   > n_block(outer) * n_block(inner) + n_block(outer)`
   > n_block(inner) * n_block(outer) + n_block(inner)`

   And the # of seeks will be decreased to `n_block(outer) + n_block(outer)`.

   > This will only improve the performance under the worst case, for the best
   > case, everything remains the same as the nested-loop join.

4. Other optimizations that can be made to nested-loop join and block 
   nested-loop join:

   1. If the join attribute (column) in an equi-join in the inner relation is
      ordered, then the loop for the inner relation can terminate as soon as
      the last match is found.

   2. The block nested-loop join improves the performance of nested-loop join
      by processing at the basis of block. Then we can also improve the perf
      by using a bigger "block".

      Say we have `M` blocks available for the join operation, then we can read
      `M-2` blocks of the outer relation at a time so that the # of block transfers
      can be decreased to `ceiling(n_block(outer)/(M-2)) * n_block(inner) + n_block(outer)`.
      The # of seeks will be `ceiling(n_block(outer)/(M-2)) * 2` (in the worst 
      case)
   
   3. If an index of the attribute (column) of the inner relation is available,
      then the # of block transfers can be possibly decreased.

      ```rs
      for outer_block in outer {
          for inner_block in inner {
              for outer_tuple in outer_block {
                  for inner_tuple in find_tuple_through_index(column, column_value, outer) {
                      // do the join
                  }
              }
          }
      }
      ```

      > Section 15.5.3 covers this.

5. Block nested-loop join can be dominating if the join result is almost as large
   as the Cartesian product of 2 relations.

## 15.5.3 Indexed Nested-Loop Join

1. In a nested-loop join, if an index is available on the inner loop's join
   attribute (column), index lookups can replace file scans.

   For each tuple in the outer relation, the index is used to retrieve tuples
   from the inner relation where the join condition is satisfied, this is 
   called indexed nested-loop join, it can work with existing indexes, **as well
   as with TEMPORARY INDEXES created for the sole purposes of evaluating the
   join**.

   > I kinda think we can have indexed block nested-loop join.
   > 
   > Future steve: no, we cannot do this. For the inner relation, block nested-loop
   > works by doing a file scan, with indexed nested-loop join, we no longer
   > do it, using index makes it another story.

2. The index should support point-query (at least on 1 column if the condition 
   is a conjunction) because retrieve tuples from the inner relatiion satisfying
   the join conditions with a given tuple from the outer relation is essentially
   a selection on the outer relation.

   For example

   ```sql
   SELECT * FROM students INNER JOIN takes WHERE students.id = takes.id;
   ```

   Suppose we have a student tuple where `id` is 0, then we want to retrieve
   tuples from the inner relation that also have a `id` of value 0, i.e.,

   ```sql
   SELECT * FROM takes WHERE id = 0;
   ```

3. Cost analysis

   An index loopup on the inner relation has to be performed for every tuple 
   of the outer relation, let's denote the cost of an index loopup with `c`,
   then the cost will be `n_tuple(outer) * c`.

   > `c` varies from cases to cases, see section 15.3 for details.

   In the worst case, where there is ONLY 1 block for the outer relation

   > This will affect the number of seeks on the outer relation, though
   > the number of block transfers is not affected.

   > I think that the number of memory blocks for index lookup won't affect
   > its cost.

   `n_block(outer)` seeks and block transfers are needed for the outer relation,
   so the total cost would be `n_block(outer) * (Ts + Tt) + n_tuple(outer) * c`.

   It is generally more efficient to use the relation with fewer tuples as
   the outer relation as `n_tuple(relation)` normally is a huge number.

   The # of block transfers of indexed nested-loop join will be much less than
   the one in block nested-loop join, but using index would cause more seeks.

## 15.5.4 Merge Join

1. The merge-join algorithm (which is also called sort-merge-join) can be used to
   compute

   1. equi-join
   2. natural join

   > Nested-loop join and block nested-loop join support any joins regardless of
   > their join condition. Indexed nested-loop join can be used if an index can
   > be used on the join condition.
   >
   > QUES: I kinda think that some kinds of non-equi join can also be supported,
   > e.g.:
   >
   > ```sql
   > SELECT * FROM students JOIN takes ON students.score < takes.core;
   > ```

   The process of merge join is similar to the **merge stage** of external 
   sort-merge.

### 15.5.4.1 Merge Join Algorithm

1. Merge Join Process

   > NOTE: the following procedure assumes that the relatiosn are sorted in
   > ascending order.

   ```rs
   let mut result: Vec<Tuple> = Vec::new();

   let mut outer_tuple: Option<Tuple> = Some(/* the first tuple of the outer relation */)
   let mut inner_tuple: Option<Tuple> = Some(/* the first tuple of the inner relation */)

   'end: loop {
        if outer_tuple.is_none() || inner_tuple.is_none() {
            break 'end;
        }

        // collecting tuples whose `join column` fields have the same value as the first
        // inner tuple
        let mut tuples_of_inner_with_same_value = Vec::new(); 
        let columns_value = inner_tuple.columns_value(/* join columns */)
        tuples_of_inner_with_same_value.push(inner_tuple.clone());
        inner_tuple.move_to_next();
        'a: while let Some(inner) = inner_tuple {
            if innner.columns_value(/*join column*/) == columns_value {
                tuoles_of_inner_with_same_value.push(inner_tuple);

                inner_tuple.move_to_next();
            } else {
                break 'a;
            }
        }

        // find the first outer tuple whose `join column` values are gteq to `columns_value`
        'b: while let Some(outer) = outer_tuple {
            if outer.columns_value(/*join columns*/) >= columns_value {
                break 'b;
            }

            outer_tuple.move_to_next();
        }

        // do the join operation
        //
        // This process can exit in 2 ways:
        // 1. outer_tuple is None, there is no tuples in the outer relation, 
        //    end the merge join
        //     
        // 2. outer.columns_value(/*join coluns*/) > columns_value
        // 
        'c: while let Some(outer) = outer_tuple {
            if outer.columns_value(/*join columns*/) == columns_value {
                for inner in tuples_of_inner_tuple_with_same_value {
                    result.push(join(outer, inner));    
                }

                outer_tuple.move_to_next();
            } else {
                break 'c';
            }
        }
   }
   ```

2. The algorithm described in the last note requires that 
   `tuples_of_inner_with_same_value` can fit in the memory, this can be met in
   most cases.

   If it can not be met, then we can do a block nested-loop join for it.

   > QUES: how

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-27%2008-44-37.png)

   For example, we cannot load all the 2 blocks of the inner relation to the 
   memory, then we can do it as follows:

   ![1](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-27%2008-46-46.png)
   ![2](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-27%2008-47-24.png)
   ![3](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-27%2008-47-56.png)
   ![4](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-27%2008-48-31.png)

### 15.5.4.2 Cost Analysis

1. The good thing that merge join has is the tuples are sorted so that every tuple
   will **ONLY be read once**.

   With nested loop join, every tuple will be inner table will be read 
   `n_tuple(outer)` times.

   > Kinda reminds me of the bulk loading of B+Tree

2. The # of block transfer is `n_block(outer) + n_block(inner)` since all the 
   tuples will be only read once, so all the blocks will be only read once as
   well.

3. The # of seek depends on the available buffer size

   Assume that for each realtion, `Bm` memory pages are available, then the textbook
   says that it will be `ceil(n_block(outer)/Bm) + ceil(n_block(inner)/Bm)`

   > QUES: Honestly, I don't know why.
   >
   > I think it depends on the # of the blocks that `tuples_of_inner_with_same_value`
   > takes and how we work around the loading strategy if it cannnot fit in memory.


### 15.5.4.3 Hyprid Merge Join

1. Merge join requires that both relations are sorted, if there is only one 
   relation that is ordered, then the other relation is not but bas a secondary
   B+Tree index, we can still do a merge join with them.

   > Secondary index has to be dense.

   With merge join, for the innner table, we load the tuples that have the same
   value on the join columns, with hyprid merge join, assume it is the inner table
   that has the secondary B+Tree index, then we load all the pointers to the tuples
   that have the same value on the join column.

   Accessing tuples through secondary index can be costly, we then sort them 
   according to their addresses, then we load the tuples following these pointers,
   and do the merge join.

2. In the above case, there is only 1 realtion that is not sorted and has a 
   secondary index.

   What if both relations are not ordered and have a secondary B+Tree index on
   the join columns.

   For the outer relation, we can still traverse the B+Tree index and find the
   first index entry that the tuple it points to is greater than or equal to 
   `columns_value`, then if the value equqls to `columns_value`, then we keep
   traversing the B+Tree and collect the index entries, sort them by their 
   addresses, then load them from the disk and do the join operation.

## 15.5.5 Hash Join

1. Same as merge join, the hash join can be ONLY used for:

   1. equi-join
   2. natural-join

   Why: See below.

2. The hash join algorith introduced in the textbook is GRACE hash join, it is 
   named after the database machine from Japan.

### 15.5.5.1 Basics

> NOTE: The following notes are slightly different from the textbook, they mainly 
> come from this MySQL [post](https://dev.mysql.com/blog-archive/hash-join-in-mysql-8)
>
> The textbook introduces split (by hash) first

1. The typical hash join divides into 2 phases
  
   > This is called `Classic Hash Join` according to 
   > [Wikipedia](https://en.wikipedia.org/wiki/Hash_join).

   1. Build
     
      For the smaller relation, build an in-memory hash index for it

      > This relation is called the build relation, or the build input.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/build-phase-1.jpg)

   2. Probe

      > The other relation is called the probe relation.

      Iterate over the tuples from the probe relation, take the values of the join 
      columns, access the build tuples from the build relation through the hash index,
      check if this tuple has the same value as the probe tuple, if so, join them
      and return it to the client.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/probe-phase-1.jpg)

      > QUES: Isn't this a simple indexed nested-loop join

2. To build an in-memory hash index for the build relation, it is required that
   the build relation should fit in memory.

   > We should choose the smaller relation as the build relation.

   > We won't load the whole relation to memory in order to build an index for
   > it, but the index should exist in memory, and its size should at least be
   > the size of the build relation.

3. What if the build relation cannot fit in memory? Split!

   > This is called `Grace Hash Join`

   If the build relation is too big to fit in memory, then split both relations
   to multiple partitions using a hash algorithm, and do build-and-probe for
   every partition pair.

   > So a grace hash join has 3 phases
   > 
   > 1. Partition
   > 2. Build
   > 3. Probe

   We should be able to control the # of partitions, and in order to ensure that
   every partition can fit in memory, we may want more partitions.

   > Splitting the relations:
   >
   > 1. Reduces the size of data that needs to fit in memory
   > 2. If two tuples have the same values on the join columns, then they should
   >    be put in the same partition pair.
   >
   >    This is why hash join can only hanlde equi-join and natural join. 

   NOTE: The hash algorithms used in split and hash index building should be 
   different but both applied to the join columns.

4. Detailed algorithm

   1. For the tuples of outer relation, we hash them (against their join columns)
      to split them into different partitions (assume n partitions), and write 
      them to partitions disk files.

   2. For the tuples of the inner relation, do the same thing.

   > They may have different number of partitions, we need to use the bigger one
   > as `n_partition`. 

   3. For the partitions of the innner relation, build an in-memory hash index
      for it, then load the corresponding partition of the outer relation, and
      do an indexed nested-loop join.

      The hash algorithm used by this hash index needs to be different from the one
      we used for partition, though it should still be applied to the join columns.

   4. During the indexed nested-loop join, we still need to check if these 2 tuples
      have the same value on the join columns since:

      `hash(x) == hash(y) even when x != y`

5. To ensure that the max partition can fit in memory, we would like to have more
   partitions if possible.

   > More partitions, the size of the partition will be likely smaller.

   > QUES: how can we control the number of partitions
   >
   > Think how we build a hashmap with linear probe hashing, we choose the number
   > of buckets, then `hash(key)` to get a value, say it is `n`, then we calculate
   > the index of bucket to which this `key` will go with `n % # of bucket`.
   > 
   > The # of bucket is basically the number of partitions. 

The textbook says that:

   > THe value `nh` must be chosen to be large enough  such that, for each i, the
   > tuples in the partition `si` of the build relation, along with the hash index
   > on the partition, fit in memory. 

   I think:

   1. One cannot choose the value of `nh` but only the hash algorithm
   2. The hash index has to fit in memory, this is right, but is it necessary to
      ask the corresponding partition to fit in memory? 

      > QUES: this might be related to how an in-memory hash index is built, do you
      > need to load all the data into memory at once?

### 15.5.5.2 Recursive Partitioning

1. Quote the book

   > If the value of `nh` (the # of partitions) is greater than or equal to the 
   > number of blocks of memory, the relation cannot be partitioned in one pass,
   > **since there will not be enough buffer blocks**.

   The textbook assumes that we want to buffer while splitting relations, which
   is good since the # of random I/O will be reduced, but splitting can be done
   using ONLY 1 block if one don't care about the random I/O.

2. How recursive partitiioning work

   Let's say we want to split the relatiion into 8 partitions, but ONLY 3 memory
   blocks are available:

   1. The first pass, we split our relation into 3 partitions

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-03-26%20at%2020.43.49.png)

   2. Then 

   > No idea how this works


### 15.5.5.3 Handling of Overflows

> Revisit this in the future when I understand how recursive partitioning works.

### 15.5.5.4 Cost of Hash Join

1. When recursive partitioning is not needed:
 

   1. The # of block transfers

      1. Partition: `2 * (n_block(outer) + n_block(inner))`
      2. Build: `n_block(inner/build relation)`
      3. Probe: `n_block(outer)`

      > NOTE: for the partition phase, say a relation has `n` blocks, then splitting
      > it to multiple partitions would typically require writing more blocks when
      > persisting partitions to disk
      >
      > This is because the smallest unit of disk I/O is a block, and each partition
      > can have a partial tailing block, thus require more writes. 
      >
      > In the worst case, i.e., every partition has a partial block, `4 * the number of partitions`
      > will be required (half for read, half for write).

      So, `3 * (n_block(outer + n_block(inner)))` block transfers 

      > Plus `4 * n_partitions` if you take partital blocks into account, in 
      > worst case, in reality, `4 * n_partitions` will be quite small when
      > compared to `n_block(outer) + n_block(inner)`.
     
   2. The # of seeks

      1. Partition
   
         The textbook says:

         > Asssuming `bb` blocks are allocated for the input buffer and each 
         > output buffer, partitioning requires a total of 
         > `2 * (ceil(n_block(outer)/bb) + ceil(n_block(inner)/bb))`

         I don't understand why, I think it depends on

         1. The # of partitions
         2. Buffer or not while partitioning
         3. The # of blocks available
         
      2. Build: `n_partition` (only needed by the build relation)
      3. Phase: `n_partition` (only needed by the probe relation)

2. With recursive partitioning, revisit this in the future.

### 15.5.5.5 Hybrid Hash Join
## 15.5.6 Complex Joins

1. Nested loop-join and block nested loop-join can be used regardless of the 
   join conditions, so they can be used.

2. Conjunctions

   For results of:

   ```
   condition_a & condition_b & condition_c
   ```

   The result of join over the above condition is guaranteed to be a subset of 
   the result of join over `condition_a`.

   So while returning results of join over `condition_a` to the client, we can
   check and see if this result tuple satisfies `condition_b` and `condition_c` 

3. Disjunctions

   Join result of 

   ```
   condition_a & condition_b & condition_c
   ```

   is

   ```
   union(join_over(condition_a), join_over(condition_b), join_over(condition_c))
   ```

## 15.5.7 Joins over Spatial Data

1. Currently, the algorithm learned has no assumptions over the type of the join
   columns, however, it does assume that some arithmetic operations are supported:

   1. equi
   2. less then
   3. greater than

   Not all data types support the above operation, e.g., embedding and spatial
   data.

2. Can we use the join algorithms to do join on them:

   * nested-loop join

     Supported

   * block nested-loop join

     Supported

   * indexed nested-loop join

     For the join operation we are gonna do, if there is a type idnex having
     the operation supported, the indexed nested-loop join can be used.

   * merge join
     
     Probably not, if the underlying data type does not support comparsion.

   * classic hash join
   * grace hash join
   * hyprid hash join

     I think yes.

# 15.6 Other Operations
## 15.6.1 Duplicate Elimination (`DISTINCT`)

1. `DISTINCT` can be implemneted using either

    1. sort

       After sorting, the duplicatee tuples will be adjacent to each other.

       With external merge sort, duplicate entries can be removed when writing
       to disk.

    2. hash

       After hashing a tuple, we ONLY put it in the target bucket if a tuple
       with the same value does not exist in that bucket.

## 15.6.2 Projection
## 15.6.3 Set operations

1. Supported set operations

   1. Union

      1. UNION DISTINCT 
        
         > `DISTINCT` can be omitted, the behavior won't change.

      2. UNION ALL

   3. Intersection

      > Isn't this a simple merge join? 
      >
      > Future steve: they are kinda similar but different
      >
      > merge join only care about the join columns, different tuples with the
      > same join column are still different tuples. With set operations, they
      > will be seen as the same tuple and deduplicated.

   4. Set-difference

2. Implement them by sorting

   NOTE: all the implementations below require ONLY 1 scan of each array/relation.

   ```rs
   /// Assume `sorted_array` is an ordered array, append `item` to it if an 
   /// item with the same value hasn't been present.
   fn dedup_push(sorted_array: &mut Vec<i32>, item: i32) {
       if let Some(last) = sorted_array.last() {
           if item != *last {
               sorted_array.push(item);
           }
       } else {
           sorted_array.push(item);
       }
   }
   ```

   1. Union

      1. UNION DISTINCT
         
         ```rs
         /// Assume `lhs` and `rhs` are 2 ordered arrays, union them.
         ///
         /// It is basically an implementation of the merge process during merge sort with
         /// an extra deduplication step.
         fn union_distinct(lhs: &[i32], rhs: &[i32]) -> Vec<i32> {
             let mut ret = Vec::new();
             
             let mut lp = 0;
             let mut rp = 0;
             while lp < lhs.len() && rp < rhs.len() {
                 let item;
                 match lhs[lp].cmp(&rhs[rp]) {
                     Ordering::Less => {
                         item = lhs[lp];
                         lp += 1;
                     }
                     Ordering::Greater => {
                         item = rhs[rp];
                         rp += 1;
                     }
                     Ordering::Equal => {
                         item = lhs[lp];
                         lp += 1;
                         rp += 1;
                     }
                 }
                 
                 dedup_push(&mut ret, item);
             }
            
             while lp < lhs.len() {
                 let item = lhs[lp];
                 dedup_push(&mut ret, item);
                 lp += 1;
             }
            
             while rp < rhs.len() {
                 let item = rhs[rp];
                 dedup_push(&mut ret, item);
                 rp += 1;
             }

             ret
         }
         ```

      2. UNION ALL

         Since there is no need to duplicate, why not just concatenate them and print.

   2. Intersection

      ```rs
      fn intersect(lhs: &[i32], rhs: &[i32]) -> vec<i32> {
          let mut lp = 0;
          let mut rp = 0;
          let mut ret = vec::new();

          while lp < lhs.len() && rp < rhs.len() {
              match lhs[lp].cmp(&rhs[rp]) {
                  ordering::equal => {
                      let item = lhs[lp];
                      dedup_push(&mut ret, item);
                      
                      lp += 1;
                      rp += 1;
                      while lhs[lp] == item {
                          lp += 1;
                      }
                      while rhs[rp] == item {
                          rp += 1;
                      }
                  }
                  // we are trying to find 2 values that are equal, `lhs[lp]` is too
                  // small, let's check the next value.
                  ordering::less => {
                      lp += 1;
                  }
                  // we are trying to find 2 values that are equal, `rhs[rp]` is too
                  // small, let's check the next value.
                  ordering::greater => {
                      rp += 1;
                  }
              }
          }
          ret
      }
      ```

   3. Difference

      ```rs
      /// `lhs` - `rhs`
      fn difference(lhs: &[i32], rhs: &[i32]) -> Vec<i32> {
          let mut lp = 0;
          let mut rp = 0;
          let mut ret = Vec::new();

          'outer: loop {
              if lp == lhs.len() {
                  break 'outer;
              }
              let item = lhs[lp];
              lp += 1;
              'find_dup: while let Some(next) = lhs.get(lp) {
                  if *next == item {
                      lp += 1;
                  } else {
                      break 'find_dup;
                  }
              }

              'check_exist: loop {
                  match rhs[rp].cmp(&item) {
                      Ordering::Equal => {
                          let val = rhs[rp];
                          rp += 1;
                          'find_dup: while let Some(next) = rhs.get(rp) {
                              if *next == val {
                                  rp += 1;
                              } else {
                                  break 'find_dup;
                              }
                          }

                          continue 'outer;
                      }
                      Ordering::Less => {
                          rp += 1;
                      }
                      Ordering::Greater => {
                          // item does not exist in `rhs`
                          ret.push(item);
                          break 'check_exist;
                      }
                  }
              }
          }

          ret
      }
      ```

3. Implement them by hashing

   1. Union
      1. UNION DISTINCT

         1. Build an in-memory hash index on `lhs`
         2. Add the tuples in `rhs` to the hash index only if they are not alreay present
         3. Add the tuples in the hash index to the result

         ```rs
         pub fn union_distinct(lhs: &[i32], rhs: &[i32]) -> Vec<i32> {
             // build an in-memory hash index
             let mut hash_set = HashSet::new();
             hash_set.extend(lhs.iter());
             hash_set.extend(rhs.iter());
            
             hash_set.into_iter().collect()
         }
         ```
      2. UNION ALL

         Again, why not just concatenate and print them.
   
   2. Intersection

      1. Build an in-memory hash index for `lhs`
      2. For each tuple in `rhs`, probe the index, if the tuple is present in
         the index, add it to the result. 

      ```rs
      pub fn intersect(lhs: &[i32],rhs: &[i32]) -> Vec<i32> {
          // build an in-memory hash index for one relation
          let mut hash_set = HashSet::new();
          hash_set.extend(lhs.into_iter().copied());

          let mut ret = Vec::new();
            
          for item in rhs.into_iter() {
              if hash_set.contains(&item) {
                  ret.push(*item);
              }
          }
            
          ret
      }
      ```

   3. Difference

      1. Build an in-memory hash index for `rhs`
      2. For each tuple in `lhs`, probe the hash index, if the tuple is not present
         in the hash index, add it to result.

      ```rs
      pub fn difference(lhs: &[i32], rhs: &[i32]) -> Vec<i32> {
          // build an in-memory hash index for `rhs`
          let mut hash_set = HashSet::new();
          hash_set.extend(rhs.into_iter().copied());
          let mut ret = Vec::new();
          
          for item in lhs.into_iter() {
              if !hash_set.contains(&item) {
                  ret.push(*item);
              }
          }
          
          ret
      }
      ```

      1. Build an in-memory hash index for `lhs`
      2. For each tuple in `rhs`, probe the hash index, if the tuple is present
         in the hash index, delete it from the index.
      3. Add the remaining tuples in the hash index to the result

      ```rs
      pub fn difference(lhs: &[i32], rhs: &[i32]) -> Vec<i32> {
          let mut hash_set: HashSet<i32> = lhs.iter().copied().collect();
          for item in rhs {
              if hash_set.contains(item) {
                  hash_set.remove(item);
              }
          }
          
          hash_set.into_iter().collect()
      }
      ```

## 15.6.4 Outer Join

1. All the join algorithms/implementations we talked about are for inner joins, 
   i.e., they won't handle unmatched data.

2. There are generally 2 ways of how can we do outer join

   1. Do inner join, then add the unmatched data to the result

      Take `Left outer join` as an example:
         
         After computing the result of innner join, scan the outer relation again,
         find the tuples whose join column values do not exist in the inner join
         result, then add them to the result, the fields that come from the inner
         relation are filled with value NULL.

    2. Modify the join algorithms we have learned to add native support for outer
       join.

       1. Nested-loop join
          
          Nested-loop join can be easily modified to do left outer join, for the
          tuple in the outer relation that does not match any tuples in the inner
          relation, add it to the result.

          For right outer join and full outer join, it is hard to implement 
          with nested-loop join.

       2. Block nested-loop join
       3. Indexed nested-loop join
       4. Merge join
          
          1. I think the merge join algorithm described in the textbook can be
             easily extended to support right outer join:

             After collecting the tuples whose join column have the same value
             from the inner relation, let's call this value `n`, we will try to
             find the first tuple from the outer relation whose value on join 
             column are greater than or equal to `n`.

             If the value of the join column of first tuple we will access in 
             the outer relation is greater than `n`, then there is no matched 
             tuple for those inner tuples, add them to the result.

           2. However, the textbook says:

              > Merge join can be extended to compute the full outer join as 
              > follows: When the merge of the 2 relations is being done, 
              > tuples in either relation that do not match any tuple in the
              > other relatioin can be padded with nulls and written to the
              > output.

              I think, this is not the algorithm presented in section 15.5.4.1,
              it is more like the intersction operation described in 15.6.3.
              (correct me if I am wrong, future steve).

              > Future steve is coming!
              >
              > The above thought is indeed incorrect! The algorithm described
              > in 15.5.4.1 can be modifyed to support:
              > 
              > 1. left outer join
              > 2. right outer join
              > 3. full outer join

              One can change the part "find the first outer tuple whose `join 
              column` values are gteq to `columns_value`" to iterate over the
              outer relation, then compare the value of the join column and
              `columns_value`, then:

              ```rs
              match val.cmp(&columns_value) {
                  Ordering::Eq => {
                      /* join */
                  }  
                  Ordering::Less => {
                      // there is no mathced tuple for `val`, if this is a left/full
                      // outer join, add it to the result.
                  }
                  Ordering::Greater => {
                      // there is no mathced tuple for the inner tuples whose 
                      // value of the join column is `columns_value`, if this 
                      // is a right/full outer join, add them to the result.
                  }
              }
              ```

       5. Hash join

          It is quite straightforward to implement left outer join with hash
          join:

          1. Build an in-memory hash index for the inner relation
          2. For each tuple in the outer relation, probe it with that hash index,
             if it is present in the hash index, do the join with the tuples in
             the index, otherwise, pad it with nulls and add it to the result.

          Then what about right and full outer join? One common strategy used
          in DBMS is that store a sign bit for the every tuple in the hash index,
          recording whether it has been matched or not. After the inner join,
          scan the hash index, pad the tuples whose sign is not set with nulls
          and add them to the result.

## 15.6.5 Aggregation

Aggregation functions can be implemented on the fly, let's try something easy
frist:

```sql
SELECT MAX(score) FROM students;
```

```rs
macro_rules! tuple {
    ($name:expr, $score:expr, $gender:expr) => {
        Tuple {
            name: ($name).into(),
            score: $score,
            gender: ($gender).into()
        }
    };
}

#[derive(Debug)]
struct Tuple {
    name: String,
    score: usize,
    gender: String,
}

fn students() -> Vec<Tuple> {
    vec![
        tuple!("steve", 1, "male"),
        tuple!("fen", 2, "female"),
        tuple!("huhu", 4, "female"),
        tuple!("gaga", 0, "male"),
    ]
}

fn max_score(tuples: &[Tuple]) -> Option<usize> {
    let mut result = None;
    for tuple in tuples {
        if let Some(curr_max) = result {
            if tuple.score > curr_max {
                result = Some(tuple.score);
            }
        } else {
            result = Some(tuple.score);
        }
    }
    
    result
}
```

Without `GROUP BY`, it is basically equivalent, for a given array, find the largest
number in it.

Add the `GROUP BY` clause:

```sql
SELECT name, MAX(age) FROM students GROUP BY name;
```

```rs
fn max_score_group_by_gender(tuples: &[Tuple]) -> impl Iterator<Item = (&str, usize)> {
    let mut max_score_per_group: HashMap<&str, usize> = HashMap::new();
    // Alternatively, a `BTreeMap` can also be used.
    
    for tuple in tuples {
        match max_score_per_group.entry(&tuple.gender) {
            Entry::Occupied(mut exist) => {
                let curr_max = exist.get();
                
                if tuple.score > *curr_max {
                    *exist.get_mut() = tuple.score;
                }
            }
            Entry::Vacant(new) => {
                new.insert(tuple.score);
            }
        }
    }
    
    max_score_per_group.into_iter()
}

fn main() {
    for (gender, max_score) in max_score_group_by_gender(&students()) {
        println!("{} {}", gender, max_score);
    }
}
```

```sh
$ cargo r -q
female 4
male 1
```

With `GROUP BY` added, we need to distribute tuples into different groups by their
`GROUP BY` fields, which can be done by a `HashMap` or `BTreeMap`.


# 15.7 Evaluation of Expressions

> In this section, we examine how to coordinate the execution of multiple 
> operations in a query evaluation plan, in particular, how to use pipelined
> operations to avoid writing intermediate results to disk.

> Don't confuse the `Expressions` here with those SQL `Expr`s, the `Expressions`
> here is a Relational Algebra Expression containing multiple operations.

1. How can we hanlde an expression with multiple operations

   1. Materialization

      One obvious way is to do one operation at a time (finish it at once), in 
      an appropriate order, the result of an operation will be stored in a 
      temporary relation for subsequent use.

      The temporary result, unless it is small, must be written to disk.

      > Considering that the intermediate result has to be written to the disk,
      > this way is **not suitable for OLAP systems** (ok for OLTP), where the
      > intermedicate results are relatively big.
     
   2. Pipelining (Iterator model/volcano model)

      Materialization does one operation at a time, pipelining does several 
      operations simultaneously, the results of one operation will be passed
      to the next operation so that it does not need to be stored temporarily.

      > QUES: For those **blocking** operations, they won't produce any output unless
      > all the input data is handled, then I guess the iterator model will behave
      > same as materialization.
      >
      > Future steve: u r right

## 15.7.1 Materialization

```
SELECT name 
FROM department NATURAL JOIN instructor
WHERE department.building = 'Watson';
```

The above SQL will be represented in:

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-04-05%20at%2008.34.20.png)

From the lowest level to the top level, every node will be executed sequentially.

Cost analysis:

The cost generally contains 2 parts:

1. The cost of all operations
2. THe cost of writing the intermediate results to the disk

## 15.7.2 Pipelining

1. We improve query-evaluation efficiency by reducing the number of temporary 
   files that are produced. We achieve this reduction by combining several
   relational operations into a **pipeline** of operations.

2. Benefits 

   1. It eliminates the cost of reading and writing temporary relations 
   2. It can start generating result quickly

### 15.7.2.1 Implementation of Pipelining

1. The implementation of pipelined evaluation can be divided into 2 categories:

   1. Pull-based
   2. Push-based

2. Pull-based pipeline model

   We know how this works.

3. Push-based pipeline model

   For push-based model, each operation will have one thread executing to compute
   the result, once complete, the result will be passed to the thread of the next
   operation.

4. Comparison between pull-based model and push-based model

   > a good post: [Query Engines: Push vs. Pull](https://justinjaffray.com/query-engines-push-vs.-pull/)

   1. Pull-based model is easier to implement
   2. Pushed-based model is very useful in parallel processing system
   3. Push-based model has lower number of function calls (no `next()`)
   4. Push-based model is increasingly used in systems that generate machine
      code (query compilation) for high performance query evaluation.

      > QUES: why

### 15.7.2.2 Evaluation Algorithms for Pipelining
## 15.7.3 Pipelines for Continuous-Stream Data

1. Pipelining can also be used in cases where data entered into the database
   in a continuous manner.

   Push-based model is the suited for this continuous query evaluation.

   > QUES: I am not quite sure the reason, but it indeed feels that the data
   > will be pushed to the system in a streaming way. 

# 15.8 Query Processing in Memory

> Before this section, all the query processing algorithms assume the data is 
> stored on the disk so that we:
>
> 1. Care about the the # of block transfer and the # of seeks in cost analysis
> 2. Try to minimize the disk I/O
>
> In this section, we talk about algorithms that help minimize memory access
> costs by using cache-conscious processing algorithm and query compilation.

## 15.8.1 Cache-Conscious Algorithms

1. Latency: the time it takes to respond to a request
   
   * L1 cache: 1 nanosecond
   * L2 cache: 5 nanoseconds
   * L3 cache: 10-15 nanoseconds
   * Memory: 50-100 nanoseconds
   * NVME SSD: 20-100 microseconds
   * SATA SSD: 150-200 microseconds 

2. Cache line

   It is the smallest access unit that the CPU use when interacting with cache:

   1. Read cache/write cache
   2. If the requested data is not in cache, it will load the corresponding data
      from memory, in the unit of cache line

   > It is quite similar to memory page when we load files from disk.

   > On AMD64 CPUs, the cache line size is typically 64 bytes. On Apple m1 chips,
   > cache line size is 128 bytes.

3. Cache/memory is kinda similar to Memory/disk, but:

   With memory/disk, database systems can control what is in the cache. 

   > OS page cache
   >
   > DBMS usually do kernel page cache bypass to make the memory/buffer
   > context aware. Even with kernel page cache, the OS uses its own
   > strategy but provides severals syscalls to allow application to 
   > interfere, e.g., `madvise(2)`.

   But the strategy of CPU cahce is totally controlled by the hardware 
   so that DBMS can do nothing about it.

   Though we cannot control it, we can implement the system in a cache-friendly
   way.

4. Make queries cache-friendly

   1. Sort merge
      
      We want every run to fit in cache

   2. Hash join
      
      During the partitioning stage, the size of the partition along with the hash
      index should fit in cache, then in the probe stage, cache misses can be
      minimized.

   3. Tuple layout
      
      Columns that are open used together can be stored continuously. For example,
      columns used in the `GROUP BY` clause.

## 15.8.2 Query Compilation

1. We know that the traditional volcano model is not CPU-friendly, i.e., multiple
   virtual function `next()` are exectued during execution

   > [Apache Spark as a Compiler: Joining a Billion Rows per Second on a Laptop][post]
   >
   > [post]: https://www.databricks.com/blog/2016/05/23/apache-spark-as-a-compiler-joining-a-billion-rows-per-second-on-a-laptop.html

   If data residents on disk, this drawback won't be too obvious, but it will 
   immediately be the bottleneck if the data lives in memory.

2. With volcano model, SQL is actually **interpreted** rather than compiled to 
   machine code (or any bytecode, e.g., wasm).

3. Many in-memory database compile query plans into machine code or intermediate
   level byte-code.

   [PostgreSQL JIT](https://www.postgresql.org/docs/11/jit-decision.html)

   > TODO: add more examples to this note when you find them.

## 15.8.3 Column-Oriented Storage
# 15.9 Summary


