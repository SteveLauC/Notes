1. Several things distributed database systems employ to be

   1. highly-available
   2. durable
   3. performant
   4. Scalable
   
   * Replication: place copies of data on different machines
   * Distribution: Place partitions of data on different machines
   * Decentralization: place different DBMS activities on different machiens
   
   For a database with typical shared-nothing architecture, replication
   means primary and replica shards, distribution means sharding, decentralization 
   means many nodes in the cluster.
   
   For a database with shared-disk architecture, replication will be handled by
   object storage, distribution does not exist, decentralization exists, but
   these nodes are all state-less compute nodes.
   
2. Common distributed Postgres architectures

   * Read replicas (built-in)
   
     Postgres has built-in support for physical replication to read-only replicas,
     which can be set up by adding standby node
     
   * Network-attached block storage
   
     Common architectures for cloud-based databases
     
     > Shared-disk arch
     
   * DBMS-optimized cloud storage (Amazon Aurora, Neon)
   
     Similar to Network-attached block storage, but optimized for Database components.
     
   * Transparent sharding (Citus)
    
     Shared-nothing arch, quite similar to Elasticsearch. 
     
   * SQL over distributed key-value stores
     
     TiDB/CockroachDB/YugabyteDB
     
     Though I think they are also using netowrk storage to become cloud-native.