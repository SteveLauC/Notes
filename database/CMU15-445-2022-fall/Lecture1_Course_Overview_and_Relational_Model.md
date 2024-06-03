1. what is database?

    organized collection of inter-related data that models some aspect of the 
    real world

2. flat file storage problems
    1. data integrity
    * How can we ensure that the artist is the same for each album release
    * What if someone overwrites the album year with an invalid string
    2. implementation perspective
    * parser program is complicated to impl
    3. durability
    * what if your machine crashes when updating a record

    > That's why we need DBMS. So we don't need to handle this complexity in our
    application code

2. data model and schema

    data model: a high level abstraction; A collection of concepts to describe the
    data stored in a database

    * SQL DB
      * rational(MySQL...)
    * NoSQL DB
      * key value(skytable)
      * graph(neo4j)
      * Document(es/mongodb)/object-oriented
    * Vector DB
      * For Machine Learning (Vector databases like Milvus)

    schema: defines exactly what we are going to store in a database

3. DML
    * Procedural: how
      Relational Algebra
    * Declarative: what
      SQL

    > Why is SQL a declarative QL?
    >
    > Because it does not contain the execution detail, just the result we want. 
    >
    > Let's give an example:
    > For a query like "Retrieve the joined tuple from R and S where S.id=102", 
    > in Relational Algebra, we can do:
    >
    > 1. σ id=102(R ▷◁ S)
    > 2. R ▷◁ (σ id=102(S))
    >
    > They have the same effect but can have totally different performance, say there
    > are millions of tuples in relation S but only one tuple satisfies the restriction
    > that `id=102`, expression 1 will be much more slower than the expression 2. 
    > If we use expression 1, then it will just execute like that, no optimization.
    > 
    > In SQL, we can just do `SELECT * FROM R, S WHERE S.id = 102`, we don't specify
    > whether it will do the filter first or join first. It is the DBMS that thinks
    > plan 2 is much more efficient and chooses to go with it.
    >
    > And even you write the SQL in the way Relational Algebra is written, we still
    > have the query optimizer to can alter the statements.


4. comparison between our python query code(physical level highly related) and SQL

   ![pic](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-02%2015-47-21.png)

   
