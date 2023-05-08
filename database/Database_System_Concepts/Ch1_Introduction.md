#### Ch1: Introduction


> Sections labeled with `*` are IMPORTANT.
>
> ##### 1.1 Database-System Applications
>
> Brief introduction to the usage cases of DB
>
> ##### 1.2 Purpose of Database Systems
>
> Advantages over file system
>
> ##### 1.3 View of Data
>
> ###### 1.3.1 Data Model
> ###### 1.3.2 Relational data model
> ###### 1.3.3 Data abstraction
> ###### 1.3.4 Instances and Schema
>
> ##### 1.4 Database Languages
> 
> ###### 1.4.1 DDL
> ###### 1.4.2 A specific DDL: SQL
> ###### 1.4.3 DML
> ###### 1.4.4 A specific DML: SQL
> ###### 1.4.5 Database Access from Application Programs
>
> ##### 1.5 Database Design
>
> How to design a great database schema, you can ignore this section if you
> want to learn the implementation detail of a DB for now.
>
> ##### 1.6 Database Engine
>
> ###### 1.6.1 Storage Manager
> ###### 1.6.2 Query Processor
> ###### 1.6.3 Transaction Management
>
> ##### 1.7 Database and Application Architecture
>
> ##### 1.8 Database users and Administrators
>
> ##### 1.9 History of Database Systems
>
> ##### 1.10 Summary


##### 1.1 Database-System Applications

1. Broadly speanking, there are two modes in which databases are used:

   1. (OLTP) The first mode is to support `online transaction processing`, where a large
      number of users use the database, with each user retrieving small amount of
      data, and performing small updates. This is the primary mode of use for the
      vast majority of users of database applications.

   2. (OLAP) The second mode is to support `data analytics`, that is, the processing
      of data to draw conclusions, and infer rules or decision procedures, which are
      then used to drive business decisions.

##### 1.2 Purpose of Database Systems

1. Advantages over file system

   1. Data redundancy and inconsistency
     
      > redundancy is the cause, inconsistency is the consequence.

   2. Difficulty in accessing data

      For every data access, you have to write a specific program to operate it.

      > In this perspective, `DML` such as `SQL` and `query engine` can be seen 
      > as a generic data processing program.

   3. Data isolation

      > I don't quite understand this

   4. Integrity problems

      It is hard to hold those constraints.

   5. Atomicity problems

      It is difficult to ensure atomicity in a conentional file-processing
      system.

   6. Concurrent-access anomalies

   7. Security problems
       

##### 1.3 View of Data

> A major purpose of a database system is to provide users with an *abstract*
> view of the data.

###### 1.3.1 Data Model

1. Some common `data model`s
  
   1. Relational Model

   2. Entity-Relationship Model

      ER model is widely used in database design.

   3. Semi-structured Data Model

   4. Object-Based Data Model

###### 1.3.2 Relational data model

###### 1.3.3 Data abstraction

1. Three levels of data abstraction

   > 有点数据库课上学的三级模式，两级映射的意思，但 1.3.4 节讲的 `phycial schema`
   > `logical schema` and `subshema` 才是内模式、模式和外模式。

   1. Phycial level

      > For DB developer

   2. Logical level

      > For DBA and DB designer

   3. View level

      > For end user

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_20221130-154039.jpeg)

###### 1.3.4 Instances and Schema

1. Three schemas

   1. Phycial Schema

      Describes the database design at the phycial level

   2. Logical Schema

      Describes the database design at the logical level

   3. Subschema
    
      Describues different views of the database.
   

   ![三级模式两级映射](https://github.com/SteveLauC/pic/blob/main/20221130-162820.jpeg)

##### 1.4 Database Languages

1. Database languages

   1. DDL
   2. DML

   > And in practice, DDL and DML are not two separate languages.

###### 1.4.1 DDL

1. DDL, used to specify the schema (logical schema, to be specific) and `
   integrity constraints`

2. The output of DDL is called `data directory`, which contains metadata. The
   `data directory` is considered to be a special type of table that can be
   accessed and modified only by the database system itself (not a regualr user).

###### 1.4.2 A specific DDL: SQL
###### 1.4.3 DML

1. Categories of DML

   1. Procedual DML (how)
     
      * Relational algebra

   2. Declarative DML (what)
     
      * SQL

2. DML is used to query and update database

###### 1.4.4 A specific DML: SQL
###### 1.4.5 Database Access from Application Programs
##### 1.5 Database Design
##### 1.6 Database Engine

###### 1.6.1 Storage Manager
###### 1.6.2 Query Processor
###### 1.6.3 Transaction Management


##### 1.7 Database and Application Architecture

##### 1.8 Database users and Administrators

##### 1.9 History of Database Systems

##### 1.10 Summary
