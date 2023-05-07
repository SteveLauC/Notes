> # 24.1 Bloom Filter
> # 24.2 Log-Structured Merge Tree and Variants
> # 24.3 Bitmap Indices
> # 24.4 Indexing of Spatial Data
> # 24.5 Hash Indices


# 24.1 Bloom Filter
# 24.2 Log-Structured Merge Tree and Variants
1. B+Tree is not suitable for workload where there are a very number of writes.

2. The key idea of LSM is to replace random I/O in CRUD with a smaller number
   of sequential I/O.

3. MemTable

   This memtable is implemented using B+Tree

# 24.3 Bitmap Indices
# 24.4 Indexing of Spatial Data
# 24.5 Hash Indices
