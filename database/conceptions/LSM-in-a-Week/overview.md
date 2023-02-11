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
3. When a memtable is full, flush it to the disk as an SST file **in the 
   background**
4. Compaction **in the background**

# Read Flow

![diagram](https://github.com/SteveLauC/pic/blob/main/lsmt-read-flow.jpeg)

1. Try to find the key in memtable (new to old)
2. If the key is not found, search the SST (top layer to bottom layer, why)
