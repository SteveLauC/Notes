> Post URL: https://yetanotherdevblog.com/lsm/

> A log-structured merge-tree (LSM tree) is a data structure typically used when
> dealing with **write-heavy** workloads. The write path **is optimized by only 
> performing sequential writes**. LSM trees are the core data structure behind 
> many databases, including BigTable, Cassandra, Scylla, and RocksDB.

# SSTables

LSM trees are **persisted to disk** using a **Sorted Strings Table (SSTable)** 
format. As indicated by the name, SSTables are a format for storing key-value 
pairs in which the keys are in sorted order. An SSTable will consist of multiple
sorted files called segments. These segments are immutable once they are written 
to disk. A simple example could look like this:

![SSTable diagram](https://github.com/SteveLauC/pic/blob/main/SSTable-diagram.png)

# Writing Table

Every data in a SSTable is sorted, but the data in LSM tree is not. When writing
to the disk, how does the data got sorted.

There is a intermediate memtable that is responsable for handling this, and 
the underlying data structure used in this memtable is red-black tree, 

> Red-black tree is a special kind (balanced) of binary search tree

Our writes get sorted in this memtable unitl this tree reaches a pre-defined size.
Once the red-black tree has enough entries, it is flushed to disk as a **segment**
on disk in sorted order.

![flush to the disk](https://github.com/SteveLauC/pic/blob/main/output-onlinepngtools--5-.png)

# Reading Data

We store a in-memory sparse index(which is also sorted) to quicken our reads.
we have to scan the whole SSTable to locate the key-value that we are 
looking for if there is no such an index, with this index, we can reduce the 
scope of the search to a very small protion

![dia](https://github.com/SteveLauC/pic/blob/main/output-onlinepngtools--6-.png)

# Compaction
