> This chapter is about:
>
> * Before I read it
>   * Introduction to verious data models, their pros and cons.
>   * Introduction to verious query languages, their pros and cons.
> * After I read it
>   * Basic introduction to verious data models, qls, their pros and cons.
>     * Relational model
>     * Document model
>       * MongoDB's pipeline 
>     * hierarchical model
>     * network model
>     * graph model
>       * Cypher
>       * SPARQL
> * One-to-many relationships 
> * Many-to-one relationships 
> * Many-to-many relationships 

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
> 
>   * The pros and cons of various data models and query languages
>   * The history of data models and query languages

> What have you learned from it
>
> * The history of NoSQL
> * The definitions of one-to-many, many-to-one, many-to-many relationships
> * A general intro to graph databases 


> TOC
>
> * Relational Model vs Document Model
>   * The Birth of NoSQL
>   * The Object-Relational Mismatch
>   * Many-to-One and Many-to-Many Relationships
>   * Are Document Databases Repeating History?
>   * Relational vs Document Databases Today
>     * Convergence of Document and relational databases
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
      
      > Does the scale issue stil exist with the share-disk architecture?
     
   2. Dynamic, flexible schema support
   
      > Most relational databases still lack this feature today (2024)
      
   3. Preference for open source software over commercial database products
      
      > This is intersting, Postgres 9 was released at that time. And it is open 
      > source
      
   4. People need some specialized query operations that are not welled supported by
      relational model
      
      > Most relational databases support the JSON type

## The Object-Relational Mismatch

1. TIL, what does ORM mean? It is object-relational mapping, since writing SQLs directly
   in your code would not be a good experience, people inverted this mapping tool to 
   mitigate the mismatch.

   The term "object" means "object-oriented", becase the popular programming languages
   back in the day are mostly OOP, technically Rust is not OOP, but I guess people just
   use this whole ORM term to mean the database connection libraries.
   
2. Document data model is good at one-to-many relationship, and the relational 
   model (before adding support for types like array and json), cannot handle
   this easily, normalization and split the data across multiple tables are 
   necessary.
   
   ```
   {
       "name": "foo",
       "work_experience": ["job1", "job2"]
   }
   ```
   
   For example, for the above document, to express it in the relational model,
   you have to define 3 tables:
   
   ```sql
   create table workers (id int, name text);
   create table job (id int, title text);
   create table work_experience (work_id int, job_id int);
   ```
   
   And the `work_expreience` table will be something like:
   
   ```
   foo's ID job1's ID
   foo's ID job2's ID
   ```
   
   Well, almost all the relational databases have array now, so you can just do:
   
   ```
   create table workers (name text, work_experience text []);
   ```

## Many-to-One and Many-to-Many Relationships

1. Example of many-to-one relationship

   If a person is only allowed to have one job, then a company will have many
   employees.

2. Example of many-to-many relationship

   A student can take many courses, a course will have mnay attended students.
   
3. Document data model can naturally express one-to-many relationship, but it 
   cannot handle many-to-one and many-to-many relationships.
   
   > Well, relation model cannot express them naturally either.
   
   **Normalization and join are necessary** to handle such relationships, for both
   document model and relational model.
   
4. Document databases are weak at join. But they are trying to solve the issue.
   
5. You can see that document databases and SQL databaes are unifying.

## Are Document Databases Repeating History?

1. There was a hierarchical data model, which is similar to JSON, can express
   one-to-many relationships naturally. But it cannot express many-to-one and
   many-to-many relationships either.
   
   ![illustration](https://github.com/SteveLauC/pic/blob/main/hierarchical_model_10_24_2024.png)
   
   There are various solutions to solve the issue of hierarchical data model,
   among the which, the 2 most prominent were the:
   
   * relational model (using normalization and join)
   * network model 
   
2. Network Model

   ![illustration](https://github.com/SteveLauC/pic/blob/main/network_model_20_24_2024.png)
   
   Different from the hierarchical model, it allows a node to have multiple parents.
   
   It can express many-to-one relationship, both B1 and B2 are employees, C4 is 
   their company. And it can express many-to-many relationship, e.g., student B1
   takes courses C1, C2, C3 and C4, and course C3 has students D1, D2, D3, D4.
   
   > QUES(Solved): Difference between network model and graph model
   >
   > [SO: What is the difference between a Graph Database and a Network Database?][link]
   >
   > [link]: https://stackoverflow.com/q/5040617/14092446
   >
   >
   > 1. CODASYL (the ql of network model) is imperative, but the qls for graph
   >    databases are often declarative.
   > 2. In the netwrok model, to reach a record, you have to traverse its access 
   >    path, while in graph model, you can access any vertex by a unique ID.
   > 3. In the network model, the database had a schema that specified which record 
   >    type could be nested within which other record type. In a graph database, 
   >    there is no such restriction: any vertex can have an edge to any other vertex. 
   >    This gives much greater flexibility for applications to adapt to changing 
   >    requirements.
   > 4. In the network model, the children of a record were ordered, in graph 
   >    databases, vertices and edges are not ordered.
   >
   > In conclusion, graph database is more easy to use and flexible.
   
3. Access Path

   > In SQL databases, access path describes how you access the data, either by a
   > TableScan or IndexScan. I think this term comes with Network model.
   
   In the network model, in order to access a node, you have to follow a path from
   a root record along these links, this was called an access path. When usd to 
   express many-to-many relationships, you usually have many access paths to the 
   same node, programmer working with the network model has to keep track of these
   different access paths in their head, and manually pick up one that you think
   it is good, this makes application code complcated and hard to maintain.
   
4. Network model lets the programmer to choose the access path, while relational
   model offloads the task to the database system. This is the most likely reason
   why relation model winned the competition.

## Relational vs Document Databases Today

Today relational model and document model are basically same except for different
schema flexibility. Most document databases don't force its data to have a fixed 
schema, and thus they are usually called "schemaless", but schemaless is misleading,
they do have scheams, when code reads the data, a schema will be assumed, so it
is better to be called "schema-on-read". Relational databases are different, they
request you to set the shema when creating tables, and verification will be done
on write, thus "schema-on-write".

Rockset, as a SQL database, is "schema-on-write".

### Convergence of Document and relational databases

1. Relational databases added the support for JSON type

2. Document databases added support for join to solve the many-to-one and 
   many-to-many problems
   
3. And vector database is also joining the convergence party (in 2024)

# Query Languages for Data

1. SQL is a declarative language, a declarative language does not only make 
   application code easier to write, but also allows the database system to 
   optimize the performance **without breaking application code**.
   
   It is a layer of abstraction that hides the implementation details of database
   system.
   
## Declarative Queries on the Web

1. CSS is also a declarative language

## MapReduce Querying

> The book says: "MapReduce is niehter a declarative query language nor a fully
> imperative query API, but somewhere in between"
>
> It is indeed surprising to see MapReduce here

1. MapReduce is a programming model popularized by Google, many NoSQL databases
   that shard data into many partitions also adopt limited form of MapReduce.
   
   > Elasticsearch's distributed query execution is basically MapReduce.

# Graph-Like Data Models

> Postgres has an extension that turns it into a graph database: Apache AGE

1. Relational model can hanlde many-to-many relationships, but the support is
   limited, when the connection between entries get much more complicated, graph
   model should be used instead.
   

## Property Graphs
   
1. For graph model, there are 2 types of graph data model:

   1. Property graph model
      * Neo4j
      * Titan
      * InfiniteGraph
   2. triple-store model
      * Datomic
      * AllegroGraph

2. In the property graph model

   each vertex consists of:
   
   1. A unique identifier
   2. A set of incoming edges
   3. A set of outgoing edges
   4. A collection of properties (key-value pairs)
   
   Each edge consists of:
   
   * A unique identifier
   * The vertex at which this edge starts
   * The vertex at which this edge ends
   * A label to describe the kind of relation between 2 vertices it connects
   * A collection of properties (key-value pairs)
   
   Considering this, you can think of a graph store as 2 relational tables, one
   for vertices, one for edges:
   
   ```sql
   create table vertices (
       vertex_id integer primary key,
       properties json
   );
   
   create table edges (
       edge_id integer primary key,
       start_vertex integer references vertices (vertex_id),
       end_vertex integer references vertices (vertex_id),
       label text,
       properties json,
   );
   
   -- To be able to locate of incoming and outgoing edges for a given vertex quickly
   -- incoming edges: select edge_id from edges where end_vertex = {GIVEN VERTEX}
   -- outgoing edges: select edge_id from edges where start_vertex = {GIVEN VERTEX}
   crate index edges_start on edges (start_vertex);
   crate index edges_end on edges (end_vertex);
   ```
   
   Some important aspects of this model are:
   
   1. Any vertex can have an edge connecting it with other vertices. There is 
      no schema that restricts which kinds of vertices can or cannot be associated.
      
      > QUES: Could triggers be used for this?
      
   2. Given any vertex, you can efficiently find both its incoming and its outcoming
      edges, and thus traverse the graph.
   

## The Cypher Query Language

1. The cypher query language is a declarative query language created for property
   graphs, created by Neo4j.
   
   > It is named aftere a character in the movie The Metrix
   
## Graph Queries in SQL

1. For our SQL tables defined above, we can do graph queries in SQL, but it would
   be quite clumsy. Postgres AGE provides a `cypher()` function, which allows you
   to write Cyper query language in SQL.

## Triple-Stores and SPARQL

1. Triple-stores is another type of graph model, it stores every thing in aa 
   triple (subject, predicate, object), e.g., `(foo, lives_with_in, U.S.)`
   
   The "subject" is equivalent to a vertex in a graph, what the "predicate" and
   "object" are depends on what the "object" is:
  
   1. If "object" is a primitive type literal, then "predicate" and "object" behave
      like an property of "subject".  
      
   2. If "object" is another vertex in the graph, then "predicate" will be an edge
      in the graph.
      
2. SPARQL predates Cypher, and Cyper's pattern matching was borrowed from SPARQL, 
   so they are quite similar.
   
   I kinda like the syntax of SPARQL.

## THe foundation: Datalog

Datadog is a query language, not that company.

# Summary

