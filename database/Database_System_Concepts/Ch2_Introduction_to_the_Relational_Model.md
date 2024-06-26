#### 2.1 structure of relational databases
1. relational model为什么能成功?
    1. 它简洁，简化了程序员的负担
    2. 和底下的存储结构无关，如果有新的存储方式出现，仍然可以在其上实现relational
    model

2. relational model的结构
   
   a relational database consists of a collection of tables.

#### 2.2 database schema
1. terminologies used in the relational theory and its structure, and their correspondence

   * relation -> table
   * tuple -> row
   * attribute -> column

2. domain
   
   for each attribute of a relation, there is a set of permitted values , 
   called the domain

   And we require that the domains of a relation must be **atomic**. A domain is
   atomic if elements of the domain are considered to be indivisible units

#### 2.3 keys
1. key
  
   we use key to distinguish a tuple in a relation

   * superkey: a distinct set of one or more attributes that, taken **collectively**
   , allow us to identify a tuple in a relation
   > If K is a superkey, then any super set of K is also a superkey
   * candidate keys: we are often interested in superkeys for whch no proper 
   subset is sukerkey(minimal superkey)
   > It is possible that several distinct sets of attributes can serve as a candidate
   key(可以有好几个候选码)
   * primary key: a candidate key that is chosen by the database designer as a
   principal means of identifying tuples with a relation
   > In a relation schema, primary keys are listed before the other attributes
   and underlined

2. primary key constraint
   
   The designation of a key represents a constraint in the real world enterprise
   being modeled. Thus, primary keys are also referred as primary key constraint

   The primary key must contain unique value and can not be null

3. foregin key constraint

   A foreign key constraint from attribute A of relation r1 to the **primary key** B
   of r2 stats that: on any database instances, the value of A for each tuple in r1
   must also be the value of B for some tuple in r2

   Attribute A is called a foreign key from r1. r1 is called the referencing relation
   of the foreign key constraint and r2 is called the referenced relation

   > Note: in a foreign key constraint, the referenced attribute of the referenced 
   relation must be a primary key. The more general case, a referential-integrity
   constraint, relaxes the requirement that the referenced attributes from the
   primary key of the referenced relation. Foreign key constraint is a special
   case of referential intergrity constraint
   
   > Database systems today typically support foreign key constraint key instead
   of referential integrity constraint

#### 2.5 Relational Query Language
1. categories of query language
  
   * imperative ~:中文译作命令式查询语言
   * functional ~
   * declrative query language

#### 2.6 The Relational Algebra

1. What is relational algebra

   The relational algebra consists of a set of operations that take **1 or 2**
   relations as input and produce a new relation as their result.

2. Unary and binary operations

   Operations like 

   * select
   * project
   * assignment
   * rename

   operate on one relation so that they are called unary operation.

   Set operations like

   * Union
   * Intersection
   * set difference
   * Cartesian product (cross product)

   operate on pairs of relations and are **binary** operations.

3. By definition, a relation is a set of tuples so that a the tuples cannot
   be duplicate. But in practise, tables in database systems are permitted to
   contain duplicate tuples.

4. Operations

   1. select $ \sigma $ (unary)

      select tuples that satifies the given predicate

      > In predicate, we can combine sub-predicate using connections like and, 
      > or and not. 
      >
      > * and: $ \wedge $
      > * or: $ \vee $
      > * not: $ \neg $

   2. project $ \Pi $ (unary)
   
      The basic version of project operation allows only attribute names to be 
      present in the predicate(referenced columns), a generalized version allows 
      experssion involving attributes to apperar in it.

      > ```sql
      > SELECT 1;
      > ```

   3. Cartesian product $ \times $ (binary)

      This operation can be used to combine information from any 2 relations, e.g.,
      doing the cartesian product over table a and b would be something like

      $$ a \times b $$

   4. (Inner) (theta) Join $ \Join_{\theta} $ (binary)

      The join operation allows us to combine a selection and a Cartesian product
      into a single operation.

      Consider relations `r` and `s`, then let $ \theta $ be a predicte on attributes
      in schema $ R \cup S $, then the join operation is defined like this:

      $$ r \Join_{\theta} s = \sigma_{\theta} (r \times s) $$

   5. Set operations (binary)

      > Technically, Cartesian product is also a set operation (they all come from
      > set theory)

      > All set operations are **binary** operations.

      1. Union $ \cup $
      2. Intersection $ \cap $ 
      3. Set-difference $ \setminus $ 

      > All the set operations, must be done to **compatible** relations:
      >
      > 1. The 2 input relations should have the same number of attributes
      > 2. For each i, the type of the ith attributes of both input relations must
      >    be same
      >
      > This schema-compatible restriction is not requested in the set theory, 
      > it is added in relational algebra.
   
   6. Assignment $ \leftarrow $ (binary)

      Assign the result of a relational-algebra expression to a **temporary** relation.

      This is basically equivalent to the assignment operation in a programming
      language, for example, assigning all the IDs of all male teacher to 
      `id-of-male-teacher`:

      $$ id-of-male-teacher \leftarrow \Pi_{id} (\sigma_{gender=male} (teacher)) $$

   7. Rename $ \rho $ (unary)

      > In SQL, we have the `as` keyword.

      The rename operation allows us to rename a relation or fields of it, or 
      both of them.

      For example, to rename relation teacher to foo:

      $$ \rho_{foo} (teacher) $$

      To rename the name field of the teacher relation to bar:

      $$ \rho_{bar/name} (teacher) $$

      Assume the relation teacher has 3 attributes, to rename all the attributes 
      something else, we do:

      $$ \rho_{(name1, name2, name3)} (teacher) $$

   8. Other common relational algebra expressions

      1. Agregation
      2. Natural join $ \Join $
      3. Outer join