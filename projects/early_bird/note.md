# Earlybird: Real-Time Search at Twitter

> * Requirements of real-time search
> * Overview of Earlybird
> * Detailed discussion of index organization
> * Concurrency model

## Requirements of real-time search

1. Concurrent reads and writes

   Index strucuture must be continuously updated as documents are ingested,
   while the same index structure are accessed to serve query.

2. 
