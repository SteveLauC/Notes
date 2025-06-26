1. Read amplification

2. Write amplification

   Write amplification is introduced by compaction, the key-value pairs within
   an SST file is sorted, but the kv pairs across all the L0 SSTs are not sorted
   and can overlap, which would make read inefficient.

   So you do compaction to sort them, which will write the data that has already
   been written to disk again. This is write amplification.
   
   