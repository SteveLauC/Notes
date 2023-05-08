# Introduction

1. LSM is a data structure to **maintain key value pairs**.

2. Once the contents in the MemTable are batched into the SSTable and written
   to the disk, it becomes immutable and thus won't be changed.

   > What if I wanna update a kv pair that exists in the SSTable?
   >
   > Construct a new kv pair and insert it to the MemTable.

3. An LSM tree can balance between read, write and space amplification by changing
   the compaction algorithm. The data structure itself is super versatile and 
   can be optimized for different workloads.

4. MemTable can be seen as some kind of buffer for buffered I/O. (write to the
   disk in batches)

# Conponents

1. Write-ahead log (WAL) to persist temporary data for recovery.
2. SSTs on the disk for maintaining a **tree** structure.
3. Mem-tables in memory for batching small writes.

# APIs

1. `put(key, value)`: store a kv pair in the LSM tree
2. `delete(key)`: remove a key and its corresponding value
3. `get(key)`: get the value corresponding to a key
4. `scan(range)`: get a range of kv paris (iterators)
5. `sync()`: ensure all operations are flushed to the disk

# Write Flow

![diagram](https://github.com/SteveLauC/pic/blob/main/lsmt-write-flow.jpeg)

1. Write the kv to WAL
2. Write the kv to memtable

   > After (1) and (2) completes, we can notify the user that the write operation
   > is completed.

3. When a memtable is full, flush it to the disk as an SST file **in the 
   background**
4. Compaction **in the background**

# Read Flow

![diagram](https://github.com/SteveLauC/pic/blob/main/lsmt-read-flow.jpeg)

1. Try to find the key in memtable (new to old) (binary search/sorted)
2. If the key is not found, search the SST (top layer to bottom layer, why)
