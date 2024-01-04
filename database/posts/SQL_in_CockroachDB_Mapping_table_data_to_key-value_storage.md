> This post focus on **storage**
>
> How CorkroachDB stores SQL data in its kv store

> link: https://www.cockroachlabs.com/blog/sql-in-cockroachdb-mapping-table-data-to-key-value-storage/

> This post is kinda outdated, for a detailed and latest doc, see 
> [Structured data encoding in CockroachDB SQL][link]
>
> [link]: https://github.com/cockroachdb/cockroach/blob/master/docs/tech-notes/encoding.md


1. Create a new table

   ```sql
   CREATE TABLE students (
    id INT PRIMARY KEY,
    name STRING,
    age INT,
   )
   ```

   > We don't cover how the table's metadata is stored as SQL databases generally
   > store metadata in a SQL table.

   and insert a row:

   ```sql
   INSERT INTO students VALUES (0, "steve", 18);
   ```

   Would create such entries in the kv store:

   ```rs
   {
     /students/primary/0/name: "steve",
     /students/primary/0/age: 18,
   }


2. Encoding

   You can see that a key in the kv store consists of the following conponents:

   ```
   /{table name}/{index name}/{values of search keys}/{field name}
   ```

   And value would be the value of `{field name}`.

3. query - selection
 
   The following query would

   ```sql
   SELECT * FROM students
   ```

   would list all the values in the kv store with prefix `/students/primary`, then
   the values with the same primary key belong to the same row, we would concatenate
   them together.

   > You may find that to support such a query, the underlying kv store has to be
   > able to find entries with prefix.
   >
   > For efficient access if the kv store resides on disk, it is great that the 
   > values with the same prefix are stored continuously. (or sorted)

4. query - selection with equality

   By default, every table created in cockroachDB will be rquested to have a 
   primary key, and the database will create a index for the primary key, as you
   can see from the key prefix `/students/primary/...`.

   A point query on the primary key can be efficiently handled:

   ```sql
   SELECT * FROM students WHERE id = 0
   ```
    
   Just list the kvs that are prefixed with `/students/primary/0`.

5. Secondary index

   If we create an index for column `name`, then we will have the following
   kv entries:

   ```sql
   CREATE INDEX idx_on_name ON name;
   ```

   ```
   {
     /students/idx_on_name/"steve"/0
   }
   ```

   The format is something like:

   ```
   /{table name}/{index name}/{values of search key}/{values of primary keys}
   ```

   It is slightly different from how we store rows, and notably, there is a
   `{values of primary keys}` extra field, the fields of this index is built
   on can have duplicate values, to avoid this, it is common to append the 
   primary key.
