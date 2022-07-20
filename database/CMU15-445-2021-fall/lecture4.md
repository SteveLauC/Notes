1. The advantage of log-structured architecture is that it can turn the random 
   write into sequential write(always appending), which is way faster.

   But for reading, it needs to scan all the logs from the end to the beginning
   to find the correct value.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2017-14-44.png)

2. database workloads(what is database used for? )

   1. On-Line Transaction Processing(OLTP): fast operations that only read/update
   a small amount of data each time.(mostly used by user)(mainly write to the DB)

   2. On-Line Analytical Processing(OLAP): Complex queries that read a lot of
   data to compute aggregates(mostly used by data scientist)(mainly read from the DB)

   3. Hybrid Transaction + Analytical Processing: OLTP+OLAP

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2014-44-15.png)

   
## Today's agenda

### Data representation

> How to represent data type in DBMS(Actually, this is part of `tuple 
layout` which we have covered part of it in the last lecture)

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2017-29-10.png)


1. how to store floating point in DBMS

   Float/Double(IEEE 754) is fast cause CPU is very efficient at doing such 
   type operations. But it is inexact(0.1+0.2 != 0.3) and we need an exact type
   to represent floating point numbers

   > ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2017-47-22.png)
   > Same data, different types. Numeric/Decimal type takes twice the time 
   > compared to the Float/Double.
     

   What does PostgreSQL do to handle this?[src](https://doxygen.postgresql.org/backend_2utils_2adt_2numeric_8c_source.html#l00304)
    
   ```c
    typedef unsigned char NumericDigit;
    typedef struct {
            int ndigits; // how many digits we have
	    int weight;  // weight of first digit
	    int scale;   // scale factor
	    int sign;    // positive|negitive|NaN
	    NumericDigit *digits; // digits storage
    } Numeric;
   ```

2. How to store large values?

   Generally, a tuple is limited in its size to not be greater than the 
   size of a single page.

   To store values(attributes values) larger than a page, DBMS uses separate
   `overflow page`(a dedicated page for storing large value)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2012-29-17.png)
    
   For example:
   * In PostgreSQL: if the value is bigger than a constant 2KiB, then it
   will be stored in overflow page.

   * MySQL: page size is typically 16 KiB, threshold is the half size of page 

   * SQL Server: threshold is the page size.

3. External value storage

   If the value so big that `overflow page` cannot hold it, then DBMS will use 
   a seperate file to store it. This is named BLOB type(bad way).

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

### Storage models

For different workloads, DBMS can store tuples in different ways so that it
can fit the workloads most.

1. n-ary storage model(aka row storage)(NSM)(what we will do in the semester)

   The DBMS sotres all attributes for a single tuple continuously in a page.

   * Advantages:
     1. fast inserts, updates, and deletes
     2. good for queries that need the entire tuple

     > Suitable for writeing and OLTP

   * Disadvantage:

     1. not good for scanning large portions of table and a subset of the
     attributes. When you need just few attributes, the DBMS still has to fetch
     the *whole tuple* to get that few attributes.

2. decomposition storage model(aka column storage)(DSM)

   The DBMS sotres the values of a single attribute for all tuples continuously
   in a page. If we only need one attribute, then we just need one page.

   * Advantage:

     1. reduces the amount of wasted I/O because the DBMS only reads the data
     it needs

     2. better query processing and data compression. Compression can be done
     cause all the data in a page have the same type(domain). For example, if
     this page is all about temperature, and the data is all about 30 degrees,
     like 31, 29, then we can just store 1 and -1.

   * Disadvantage:

     1. slow for point queries(need the retire tuple), inserts, updates and 
     deletes because of tuple splitting
