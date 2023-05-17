## Today's agenda

> * Log-Structured Storage
> * Data Representation
> * System Catalogs

0. Problem of slotted-page structure
   
   * Fragmentation: Deletion of tuples can leave gaps in the pages.
   * Useless Disk I/O: Due to the block-oriented nature of non-volatile storage,
     the whole block needs to be read to fetch a tuple.
   * Random Disk I/O: The disk reader could have to jump to 20 different places 
     to update 20 different tuples, which can be very slow. 

   > The first and third problems can be addressed with log structure storage.
   > For the second one, it is disk's fault.

1. The advantage of log-structured architecture is that it can turn the random 
   write into sequential write(always appending), which is way faster.

   But for reading, it needs to scan all the logs from the end to the beginning
   to find the correct value.(read amplification)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2017-14-44.png)

### Log-Structured Storage
1. Modification on tuple-oriented structure is some kinda change that happens
   in plcae. What if the DBMS could not overwrite data in pages and could only
   create new pages? Log-structured storage works in this way.

   > e.g., S3/HDFS

   > And you need to distinguish between the features of disk the those cloud
   > stroage like S3, for disk, to modify a byte, you have to overwrite the 
   > whole page that contains that byte. In S3, this is not allowed, you have
   > to overwrite the old file.(or we can say, create a new one) 
   >
   > Partial read is supported in S3, see 
   > [`range_read()` in OpenDAL](https://docs.rs/opendal/latest/opendal/struct.Operator.html#method.range_read)

2. Log Structured Storage

   > Most classes introduce this through LSM, Andy does not like this idea.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-06%2019-01-10.png)

   In a log structure system, CRUD is simplified to two basic operatons:
   1. PUT (insert or update)
   2. DEL (delete) 
      > `DEL` can be seen as a special kind of `PUT`, it `PUT`s a special value,
      > in LSM, it is called `tombstone`.

3. Compaction

   Since log structured storcture always apapends logs to the end of the file(
   duplicate entries, space amplification), the DBMS need to compact them 
   periodically.

   After the compaction, for each entry, there will be at most 1 entries.

   > Such a compaction is resource-intensive

   ![compaction](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-06%2019-46-05.png)

   > For the deleted entries, they should be cleared too as we don't care they
   > are deleted or have never existed before.

4. Downsides of LSM
   1. Read amplification: There are multiple copies for each entries, you need to
      read them all to find the entry you want.
   2. Space amplification

   If the compaction is very frequent, then write amplification can also be a
   probelm, but with compaction, read amplication and space amplication are
   mitigated.

5. LSM or B+Tree, they are the stroage layer, that is a lower layer, we can
   support Relational Model or KV Model on any top of them.

   In reality, most SQL DBs use B+Tree, and most NoSQL DBs use LSM. But there 
   are exceptions:
   * CockroachDB, a SQL database, uses RocksDB(LSM) as its storage engine.
   * TIDB, TIKV is a KV engine built on top of RocksDB.

### Data representation

> How to represent data type in DBMS(Actually, this is part of `tuple layout`
> which we have covered part of it in the last lecture)

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2017-29-10.png)


1. how to store floating point in DBMS

   Float/Double(IEEE 754) is fast cause CPU is very efficient at doing such 
   type operations. But it is inexact(0.1+0.2 != 0.3) and we need an exact type
   to represent floating point numbers, i.e., fixed point floats.

   > ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2017-47-22.png)
   >
   > Same data, different types. Numeric/Decimal type takes twice the time 
   > compared to the Float/Double.
     

   Different implementations handle this stuff differently, this is how PostgreSQL
   handles this?

   [PostgreSQL: numeric.c](https://doxygen.postgresql.org/backend_2utils_2adt_2numeric_8c_source.html#l00304)
    
   ```c
   // In PostgreSQL, they store the string of that float and some metadata that
   // could be used to interpret the string.

   typedef unsigned char NumericDigit;
   typedef struct {
       // Metadata
       int ndigits; // how many digits we have
       int weight;  // weight of first digit
       int scale;   // scale factor
       int sign;    // positive|negitive|NaN

       // String
       NumericDigit *digits; // digits storage
   } Numeric;
   ```

   Numeric/Decimal are exact but slow since there is no hardware support, Let's
   try this in PostgreSQL:

   ```SQL
   CREATE Table fixed (
       a decimal(10, 2),
       b decimal(10, 2),
   );

   CREATE TABLE float (
       a float,
       b float,
   );

   INSERT INTO fixed values 
   (0.1, 0.2);

   INSERT INTO float values 
   (0.1, 0.2);

   INSERT INTO fixed 
   SELECT * FROM fixed;

   INSERT INTO float 
   SELECT * FROM fixed;

   ...

   SELECT count(*) FROM fixed;
   +-------+
   | count |
   |-------|
   | 8192  |
   +-------+
   SELECT count(*) FROM float;
   +-------+
   | count |
   |-------|
   | 8192  |
   +-------+
   ```

   ```SQL
   # Turn on the timing
   > \timing on 
   # Turn off the multi-threaded mode
   > SET max_parallel_workers_per_gather = 0;

   # They are all loaded into the memory
   > select sum(a+b) from fixed;
      sum
   ---------
    2457.60
   (1 row)

   Time: 2.906 ms
   > select sum(a+b) from float;
           sum
   --------------------
   2457.6000000000295
   (1 row)

   Time: 1.271 ms
   ```


2. How to store large values?

   > The bigger a object is, the less is updated.

   1. Overflow Page
      Generally, a tuple is limited in its size to not be greater than the 
      size of a single page.

      To store values(attributes values) larger than a page, DBMS uses separate
      `overflow page`(a dedicated page for storing large value), it is typically
      bigger than the average page.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2012-29-17.png)
       
      For example:
      * In PostgreSQL: if the value is bigger than a constant 2KiB(this is called
        `TOAST`), then it will be stored in overflow page.

      * MySQL: page size is typically 16 KiB, threshold is the half size of page 

      * SQL Server: threshold is the page size.

   2. External value storage

      If the value so big that `overflow page` cannot hold it, then DBMS will use 
      a seperate file to store it. This is named BLOB type(bad way).

      Since the data is managed by the file system, you may encounter violation of
      constrictions. Some DBMSs, however, to prevent this from happening, make them
      integrated with the underlying file system to stop this.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2012-29-59.png)

### System catalogs

> What is system catalog and how is it stored?

The system catalogs contain metadata about databases:
* tables, columns, indexes, views
* users, permission
* internal statics

Every DBMS stores its catalogs in its own table. For example, in MySQL:

```sql
> use information_schema;
> show tables;
+---------------------------------------+
| Tables_in_information_schema          |
+---------------------------------------+
| ADMINISTRABLE_ROLE_AUTHORIZATIONS     |
| APPLICABLE_ROLES                      |
| CHARACTER_SETS                        |
| CHECK_CONSTRAINTS                     |
| COLLATIONS                            |
| COLLATION_CHARACTER_SET_APPLICABILITY |
```

In PostgreSQL, you get this a bunch of stuff: https://www.postgresql.org/docs/current/catalogs-overview.html
