1. Even though Postgres is not a distributed database, it still has a concept of 
   cluster, e.g., command `initdb` is used to create the database cluster.
   
   Within this cluster, you can have multiple server to make the database highly
   available or balance the workloads. 
   
2. By having multuple nodes, Postgres only does replication, no distribution
   (partition the table and send different parts to different nodes), even though
   partitioning does eixst in Postgres.
   