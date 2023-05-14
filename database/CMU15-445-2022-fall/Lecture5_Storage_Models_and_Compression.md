### Database workloads

1. database workloads(what is database used for? )

   1. On-Line Transaction Processing(OLTP): fast operations that only read/update
      a small amount of data each time.(mostly used by user)(mainly **write** 
      to the DB)

   2. On-Line Analytical Processing(OLAP): Complex queries that read a lot of
      data to compute aggregates(mostly used by data scientist)(mainly **read**
      from the DB)

   3. Hybrid Transaction + Analytical Processing: OLTP+OLAP

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2014-44-15.png)

2. The relational model does not specify how the tuples should be stored on
   the disk, and there is a possibility that the attributes of a tuple are
   not stored in a single page.

   > In this semester, we assume the database is used for OLTP and uses row 
   > oriented storage underneath.

### Storage models

For different workloads, DBMS can store tuples in different ways so that it
can fit the workloads most.

1. n-ary storage model(aka **row storage**)(NSM)(what we will do in the semester)

   The DBMS sotres all attributes for a single tuple continuously in a page.

   * Advantages:
     1. fast inserts, updates, and deletes
     2. good for queries that need **the entire tuple**

     > Suitable for writeing and OLTP

   * Disadvantage:

     1. not good for scanning large portions of table and a subset of the
        attributes. When you need just few attributes, the DBMS still has to 
        fetch the *whole tuple* to get that few attributes.

2. decomposition storage model(aka **column storage**)(DSM)

   The DBMS sotres the values of a single attribute for all tuples continuously
   in a page. If we only need one attribute, then we just need one page.

   * Advantage:

     1. reduces the amount of wasted I/O because the DBMS only reads the data
        it needs

        > OLAP SQL queries are complex, but may ONLY touch few attributes.
        >
        > ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-14%2020-50-44.png)
        > This query only needs attributes `lastlogin` and `hostname`, in row
        > storage, the I/O for other attributes are wasted.

     2. better query processing and data compression. Compression can be done
        cause all the data in a page have the same type(domain). For example, if
        this page is all about temperature, and the data is all about 30 degrees,
        like 31, 29, then we can just store 1 and -1.

   * Disadvantage:

     1. slow for point queries(need the retire tuple, may access multiple pages),
        inserts, updates and deletes because of tuple splitting

### Tuple Identification in Column Storage
To put the tuples back together when using a column store, there are two common 
approaches: 

1. fixed-length offsets(mostly used)

   Values of the same attribute have the same length so that when searching for
   a specific tuple, we use `LEN_OF_ATTR` * `index` to find its attributes and
   put them together.

   > What about var-len types

2. Embedded tuple IDs

   For every attribute in the columns, the DBMS stores a tuple id (ex: a primary
   key) with it.

   > Overhead: we need to store an extra attribute, and when seaching for a 
   > specific tuple, we need to read the tuple ID first. (Or there need to be
   > an index)

### Database Compression

1. Why do we need compression
  
   Compression is widely used in disk-based DBMSs. Because disk I/O is (almost) 
   always the main bottleneck.

   > For in-memory databases, they don't have that bottleneck but also wants to
   > reduce the memory consumption by compression.

2. Why the data in a database can be compressed
   
   1. Skewed distribution (math term)

      Which means there will be multiple repeated data. For example, in a English
      book, the most common word is `the`, and the next one is `a`.

   2. Values of different attributes have high `correlation`
      
      For example, one column is zip code, another one is the city name, their
      relationship is one-to-one, we know the zip code, we know the city name.

1. Three goals of database compression
  
   1. Must produce fixed-length values
      
      We want to use `fixed-length offsets` approach when identifying a specific
      tuple.

   2. Postpone decompression as long as possible
      
      > The optimal case is that we can operate on the compressed data directly.

      This is called `late materialization`.

   3. Loseless
     
      We don't wanna lose our data.
