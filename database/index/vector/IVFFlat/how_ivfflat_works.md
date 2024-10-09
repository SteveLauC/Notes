# Context

```
create table foo (str text);
insert into foo values ('hello');
insert into foo values ('greet');
insert into foo values ('hate');

select * from foo where  embedding_column <=> '[1,2]' limit 4 # ann(str, 'hi', 4);
hello
greet
```

# What does the execution plan look like without index

```
TopK
Calulate similarity
TableScan
```

# What does the execution plan look like with index

```
IndexScan
```



1. Devide the vector into multiole lists, each list is known as a cluster
2. Then build an inverted index for every list, mapping the list to the vectors that contain it


3. When searching the 

hello   
greet   
hate     

[hello, greet] (inverted index, centroid -> vector lists)
[hate]

searching "work"

one approach is to search not only the region of the closest centroid but also the regions of the next closest R centroids.

# Index building parameters

the # of clusters

# Query-time parameter 

Probe - the number the clusters that should be searched besides the hit cluster

# Questions

1. When a query comes in, the nearest clusters to the query are identified and only the vectors in those clusters are searched. 

how