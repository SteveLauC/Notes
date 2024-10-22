> This chapter is about:
>
> * Before I read it
>   * Introduction to verious data models, their pros and cons.
>   * Introduction to verious query languages, their pros and cons.
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
> 
>   * The pros and cons of various data models and query languages
>   * The history of data models and query languages

> What have you learned from it
>
> *
> *


> TOC
>
> * Relational Model vs Document Model
>   * The Birth of NoSQL
>   * The Object-Relational Mismatch
>   * Many-to-One and Many-to-Many Relationships
>   * Are Document Databases Repeating History?
>   * Relational vs Document Databases Today
> * Query Languages for Data
>   * Declarative Queries on the Web
>   * MapReduce Querying
> * Graph-Like Data Models
>   * Property Graphs
>   * The Cypher Query Language
>   * Graph Queries in SQL
>   * Triple-Stores and SPARQL
>   * THe foundation: Datalog
> * Summary

1. The limits of my language mean the limits of my world.

   Both data model and query language affect the way you think.
   
2. For an application developer, data model is probably the first thing you need
   to consider and choose when building your application, not only because it
   affect how you think, but **it decides what you can do and cannot do**.

# Relational Model vs Document Model

1. The reason why relation databaeses won during the databsae competition was that
   it trid to hide the implementation details under a clean interface, while other
   databases at that time require you to think about the underlying storage details.
   
## The Birth of NoSQL

1. NoSQL is the last attempt to overthrow the dominance of relational model. 

2. The term `NoSQL` was originally a Twitter hashtag for an open-source, nonrelational
   database meetup in 2009.
   
3. What drives the NoSQL trend

   1. Perple want something that is web-scale
   2. Dynamic, flexible schema support
   3. Preference for open source software over commercial database products
      
      > This is intersting, Postgres 9 was released at that time. And it is open source
      
   4. People need some specialized query operations that are not welled supported by
      relational model

## The Object-Relational Mismatch



## Many-to-One and Many-to-Many Relationships
## Are Document Databases Repeating History?
## Relational vs Document Databases Today
# Query Languages for Data
## Declarative Queries on the Web
## MapReduce Querying
# Graph-Like Data Models
## Property Graphs
## The Cypher Query Language
## Graph Queries in SQL
## Triple-Stores and SPARQL
## THe foundation: Datalog
# Summary

