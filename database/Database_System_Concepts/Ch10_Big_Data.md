> * 10.1 Motivation
> * 10.2 Big Data Storage Systems (Storage)
> * 10.3 The MapReduce Paradigm (Compution)
> * 10.4 Beyond MapReduce: Algebraic Operations
> * 10.5 Streaming Data
> * 10.6 Graph Databases
> * 10.7 Summary

# 10.1 Motivation
1. In 2000s, with the popularity of the Internet, a big volume of data has been
   created, most of which are unstructured and semi-structured, i.e., it is not
   in realtional model.

   Such a big volume of data far exceeds the level that could be handled by a
   relational database.

2. The characteristics of big data
   1. Volume: The amount of data to be stored and processed is much larger than
      traditional databases, including traditional parallel database systems.

   2. Velocity: the rate of arrival of data are much higher in today's networked
      world than in earlier days.

   3. Variety: Various formats of data, and apparently, most of them are not
      relational.

3. SQL is by far the most widely used language for querying relational databases.
   However, there is a wide variety of query language options for Big Data
   applications, driven by the need to handle more variety of data types.

   For example, for time series data, PromQL (Prometheus Query Language).

4. A key to the success of these techniques is the fact that they allow 
   specification of complex data processing task, while enabling easy 
   parallelization of the tasks.

   These techniques free the programmer from having to deal with issues such
   as how to perform parallelization, how to deal with failures, how to deal
   with load imbalances between machines, and may other similar low-level issues.

# 10.2 Big Data Storage Systems
1. Applications of Big Data requires extremely high scalability, to achieve this,
   the data has to be partitioned and stored across thousands of computing and
   storage nodes (distributed).

2. `Separating storage and compute` is a concept in distributed systems, where
   storage and compute are able to be separated so that they can be put on different
   node to achieve better scalability.

3. A number of storage systems for Big Data:

   1. Distributed File System(DFS)

      There allows files to be stored across a number of machines,while allowing
      accesss to files using traditional file-system interface. Clients do not
      need to bother about where the files are stored.
   
      DFS is designed to store very large files and support big numbers of 
      concurrent clients.

      A big file is broken into multiple blocks, the blocks of the same file
      can be partitioned across multiple machines. Further, each file block
      is replicated across multiple machines (typically three), so that a 
      machine failure does not result in the file becoming inaccessable.

      For example:
      * Google File System (GFS)
      * hardoop File System (HDFS)
        > HDFS is based on the GFS architure 

      ![arch hdfs](https://github.com/SteveLauC/pic/blob/main/arch_hdfs.jpeg)

   2. Sharding across multiple databases

      The term `sharding` refers to the partitioning of data across multiple
      databases or machines.

      The category of partitioning (how a table is partitioned):
      * Range Partitioning: for example, all tuples with `key` in range [0, 10000]
        are stored in this database, and other tuples are stroed in the second
        database.
      * Hash Partitioning: Use a hash function to decide which database a tuple 
        goes to.

      > Is this a "workaround" to enable the traditional databases to be capable
      > of handling big data? (to support horizontal scaling)

   3. Key-Value Storage Systems: NoSQL guys

      (Big) Web applications need to store very large numbers of relatively small 
      records, storing each small record as a separate file is infeasible as
      file systems, including distributed file systems, are not designed to
      store such large numbers of files.

      Ideally, a massively parallel realtional databases should be used to
      store such data, but building such a relational database is hard.

      A number of storage systems have been developed that can scale to the needs
      of web applications and store large amount of data, but typically offering
      a simple key-value interface.

      [Video: MongoDB is Web Scale](https://www.youtube.com/watch?v=b2F-DItXtZs&ab_channel=gar1t)

      Features provided by Key-Value stores are quite limited since it is hard
      to implement those great features without losing the support for horizontal
      scaling, and they are proud to not support SQL features initially, thus, 
      they call them NoSQL databases, but later on, they find the lack for SQL
      and great features like transction is not good.

      Examples:
      * BigTable(Google)
      * HBase
      * MongoDB (Doc DB)


   4. Parallel and Distributed Databases

# 10.3 The MapReduce Paradigm
# 10.4 Beyond MapReduce: Algebraic Operations
# 10.5 Streaming Data
# 10.6 Graph Databases
# 10.7 Summary
