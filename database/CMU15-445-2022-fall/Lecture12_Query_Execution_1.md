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
    
   * join
   * sub-query
   * sort

   They won't return a values until all the values of the child operator are
   returned.

   > So in the implementation, these blocking operators should have a loop that
   > continuously pulls that child operator until it yields `None`.

3. Materialization model is suitable for OLTP since it won't access too many tuples
   at once, and we have fewer function calls compared to the iterator model.

4. Push-based model gives you more control over the execution.

# Access Methods

1. What are access methods

   An access method is how the DBMS accesses the data stored in the table, we 
   have generally 2 access methods:

   * TableScan
   * IndexScan

2. Optimizations that can be used with `TableScan`

   1. Prefetch to avoid blocking I/O
   2. Buffer pool by pass to avoid sequential flooding

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


# Modification Queries
# Expression Evaluation
