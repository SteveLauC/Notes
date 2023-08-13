> This chapter is mainly about:
>
> * THe SQLite database system and what it is
> * What salient features SQLite supports
> * How SQLite stores tables in database files
> * How to write, compile, and execute SQLite applications
> * Some most frequently used SQLite APIs
> * The modular SQLite architecture
> * SQLite limitations


> * 2.1 Introduction to SQLite
>   * 2.1.1 Salient SQLite characteristics
>   * 2.1.2 Usage simplicity
>   * 2.1.3 SQL features and SQLite commands
>   * 2.1.4 Database Storage
>   * 2.1.5 Limited Concurrency
>   * 2.1.6 SQLite Usage
> * 2.2 Sample SQLite Applications
>   * 2.2.1 A Simple Application
>   * 2.2.2 SQLite APIs
>   * 2.2.3 Direct SQL execution

# Before the first section

1. A single SQLite database file consists of multiple B+Trees, i.e., multiple
   tables belong to this database.

# 2.1 Introduction to SQLite

## 2.1.1 Salient SQLite characteristics
## 2.1.2 Usage simplicity
## 2.1.3 SQL features and SQLite commands

1. SQL features that are supported in SQLite

   > https://sqlite.org/omitted.html

   1. Complete `ALTER TABLE` support
   2. Complete trigger support
   3. Writing to VIEWs
   4. `GRANT` and `REVOKE`

## 2.1.4 Database Storage

1. SQLite stores all the data that a database uses in a single file

## 2.1.5 Limited Concurrency

1. The concurrency level supported by SQLite is database level, and it supports

   1. multiple readers to a database file
   2. One exclusive writer to a database file

## 2.1.6 SQLite Usage
# 2.2 Sample SQLite Applications
## 2.2.1 A Simple Application

1. demo code in Rust

   ```rust
   fn main() {
       let db = sqlite::open("MyDB").unwrap();

       let statement =
           db.prepare("select SID from students order by SID").unwrap();

       for row in statement {
           let row = row.unwrap();
           let sid = &row[0];
           println!("SID = {:?}", sid);
       }
   }
   ``` 
## 2.2.2 SQLite APIs

> Reference: [C-language Interface Specification for SQLite](https://www.sqlite.org/capi3ref.html)

1. `sqlite3_open`

   The db file opening or creation is done in a **lazy mode**, it will be ONLY 
   opened or created when you access it for the first time.

   > It is recommended that new programs should use the `sqlite3_open_v2` 
   > interface.

2. `sqlite3_prepare`

   Compile a SQL statement into the SQLite bytecodes.

   It returns a `Statement` that can be seen as an iterator of the result rows.

3. `sqlite3_step`

   Call `next()` on the `Statement` iterator by executing the bytecode program.

   For SQL statements that does not return rows, the iterator yields nothing.

4. `sqlite3_column_*`

   Extract a specific column from a row `SQLITE_ROW`. 

5. `sqlite3_finalize`

   Closes and destroys a statement handle, and user will get a sign indicating
   the excuting result.

6. `sqlite3_close`

   Closes a library connection, if there are prepared statement that has not been
   finalized, this function returns `SQLITE_BUSY`.

7. Other userful functions

   1. `sqlite3_bind_*`

      This function is for binding variables.

   2. `sqlite3_reset`

      Reset the statement to its initial state, except that the bind variables 
      remain same.

      To clear bind variables, use `sqlite3_clear_bingdings`a.

## 2.2.3 Direct SQL execution

1. Demo code in Rust

   ```rust
   use std::{env::args, process::exit};

   fn main() {
       let av = args().collect::<Vec<_>>();
       if av.len() != 3 {
           eprintln!("Usage: {} DATABASE-NAME SQL-STATEMENT", av[0]);
           exit(-1);
       }

       let db = sqlite::open(av[1].as_str()).unwrap();

       db.iterate(av[2].as_str(), |pairs| {
           for (name, value) in pairs.iter() {
               print!("{} = ", name);
               if let Some(value) = value {
                   println!("{}", value);
               } else {
                   println!("NULL");
               }
           }
           true
       })
       .unwrap();
   }
   ```
