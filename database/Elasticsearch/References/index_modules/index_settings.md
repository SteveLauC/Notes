1. Index settings can be static or dynamic

   * static
      
     Can be only set at index creation time or a closed index.

     > If you want to update it for an open index, set it with `?reopen=true`, it
     > will close the index, update the settings and reopen the index under the
     > hood.
     
   * dynamic

     Can be changed on a live index

2. Static settings

   * index.number_of_shards

     The number of primary shards in this index, defaults to 1 if not set.

     This can NEVER be updated, even on a closed index. As a workaround, Elasticsearch
     allows you to [split an index](../REST_APIs/Index_APIs/Split_index_API).

     Limit: the soft limit for this setting is 1024, this soft limit can be 
     adjusted via `export ES_JAVA_OPTS="-Des.index.max_number_of_shards=128"`

   * index.number_of_routing_shards

     Used with the Index split API, and can also be used with documents routing.

     QUES: I don't quite understand this.

   * index.codec
      
     Compression algorithm used on the underlying segments file

   * index.routing_partition_size

     When this setting is set, the default document routing algorithm will be changed:

     ```
     routing_value = hash(_routing) + hash(_id) % routing_partition_size
     shard_num = (routing_value % num_routing_shards) / routing_factor
     ``` 

     See [_routing_field](../Mapping/Metadata_fields/_routing_field.md) for the
     default routing algorithm.

   * index.soft_deletes.retention_lease.period
   * index.load_fixed_bitset_filters_eagerly
   * index.shard.check_on_startup

3. Dynamic settings

   * index.number_of_replicas

     The number of replica shards each primary shard has, default to 1. You can
     set this to 0 to disable replica, at the risk of losing data when node 
     crashes.

   * index.auto_expand_replicas

     Automatically expand the number of replica shards based on the number of 
     nodes in the cluster.

   * index.search.idle.after

     > This setting ONLY works if `index.refresh_interval` is NOT explicitly set. 

     Elasticsearch refreshes every `index.refresh_interval` seconds, this may
     not be necessary if there is no search/get requests.

     This setting optimize the refresh in this way, for the `index.search.idle.after`
     seconds after a search/get request, the index will be considered idle and
     thus won't be refreshed (until a search/get request is received).

     If a search/get operation happens during idle stage, then this search will
     refresh the shard, then search.

   * index.refresh_interval
     
     How often the refresh operation will be. Defaults to 1s.

   * index.max_result_window

     If you have that much of documents, then `from + size` can not exceeds this 
     setting.