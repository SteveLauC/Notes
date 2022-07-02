1. what is database?

    organized collection of inter-related data that models some aspect of the 
    real world

2. flat file storage problems
    1. data intergrity
    * How can we ensure that the artist is the same for each album release
    * What if someone overwrites the album year with an invalid string
    2. implementation perspective
    * parser program is complicated to impl
    3. durability
    * what if your machine crashes when updaing a record

    > That's why we need DBMS. So we don't need to handle this complexity in our
    application code

2. data model and schema

    data model: a high level abstraction; A collection of concepts to describe the
    data stored in a database
        * rational
        * key value
        * graph
        * object-oriented

    schema: defines exactly what we are going to store in a database

3. DML
    * procedural: how
    * declarative: what

4. comparison between our python query code(phycial level highly related) and SQL

   ![pic](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-02%2015-47-21.png)

   
