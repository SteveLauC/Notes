1. This API is only callable if 

   * The target index is read-only
   * The cluster health is green

2. What will this API do

   1. Create a new index with the the specified new settings
   2. Hard link (at fs level) the segments file of the previous index to the 
      segments of this new index.

      > If hard link is not supported by the underlying fs, then a plain copy
      > operation will be performed.

   3. Rehash the documents of the old index
   4. Recovers the target index as though it were a closed index which had just
      been re-opened.
