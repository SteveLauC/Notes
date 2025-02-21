# ABSTRACT

1. > In particular, we foucs on safe techniques for disjunctive queries.

   QUES: figure out what WAND is good at? 
   
# 1. INTRODUCTION

1. > One interesting property of our algorithms is that they performs 
   > document-at-a-time (DAAT) index traversal, based on either 
   > document-sorted or impact-layer index structure
   
   NOTE that this algorithm requires that your index posting list is ordered 
   by document ID.

# 2. BACKGROUND
## 2.1 Inverted Indexes and Index Compression

1. > As the lists for common terms could be very long, we want to be able to 
   > skip most parts of the lists during query processing. To do so, inverted 
   > lists are often split into blocks of, say, 64 or 128 docIDs, such that 
   > each block can be decompressed separately. 
   
   NOTE that for decompression flexibility, posting lists are already chunked.
   The diagram in `Performance_of_compressed_inverted_list_caching_in_search_engines_2008`
   depicts this.
   
   > **To do so, we have an extra  table, which stores for each block the uncompressed maximum (or minimum) 
   > docID and the block size in this table**.
   
   QUES(solved): why is this needed?  Future steve: every block is fix-sized, 
   but contain different range of IDs, e.g., block 1 could be `[1, 9, 10]`, 
   block 2 is `[11, 100, 9999]`.
   
## 2.2 Query Processing


1. > current web search engines use ranking functions based on hundreds of 
   > features. Such functions are quite complicated and fairly little has been 
   > published about how to efficiently execute them on large collections. 
   >
   > One "folklore" approach separates ranking into two phases:
   >
   > In the first phase, a simple and fast ranking function such as BM25 is used 
   > to get, say, the top 100 or 1000 documents. 
   >
   > Then in the second phase a more involved ranking function with hundreds of 
   > features is applied to the top documents returned from the first phase. 
   >
   > As the second phase only examines a small number of top candidates, a significant 
   > amount of the computation time is still spent on the first phase. **In this paper we 
   > focus on executing such a simple first-phase function**, say BM25, a problem 
   > that has been extensively studied in the literature
   
   So how ranking works in a web search engine is quite different from what
   search engine software does.



2. > In general, disjunctive queries have traditionally been used in the IR 
   > community while web search engines have often tried to employ conjunctive 
   > queries as much as possible. 
   >
   > One reason is that disjunctive queries tend to be significantly (by about
   > an order of magnitude for exhaustive query processing) more expensive than 
   > conjunctive queries, as they have to evaluate many more documents.
   
   Interesting point
   
3. > To traverse the index structure, there are two basic techniques, 
   >
   > 1. Document-At-A-Time (DAAT) 
   > 2. Term-At-A-Time (TAAT)
   >
   > For conjunctive queries, DAAT is often preferred, while many optimized 
   > approaches for disjunctive queries use TAAT.
   
   QUES: figure out why? I think I have to understand how TTAT works
   
   The below paragraph comes from section *2.5 Index Traversal Techniques*:
   
   > For conjunctive and exhaustive query execution, DAAT is very fast and considered 
   > state of the art (at least for queries with a moderate number of queries terms), 
   > whereas TAAT-type methods are often bottlenecked by the nontrivial data structures. 
   > However, for disjunctive queries it is hard to integrate early termination and 
   > exploit layered indexes with DAAT. Thus, for this case most early termination 
   > algorithms in the literature are based on TAAT that use impact-sorted or 
   > layered indexes.
   
   
   
## 2.3 Early Termination Algorithms

1. We say that a query processing algorithm is *exhaustive* if it fully evaluates
   all documents that satisfy the Boolean filter condition. Any non-exhaustive 
   algorithm is considered to use *early termination (ET)*. 
   
   > Every `ORDER BY` and `LIMIT` query optimzed using heap is exhausive, I 
   > guess?
   
   *safe early termination*: Your index should return the same result as if it was
    using the *exhaustive* approach, i.e., not an approximate approach.
    
2. > Also, we focus on memory-based indexes, as for example considered in [30, 17],

   [30] T. Strohman and W. Bruce Croft. Efficient document retrieval in main 
   memory. In Proceedings of the 30th Annual International ACM SIGIR Conference 
   on Research and Development in Information Retrieval, 2007.
   
   [17] Jeffrey Dean. Challenges in building large-scale information retrieval 
   systems. In Proceedings of the Second ACM International Conference on Web 
   Search and Data Mining, 2009.
   
   
## 2.4 Index Organizations 

Many existing techniques for early termination from the DB and IR communities 
are based on the idea of reorganizing the inverted index (posting list) such 
that the most promising documents appear early in the inverted lists.

* Document-Sorted Indexes

  Posting list is sorted by document ID, Lucene and Tantivy follow this 
  approach.

* Impact-Sorted Indexes

  > Postings in each list are sorted by their impact, that is, their contribution 
  > to the score of a document. Postings with the same impact are sorted by document 
  > ID. Note that this assumes that the ranking function is decomposable (i.e., a 
  > sum or other simple combination of per-term scores), which is true for Cosine, 
  > BM25, and many other functions in the literature.
  
  
  QUES: For the BM25 ranking function, a term's score will change if there are
  document updates, so the calculated impact score is temporarily valid, how
  is this issue resolved?  

* Impact-Layered Indexes

  > We partition the postings in each list into a number of layers, such that all 
  > postings in layer i have a higher impact than those in layer i + 1, and then 
  > sort the postings in each layer by docID.
  
QUES: figure out how Lucene and Tantivy do early termination with Document-Sorted 
Indexes.
  
## 2.5 Index Traversal Techniques

1. > However, for disjunctive queries it is hard to integrate early termination 
   > and exploit layered indexes with DAAT. Thus, for this case most early 
   > termination algorithms in the literature are based on TAAT that use 
   > impact-sorted or layered indexes. In this paper we challenge this assumption 
   > and suggest DAAT algorithms may actually do better for early termination even 
   > in the case of disjunctive queries.
   
   This paper thinks that DAAT is pretty good at conjunctive queries even with
   exhausive query execution, i.e., no early termination.
   
   And, as of its writing, there is no early termination algorithms for DAAT, this
   paper is probably the first one for early termination and disconjunctive queries..

## 2.6 Two State-of-the-Art Techniques


# 3. OUR CONTRIBUTION

1. > Recall that WAND stores the maximum impact for each inverted list. Our initial 
   > insight is that skipping in WAND is limited because it uses the maximum impact
   > scores over the entire lists, which can be much larger than average.
   
   
2. > We show how to extend our techniques to layered indexes, reordered 
   > indexes, and **conjunctive query processing**
   
   QUES: does this mean that block-max WAND is only suitable for disconjunctive
   queries by default?
   
# 4. RELATED WORK

# 5. BLOCK-MAX WAND ALGORITHM