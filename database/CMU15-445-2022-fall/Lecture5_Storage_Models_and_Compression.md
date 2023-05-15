> Today's agenda:
>
> * Database Workload
> * Storage Models
> * Database Compression
> * Naive Compression
> * Columnar Compression

### Database workload

1. database workloads(what is database used for? )

   1. On-Line Transaction Processing(OLTP): fast operations that only read/update
      a small amount of data each time.(mostly used by user)(mainly **write** 
      to the DB)

   2. On-Line Analytical Processing(OLAP): Complex queries that read a lot of
      data to compute aggregates(mostly used by data scientist)(mainly **read**
      from the DB)

   3. Hybrid Transaction + Analytical Processing: OLTP+OLAP

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2014-44-15.png)


### Storage models

0. The relational model does not specify how the tuples should be stored on
   the disk, and there is a possibility that the attributes of a tuple are
   not stored in a single page.

For different workloads, DBMS can store tuples in different ways so that it
can fit the workloads most.

1. n-ary storage model(aka **row storage**)(NSM)

   > What we will do in this semester.

   The DBMS sotres all attributes for a single tuple continuously in a page.

   * Advantages:
     1. fast `inserts`, `updates`, and `deletes`
     2. good for queries that need **the entire tuple**

     > Suitable for writing and OLTP

   * Disadvantage:

     1. not good for scanning large portions of table and a subset of the
        attributes. When you need just few attributes, the DBMS still has to 
        fetch the *whole tuple* to get that few attributes.

2. decomposition storage model(aka **column storage**)(DSM)

   The DBMS sotres the values of a single attribute for all tuples continuously
   in a page. If we only need one attribute, then we just need one page.

   * Advantage:

     1. reduces the amount of wasted I/O because the DBMS only reads the data
        it needs(ONLY fetch the columns needed)

        > OLAP SQL queries are complex, but may ONLY touch few attributes.
        >
        > ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-14%2020-50-44.png)
        > This query only needs attributes `lastlogin` and `hostname`, in row
        > storage, the I/O for other attributes are wasted.

     2. better query processing and data compression. 

        Compression can be done cause all the data in a page have the same 
        type(domain). For example, if this page is all about temperature, 
        and the data is all about 30 degrees, like 31, 29, then we can just 
        store 1 and -1.

   * Disadvantage:

     1. slow for point queries(need the retire tuple, may access multiple pages),
        inserts, updates and deletes because of tuple splitting

3. Tuple identification in Column Storage

   To put the tuples back together when using a column store, there are two 
   common approaches: 

   1. fixed-length offsets(mostly used)

      Values of the same attribute have the same length so that when searching for
      a specific tuple, we use `LEN_OF_ATTR` * `index` to find its attributes and
      put them together.

      Two values of two different columns have the same `index`, they belong to
      the same tuple.

      > TODO: All values of the same column must have same length, what about 
      > var-len types?
      >
      > 

   2. Embedded tuple IDs

      For every attribute in the columns, the DBMS stores a tuple id (ex: a primary
      key) with it.

      > Overhead: we need to store an extra attribute(storage overhead), and 
      > when seaching for a specific tuple, we need to read the tuple ID first.
      > (Or there need to be an index)

### Database Compression

1. Why do we need compression
  
   Compression is widely used in disk-based DBMSs. Because disk I/O is (almost) 
   always the main bottleneck. More CPU reousrces needed since compression and
   decompression are CPU bound, but most cases, CPU is much faster than the disk.

   > For in-memory databases, they don't have that bottleneck but also wants to
   > reduce the memory consumption by compression.

2. Why the data in a database can be compressed
   
   1. Skewed distribution (math term)

      Which means there will be multiple repeated data. For example, in a English
      book, the most common word is `the`, and the next one is `a`.

   2. Values of different attributes have high `correlation`
      
      For example, one column is zip code, another one is the city name, their
      relationship is one-to-one, we know the zip code, we know the city name.

3. Three goals of database compression
  
   1. Must produce fixed-length values
      
      We want to use `fixed-length offsets` approach when identifying a specific
      tuple.

      > TODO: Isn't this ONLY needed in column storage?

   2. Postpone decompression as long as possible
      
      > The optimal case is that we can operate on the compressed data directly.

      This is called `late materialization`.

   3. Loseless
     
      We don't wanna lose our data.

4. Compression Granularity

   * Block Level: Compress a block of tuples for the same table.

   * Tuple Level: Compress the contents of the entire tuple (Row Storage only).

   * Attribute Level: Compress a single attribute value within one tuple. Can 
      target multiple attributes for the same tuple.

   * Columnar Level: Compress multiple values for one or more attributes stored 
     for multiple tuples (Column Storage only). This allows for more complicated 
     compression schemes.

5. Naive Compression

   > Compression schemas (methods) that are not stupid.

   What about simply compress the whole disk page, this is stupid as an 
   decompression is needed whenever we need to access the page. In this way,
   `late materialization` is impossible.

   And this compression schema does not consider the meaning/semantics of the
   compressed data.

6. Columnar Compression

   For compression at the column levels, we can apply the following schemas:
 
   * Run-Length Encoding (RLE)

     ```
     hello -> [('h', 0, 1), ('e', 1, 1), ('l', 2, 2), ('o', 4, 1)]
     ```

     > The result triple `(value, offset, Length)` is called `RLE triple`.

     It converted duplicate entries into a single entry to reduce the space usage.
     As we can see, only adjacant and repeated entries can be deduplicated, to 
     maximize compression opportunities, it would be great to sort the values 
     first.

     Such a compression can be seen as a `group by` on the adjacant values and
     thus can speed up `group by` queries.

     ```sql
     CREATE TABLE test (
         id INT,
         c  CHAR, 
     );

     INSERT INTO test VALUES 
     (0, 'h'),
     (1, 'e'),
     (2, 'l'),
     (3, 'l'),
     (4, 'o');

     SELECT count(*) 
     FROM test
     GROUP BY c;
     ```

   * Bit-Packing Encoding

     When values for an attribute are always less than the value's declared 
     largest size, store them as smaller data type.

     ```sql
     CREATE TABLE test (
        i BIGINT 
     )

     INSERT INTO test VALUES 
     (2),
     (4),
     (45),
     (6),
     (18);
     ```

     Using 64 bits (`BIGINT` is 64-bit long) is overkill, 8-bit is sufficient.

   * Mostly Encoding

     This is a variant of `Bit-packing`, for the following data, most values can
     be stored in `u8`, few can not. For those exceptions, instead of storing them
     in the original table, we store them in a look-up table. 

     ```sql
     insert into test values 
     (2),
     (9999999999999),
     (8888888888888),
     (4);
     ```

     ```
     // before

     | 2           |
     |9999999999999|
     |8888888888888|
     | 4           |
     ```

     ```
     // after

     | 2  |     | offset, value    |
     |sign|     |   1, 999999999999|
     |sign|     |   2, 888888888888|
     | 4  |
     ```

   * Bitmap Encoding

     > [What is `cardinality` in Databases?](https://stackoverflow.com/questions/10621077/what-is-cardinality-in-databases)
     > In this context, `Cardinality` means the number of unique values of a 
     > specific attribute.

     Store a separate bitmap for each unique value for an attribute where an 
     offset in the vector corresponds to a tuple.

     ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-15%2022-48-19.png)

     > Only practical if the value cardinality is low.

   * Delta Encoding

     Recording the difference between values that follow each other in the same column.

     > This schema can be combined with `Run length Encoding` to get better result.

     ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-15%2022-53-22.png)

   * Incremental Encoding

     An variant of `Delta Encoding`, `Delta Encoding` minimize space usage by
     getting rid of the base value, `Incremental Encoding` removes the common
     prefixes and suffixes.

     ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-15%2022-58-18.png)

   * Dictionary Compression(Mostly widely used)
     
     Build a data structure that maps variable-length values to a smaller integer
     identifier.

     > This turns `var-len` values into fixed length values.

     ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-15%2023-00-45.png)
