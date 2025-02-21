# 1. INTRODUCTION

1. The way described in this paper consists of 2 phases:

   1. Iterates in parallel over query term postings and identifies candidate 
      documents using an *approximate evaluation* (cheap) taking into account only 
      partial information on term occurrences and no query independent 
      factors.
      
      
      In this phase, *approximate evaluation* allows us to skip large portions
      of postings.
      
   2. Promising candidates are *fully evaluated* and their exact scores are 
      computed.
      
   And this paper mentions this kind of 2-phase execution is common in other 
   fields as well, e.g., sideway information passsing.
   

## 1.2 Our approach

1. The intention of this 2-level process is to minimize the number of full
   evaluations as much as possible by pruning as much as possible in the
   first level.
   
# 2. THE TWO-LEVEL EVALUATION PROCESS

## 2.1 Basic assumptions

1. > Our model assumes a traditional inverted index for IR systems in which 
   > every index term is associated with a posting list.
   
   Interesting, what else structures do we have? Postgres GIN index looks kinda
   different, but it is still the traditional inverted index, I guess.
   
   
2. The posting list should support the following operators:

   1. It should behave like an iterator
   2. `next(id)`, move the cursor of the iterator to the element whose document ID
      is greater than or eq to `id`.
      
      If no such document exists, the iterator returns a special posting element.
   
## 2.3 Scoring

1. I do understand how the general algorithm works by reading this section.
   And I agree that 
   
   > WAND is limited because it uses the maximum impact scores over the entire 
   > lists, which can be much larger than average.
   
   as mentioned in `Faster_Top-k_Document_Retrieval_Using_Block-Max_Indexes_2011.md`.