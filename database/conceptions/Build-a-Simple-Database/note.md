# MemTable

1. To delete an entry from the MemTable, we simple update that entry to a 
   special value `tombstone` if exists in the MemTable.

   If it does not exist in the MemTable, then it has been flushed to the
   SST, we should insert a new entry with value set to `tombstone` to the
   MemTable.

# WAL

1. WAL is in sync with MemTable, when the corresponding Memtable is flushed
   to the disk, then that WAL(on disk) will be wiped and make room for the 
   new one.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/relation-between-MemTable-WAL-and-SST.png)

2. Format of RocksDB WAL 

   ```text
   +----------+-----------+-----------+----------------+--- ... ---+
   | CRC (4B) | Size (2B) | Type (1B) | Log number (4B)| Payload   |
   +----------+-----------+-----------+----------------+--- ... ---+
   CRC = 32bit hash computed over the payload using CRC
   Size = Length of the payload data
   Type = Type of record
          (kZeroType, kFullType, kFirstType, kLastType, kMiddleType )
          The type is used to group a bunch of records together to represent
          blocks that are larger than kBlockSize
   Payload = Byte stream as long as specified by the payload size
   Log number = 32bit log file number, so that we can distinguish between
   records written by the most recent log writer vs a previous one.
   ```
