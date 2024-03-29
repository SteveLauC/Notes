1. How is a SQL executed in SQLite

   ![diagram](https://github.com/SteveLauC/pic/blob/main/how_a_sql_is_executed_in_sqlite3.png)
   
   SQLite works by translating SQL statements into bytecode and then running 
   that bytecode in a virtual machine.

2. SQLite is very modular and can be devided into 7 major components:
   
   1. Frontend
      > Fontend is responsible for compiling SQL

      1. Tokenizer
         Is responsible for splitting the inputed SQL statements into separate 
         tokens, by doing a lexical analysis on the input.
      2. Parser
         Is responsible for analyzing the structure of the SQL statement by 
         analyzing the tokens produced by the Tokenizer, and generates a parse 
         tree from the tokens.
      3. Code Generator
         It traverses the parse tree and generates bytecode, that when executed
         by the backend, should be able to produce the result from SQL statement.

   2. Backend
      > Backend executes the bytecode generated by the frontend.

      1. Virtual Machine
         The virtual machine takes the bytecode generated by the frontend and 
         it executes. This is the ultimate manipulator of data in the database.
         It can see a database as a collection of tables and indexes that are 
         actually stored as a data structure called B-Tree. The VM is essentially
         a big switch statement on the type of bytecode instruction.

      2. B-tree
         Is responsible for actually organizing the data into an ordered tree 
         data structure. Each table and indexes get their own B-Tree's. The 
         use of this structure helps the VM to search, insert, delete and update
         the data into trees. It also helps the VM to create or delete new tree if needed.

      3. Pager
         The B-Tree module requests information from the disk in fixed-size pages. 
         The default page_size is 4096 bytes but can be any power of two between 
         512 and 65536 bytes. The Pager Module is responsible for reading, writing,
         and caching these pages. 

         The page cache also provides the rollback and atomic commit abstraction
         and takes care of locking of the database file, implementing transactional
         ACID properties. The B-Tree driver requests particular pages from the 
         Pager and notifies the Pager when it wants to modify pages or commit or 
         rollback changes. The Pager handles all the messy details of making sure 
         the requests are handled quickly, safely, and efficiently.

      4. OS interface
         This module is responsible for providing a uniform interface to different 
         operating systems. It implements routines for file I/O, thread mutex, 
         sleep, time, random number generation, etc.

         > Isn't this covered by the standard library?

3. Data structures used by SQLite

   There are two kinds of B-tree that are employed by SQLite:
   1. B-Tree to store index, or "Index B-Tree"
   2. B+Tree to store table, or "Table B-Tree"

4. Main diff between `Table B-Tree` and `Index B-Tree`
   
   `Table B-Tree` uses a uniqure, non-null, signed 64-bit key to reference the 
   data in the internal node and the actual data is stored in the leaf nodes. 
   This 64-bit integer field is referred to as 
   [`ROWID`](https://www.sqlite.org/rowidtable.html).

   Let's take the following table as an example:

   ```
   +-------+-------+-------+-----+
   | ROWID | Name  | Marks | Age |
   +-------+-------+-------+-----+
   |    6  | Jone  | 5     | 28  |
   |   15  | Alex  | 32    | 45  |
   |   12  | Tom   | 37    | 23  |
   |   53  | Ron   | 87    | 13  |
   |   24  | Mark  | 20    | 48  | 
   |   25  | Bob   | 89    | 32  |
   +-------+-------+-------+-----+
   ```

   The following Table B-Tree will be used to store this table:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/table_b_tree.png)

   You can find that all data is stored in the leaf nodes, and each leaf node
   has a pointer pointing to the next leaf node, which enables it to perform
   binary search as the data stored is ordered.
