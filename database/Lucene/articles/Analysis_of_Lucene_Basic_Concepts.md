> [Article URL](https://alibaba-cloud.medium.com/analysis-of-lucene-basic-concepts-5ff5d8b90a53)

1. The internal representation of a lucene index

   ![diagram](https://github.com/SteveLauC/pic/blob/main/lucene_index.png)

2. Analogy between some key concepts in lucene and RDBMS:

   | Lucene    | RDBMS    |
   |-----------|----------|
   | Index     | A table (RDBMS has some constraints (e.g., Primary key constraints) on its table, Index in lucene has no such stuff) |
   | Docuement | A row in a table |
   | DocId (Sequence Number) | Primary Key |

3. What is `Field` in lucene

   Each `Document` in lucene consists of one or more `Field`s, each field has 
   three parts: 

   1. name 
   2. type 
   3. value

   For `type`, lucene has several `FieldType`s:

   > `FieldType` of lucene, copied from its doc:
   >
   > * TextField: Reader or String indexed for full-text search
   > * StringField: String indexed verbatim as a single token
   > * IntPoint: int indexed for exact/range queries.
   > * LongPoint: long indexed for exact/range queries.
   > * FloatPoint: float indexed for exact/range queries.
   > * DoublePoint: double indexed for exact/range queries.
   > * SortedDocValuesField: byte[] indexed column-wise for sorting/faceting
   > * SortedSetDocValuesField: SortedSet<byte[]> indexed column-wise for sorting/faceting
   > * NumericDocValuesField: long indexed column-wise for sorting/faceting
   > * SortedNumericDocValuesField: SortedSet<long> indexed column-wise for sorting/faceting
   > * StoredField: Stored-only value for retrieving in summary results

   The method used to index such data is depending on the type of that `Field`.

4. What are `Term` and `TermDictionary`

   The smallest unit of index and search in Lucene. **A field consists of one or 
   more terms.** A term is produced when a field is put through an Analyzer 
   (tokenizer). 

   A term dictionary is the basic **index** used to perform conditional searches 
   on terms.

5. What is `Segment` 

   sub-index.

6. Why the search/query in lucene in **NOT** real time but near real-time (NRT)

   The key point here is that `in-memory buffer` in lucene is not searchable as
   search in lucene relies on the setup of the index and this setup is completed
   during the process of segment flush (to disk).

   > The purpose of this is to set up the **most efficient** index. 
   >
   > Why is the index created during flush most effective?

7. What is `DocID` (Sequence Number)

   A number uniquely identifies a `Document` in an `Index`.

   Some things you should know about `DocID`:

   1. The `DocID` in Lucene is similar to the concept of `primary key` in a RDBMS.
      `primary key` is unique in the table, but `DocID` is actually **unique in a 
      `Segment` rather than in an `Index`.**

      Even though `DocID` is unique only in `Segment`, it can still uniquely identify
      a `Document` as `Segment`s are ordered. For example, say we have two `Segment`s
      with 100 `Document`s in each one, at the `Segment` level, the `DocID`s range 
      from 1 to 100, at the `Index` level, the `DocID`s of second segment will be 
      converted to `[101, 200]`.

   2. `DocID`s are not necessarily continuous as you can delete a `Document` in
      a `Segment` or an `Index` which will obviously create a "hole".

8. Inverted Index in Lucene

   The inverted Index is essentially a list mapping each `Term` to the `DocID`
   of the `Document` containing that `Term`.
    
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-01-02%2010-06-46.png)

   So when Lucene is searching internally, it makes a two-phase query:

   1. List the `DocId`s found to contain the given `Term`
   2. Find the `Document` based on the `DocId`s
