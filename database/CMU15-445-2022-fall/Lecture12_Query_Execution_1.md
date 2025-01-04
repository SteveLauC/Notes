> Today's agenda:
>
> * Processing Models
> * Access Methods
> * Modification Queries
> * Expression Evaluation


# Processing Models

1. What is processing model
   
   Processing models defines how DBMS executes the query plan.

   * Iterator/Volcano/Pipeline Model
   * Materialization Model
   * Vectorized/Batch Model

   > All these models can be implemented in:
   >
   > * pull-based (more common)
   > * push-based (allows more control over the pipeline)
   >
   > ways.

   > Clickhouse uses a push-based vectorized iterator model.

2. Blocking operators/pipeline breaker in the iterator model

   They won't return a values until all the values of the child operator are
   returned.
    
   * sort
     
     I think sort is indeed a blocking operator

   * join
     
     * nested loop join: kinda blocking
     * block nested loop join: kinda blocking
     * index nested loop join: If an index is available, does not seem blocking
     * Merge join: the sorting phase is blocking, after that, it is not
     * Hash join: the building phase is blocking, after that, it does not seem blocking

   * sub-query

     I think it depends on what the sub-query is, e.g.:

     ```sql
     SELECT * FROM
     (SELECT * FROM TABLE);
     ```

     The above query does not seem blocking.

   > So in the implementation, these blocking operators should have a loop that
   > continuously pulls that child operator until it yields `None`.

3. One drawback of the iterator model is that it only handles 1 tuple in a 
   function call `next()`.

4. Materialization model is suitable for OLTP since in OLTP, we won't access 
   too many tuples at once, and we have fewer function calls compared to the 
   iterator model.

   > Future steve: I think it is also better to reduce the cost of virtual
   > function call, so, if materialization model is usable, then it would be
   > better than the iterator model.

   And I think this model makes a lot of sense for in-memory databases, like 
   VoltDB. When things are in memory, the cost of the virtual functions calls
   can be dominant.

5. Iterator model handles 1 tuple at a time, materialization model does all the
   tuples at once, we typically want something in the middle.

6. Push-based model gives you more control over the execution.

7. Difference between materialization model and vectorized model
   
   In materialization model, an operator only returns until it receives all the 
   output from its child operator. While in the vectorized model, it returns as
   long as the accumulated output reaches a specific threshold.


# Access Methods

1. What are access methods

   An access method is how the DBMS accesses the data stored in the table, we 
   have generally 2 access methods:

   * TableScan
   * IndexScan

2. Optimizations that can be used with `TableScan`

   1. Prefetch to avoid blocking I/O
   2. Buffer pool by pass to avoid sequential flooding

      > For the pages that will be ONLY used for once, it makes no sense to load
      > them into the buffer pool.

      > QUES: what is sequential flooding
      >
      > Assume we:
      > 1. use LRU as the page replacement algorithm
      > 2. The # of frames within our Buffer Pool is smaller than the # of pages
      >    of the file we want to scan
      > 3. We repeatedly read that file
      >
      > Then EVERY SCAN OF A PAGE IS A CACHE MISS!
      >
      > See the demo [here](https://www.youtube.com/watch?v=cESKTl12Ulg&ab_channel=CS186Berkeley)

   3. Parallelization
      
      Do I/O using multiple threads

      > DataFusion has this by partitioning and Tokio.

   4. Late materialization 
     
      > Common in columnar store

      > DataFusion has this.
   
   5. Approximate Queries
      
      On senarios where accuracy is not that important, we can do queries on
      a subset of data to produce an approximate result.

   6. Zone maps (min-max index)

      > DataFusion has this with Parquet.

3. Index Selection

   With index scan, during the plan stage (generating physical plan), if multiple
   conditions are specified in the query and there are multiple indexes available,
   we should use the index that could produce the minimal rows.

4. Multiple index scan

   > Elasticsearch does this by default, if your query accesses multiple fields, 
   > e.g., a boolean query:
   >
   > ```json
   > {
   >   "query": {
   >     "bool": {
   >       "should": [
   >         {"term": { "int": 1 } },
   >         {"term": { "str": "foo" } }
   >       ]
   >     } 
   >   }
   > }
   > ```
   >
   > The above query accesses 2 index structures, the BKD-tree used for the `int` 
   > field, and the full-text index for `str`.

   Advanced DBMSs can use multiple indexes for a single query.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-04-12%2011-22-39.png)

   * [PostgreSQL Indexes Bitmap Scan](https://www.postgresql.org/docs/current/indexes-bitmap-scans.html)
     
      To combine multiple indexes, the system scans each needed index and prepares 
      a bitmap in memory giving the locations of table rows that are reported as 
      matching that index's conditions. The bitmaps are then ANDed and ORed together 
      as needed by the query. Finally, the actual table rows are visited and returned.

      > This is exactly how Lucene employs multiple indexes.

   * [MySQL Indexes Merge](https://dev.mysql.com/doc/refman/8.0/en/index-merge-optimization.html)

# Modification Queries

1. What are modification queries

   Modification queries are queries that modify the data:

   * UPDATE/DELETE
     
     The child operator should return the Record IDs of the tuples that need to
     be modified.
     
   * INSERT

2. `UPDATE` query problem

   For example, when you update a B+Tree, a value satisfies the predicate, then
   you remove it from the B+Tree, update the value then insert it back. It is
   possible that the new inserted value will be scanned again, and it also 
   satisfies the condition, then you update it again, an error occurs.

   ```sql
   UPDATE students
   SET AGE = AGE * 2;
   WHERE AGE % 2 = 0;
   ```

   Let's demo this with the following Rust code:

   ```rs
   use std::cmp::Ordering;

   #[derive(Clone, Copy, Debug, PartialEq, Eq, Ord)]
   struct Student {
       id: usize,
       age: usize,
   }

   impl PartialOrd for Student {
       fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
           self.age.partial_cmp(&other.age)
       }
   }

   macro_rules! student {
       ($id:expr, $age:expr) => {
           Student { id: $id, age: $age }
       };
   }

   /// Double the age if it is even.
   ///
   /// This impl is buggy, but it is ok because it is for demo only.
   fn update(students: &mut Vec<Student>) {
       let len = students.len();
       for idx in 0..len {
           if students[idx].age % 2 == 0 {
               let mut val = students.remove(idx);
               val.age *= 2;
               let mut dst_idx = None;
               for (idx, item) in students.iter().enumerate() {
                   if item.age > val.age {
                       dst_idx = Some(idx);
                   }
               }
               match dst_idx {
                   Some(idx) => students.insert(idx, val),
                   None => students.push(val),
               }
           }
       }
   }

   fn main() {
       let mut students = vec![student!(0, 4), student!(1, 5), student!(3, 9)];

       update(&mut students);
       println!("{:?}", students);
   }
   ```

   I expect the result to be:

   ```
   [Student { id: 1, age: 5 }, Student { id: 0, age: 8 }, Student { id: 3, age: 9 }]
   ```

   But I got:

   > NOTE: the student with `id` 0 has been updated for 2 times!

   ```
   [Student { id: 1, age: 5 }, Student { id: 3, age: 9 }, Student { id: 0, age: 32 }]
   ```

   To solve the issue, collect the RecordIDs of the tuples that have been updated
   so that you won't modify them over and over again.

   > This issue is called [`Halloween Problem`][wiki], it was originally 
   > discovered by IBM researchers while building System R on Halloween 
   > day in 1976.
   >
   > [wiki]: https://en.wikipedia.org/wiki/Halloween_Problem
   >
   > [Blog from CockroachDB](https://www.cockroachlabs.com/blog/the-halloween-problem-sql/)
   >
   > Also see Database System Concepts 16.6.3 for the cases where this problem
   > will never happen. 

# Expression Evaluation

1. For the conditions specified in a `WHERE` caluse, it will be evaluated against
   every tuple, and evaluating an `Expr` tree itself is slow.

   How can we speed it up? JIT

   We compile the `Expr` to machine code/wasm once, pretty much like a function,
   then we apply it to the tuples.

   > QUES: To do JIT, does the DBMS need a built-in compiler, or it can simply
   > use the one installed on the host?

   It is slow, speed it up with JIT.

2. The `PREPARE` statement can be used to avoid parsing the statements that will
   be used frequently:
  
   ```sql
   D prepare sum as select $1+$2;
   D execute sum(1, 2);
   ┌───────────┐
   │ ($1 + $2) │
   │   int32   │
   ├───────────┤
   │         3 │
   └───────────┘
   ```

3. For expression evaluation, seems that translating the SQLParserExpr type to
   your own `Expr` can be costly.
