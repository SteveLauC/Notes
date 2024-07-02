1. `cluster.routing.allocation.allow_rebalance`

   Specify when shard rebalancing is allowed, available options:

   * always - Always allow rebalancing
   * indices_primaries_active - Only when all primaries in the cluster are allocated
   * indices_all_active - (default) Only when all shards (primaries and replicas) in the cluster are allocated

2. `cluster.routing.rebalance.enable`

   Enable or disable rebalancing for specific kinds of shards

   * all - (default) Allows shard balancing for all kinds of shards.
   * primaries - Allows shard balancing only for primary shards.
   * replicas - Allows shard balancing only for replica shards.
   * none - No shard balancing of any kind are allowed for any indices.

3. 