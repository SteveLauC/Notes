> This chapter is about:
>
> * Before I read it (Look at the TOC)
>
>   * Introduce several common partitioning strategies
>   * Introduce rebalancing (fix skewed partitioning) and request routing (to do the query)
>
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>
>   * When should I use range-based partitioning or hash-based partitioning
>
>     * Range partitioning supports efficient range query, but can be uneven if
>       you use a pre-defined strategy. This can be solved by letting the DBMS
>       define the partition strategy
>
>     * Hash-based partitioning should be even if you use a good hash algorithm,
>       range query will be inefficient with this approach.
>
>   * How to do rebalancing? If I use hash-based partitioning strategies, do I 
>     need to do a rehash?
>   * How Elasticsearch/Citus/YugabyteDB do rebalancing
> 
>   * How to partition indexes
>     * Primary index
>     * Secondary index

> What have you learned from it
>
> *
> *


> TOC
>
> * Partitioning and Replication
> * Partitioning of Key-Value Data
>   * Partitioning by Key Range
>   * Partitioning by Hash of Key
>   * Skewed Workloads and Relieving Hot Spots
> * Partitioning and Secondary Indexes
>   * Partitioning Secondary Indexes by Document
>   * Partitioning Secondary Indexes by Term
> * Rebalancing Partitions
>   * Strategies for Rebalancing
>   * Operations: Automatic for Manual Rebalancing
> * Request Routing
>   * Parallel Query Execution
> * Summary

1. YugabyteDB calls its partition Tablet, I think this is because Google Spanner
   uses Tablet as well, which is also because the use of Tablet in Google 
   Bigtable.
   
2. The main reason for partitioning is scability, the data is too big to be dealt
   by a single node.

# Partitioning and Replication
## Partitioning of Key-Value Data
## Partitioning by Key Range

1. For range-based partition, in the case where we are unaware of the data 
   distributions, if we want the partitions to be even, then the partition 
   boundaries should be set by the database. Since it is controlled by the 
   database, maybe we can let it do the dynamic partitioning, i.e.,  automatically
   adjust the boundary when skewed partition occurs.

## Partitioning by Hash of Key

1. The better a hash function is, the less conflicts it will produce. So a good
   hash function can partition data evenly.
   
2. For partitioning purposes, the used hash function does not need to be 
   cryptographically strong
   
3. By using hash-based partitioning, we cannot do range query efficiently, even
   though we store the data in an order within each partition, we still need to
   scan all the partitions.
   
   > "we need to scan all the partitions"
   >
   > In MongoDB, if you enabled hash-based partitioning, any range query has to
   > be sent to all partitions
   
   This is something naturally efficient in range-based partitioning.

4. The hash algorithm used by hash-based partition should always return the same
   value for the same given key.
   
   In Rust, you should not use the default `RandomState` `BuildHasher`, as 2 `hasher`
   constructed from 2 `RandomState`s will give different values for the same key.

## Skewed Workloads and Relieving Hot Spots
# Partitioning and Secondary Indexes

> The previous section introduces how to partition data, specifically, how to
> partition key-value data, if we apply this strategy to relational model
> or document model, the key we choose for partitioning, is usually the primary
> key or document ID.
>
> Partitioning by primary key or document ID makes partitioning secondary indexes
> hard, because secondary index does not usually involve primary key or document
> ID.

1. Search engines like Elasticsearch or Solr are full of secondary indexes, they
   build index for all the fields.
   
   I just realized that all these secondary indexes in Elasticsearch store document
   IDs, i.e., the primary key value rather than disk location. MySQL does this and
   [Uber switched from Postgres to MySQL for this reason.][link_ref]
   
   [link_ref]: https://github.com/SteveLauC/Notes/blob/ab166174f4a1260edf17058cdee928053a24015c/database/Database_System_Concepts/Ch14_Indexing.md?plain=1#L1267-L1285
   
   Though ES uses Luecene under the hood, you don't know the actual disk location.
   
   If you build SQL over KV, I guess this is the only way you can do with secondary
   index.

## Partitioning Secondary Indexes by Document

Build indexes for each partition separately, ES uses this approach. This is also
called local index. For queries over the index, you need to query all the partitions
and merge the results (MapReduce)

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-11-28%20at%201.41.46%E2%80%AFPM.png)

The book says that this approach suffers from tail latency amplification due to
its query mode.

## Partitioning Secondary Indexes by Term
# Rebalancing Partitions
## Strategies for Rebalancing
## Operations: Automatic for Manual Rebalancing
# Request Routing
## Parallel Query Execution
# Summary