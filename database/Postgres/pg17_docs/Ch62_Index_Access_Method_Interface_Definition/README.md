1. The documentation says:

   > All indexes in PostgreSQL are what are known technically as *secondary 
   > indexes*; that is, the index is physically separate from the table file
   > that it describes. Each index is stored as its own physical relation.
   
   It is very interesting to see the definition of secondary index here: an
   index which is physically separated from the data storage.
   
   Database system concepts defines secondary index as: 
   
   > Index whose `Search key` specifies an order different from the phycial
   > order is called `Nonclustering Index`, and is also called `Secondary Index`.
   
   Well, if we choose the def used by the textbook to interprete the Postgres
   doc, it actually makes sense, the default table am in Postgres is heap, and
   the tuples in heap are not ordered, so all indexes's order will be different
   from the physical data order.
   
2. In practice, all index access methods divide indexes into standard-size pages 
   so that they can **use the regular storage manager and buffer manager to access 
   the index contents**. (But this is not forced)
   
   This is intersting, 
   [the disk format of pgvector HNSW index is page-based.](../../../pgvector/Understanding_pgvector_HNSW_Index_Storage_in_Postgres.md)
   
3. An index maps some key data (that comes from query) to a tuple identifier of
   a specific version (due to MVCC).
   
   > Indexes are not directly aware that under MVCC, there might be multiple 
   > extant versions of the same logical row; to an index, each tuple is an 
   > independent object that needs its own index entry. Thus, an update of a
   > row always creates all-new index entries for the row, even if the key 
   > values did not change.
   
   QUES: I am not quite sure I understand the above. Looks like it is saying
   that when an update happens, a new index entry for the updated tuple 
   (version + 1) will be created as well, so index entry and tuple are still
   in sync. 