1. Settings in Elasticsearch divides into 2 categories:

   1. Static

      Static settings cannot be changed without reindexing and closing the index.

      * number_of_shards
      * codec
   
   2. Dynamic

      Dynamic settings can be updated on the fly.

      * number_of_replicas: how many replicas every primary shard should have
      * refresh_interval

2. Template is supported by ES so that one can reuse the setting when creating
   new indices.
    
   ```
   PUT _template/template_1
   {
     "index_patterns" : ["te*", "bar*"],
     "settings" : {
       "number_of_shards" : 1
     },
     "mappings" : {
       "_source" : { "enabled" : false }
     }
   }
   ```

   ES has tons of built-in templates for integration with other products.


