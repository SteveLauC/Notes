1. How documents are routed to a shard

   $$ routing\_factor = index.number\_of\_routing\_shards/index.number\_of\_shards  $$
   $$ shard\_num = (hash(\_routing) \mod index.number\_of\_routing\_shards ) \times routing\_factor $$

2. The `_routing` field can be given when creating documents

   ```sh
   POST /<target>/_doc/<id>?_routing=<_routing>
   { ... }
   ```