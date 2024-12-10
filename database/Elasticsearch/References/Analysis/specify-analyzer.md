> url: https://www.elastic.co/guide/en/elasticsearch/reference/current/specify-analyzer.html

1. There are 2 kinds of analyzer, used in different processes

   1. Index analyzer (`analyzer`)
   2. Search analyzer (`search_analyzer`)

# How ES determines the **index** analyzer (Priority)

1. Analyzer mapping parameter set for the field
   
   ```
   PUT my-index-000001
   {
     "mappings": {
       "properties": {
         "title": {
           "type": "text",
           "analyzer": "whitespace"
         }
       }
     }
   } 
   ```
   
2. index-level default setting `analysis.analyzer.default`

   ```
   PUT my-index-000001
   {
     "settings": {
       "analysis": {
         "analyzer": {
           "default": {
             "type": "simple"
           }
         }
       }
     }
   }
   ```
   
3. Use the standard analyzer

# How ES determines the **search** analyzer

1. The `analyzer` parameter set in the query

   > This parameter is supported in:
   >
   > * Match query
   > * Phrase query
   
   ```
   GET my-index-000001/_search
   {
     "query": {
       "match": {
         "message": {
           "query": "Quick foxes",
           "analyzer": "stop"
         }
       }
     }
   }
   ```
   
2. The `search_analyzer` set in the mapping

   ```
   PUT my-index-000001
   {
     "mappings": {
       "properties": {
         "title": {
           "type": "text",
           "search_analyzer": "simple"
         }
       }
     }
   }
   ```
   
3. Index-level default setting `analysis.analyzer.default_search`
   
   ```
   PUT my-index-000001
   {
     "settings": {
       "analysis": {
         "analyzer": {
           "default_search": {
             "type": "whitespace"
           }
         }
       }
     }
   }
   ``` 
   
4. The `analyzer` set in the mapping
5. Use the `standard` analyzer

