1. `cluster.routing.allocation.enable`

   Enable or disable allocation for specific kinds of shards.

   Available options:

   * all - (default) Allows shard allocation for all kinds of shards.
   * primaries - Allows shard allocation only for primary shards.
   * new_primaries - Allows shard allocation only for primary shards for new indices.
   * none - No shard allocations of any kind are allowed for any indices.

2. `cluster.routing.allocation.same_shard.host` (bool) 

   If true, forces the replicas of the same primary shard to be allocated on
   different nodes.

3. 