
# Fields

* alias (Alias *): relation alias, if any. Otherwise, NULL.

  ```sql
  postgres=# \d foo
            Table "public.foo"
   Column |  Type   | Collation | Nullable | Default 
  --------+---------+-----------+----------+---------
   a      | integer |           |          | 
   b      | integer |           |          | 

  select a from foo               (alias: NULL)
  select a from foo as table_foo  (alis {aliasnmae: table_foo, colnames: NIL })

  -- Note that you cannot select a as column a does not exist, it is called col1 now
  select col1 from foo as table_foo (col1) (alis {aliasnmae: table_foo, colnames: [col1] })
  select col1 from foo as table_foo (col1, col2) (alis {aliasnmae: table_foo, colnames: [col1, col2] })
  ```

  
* eref (Alias *): This is the reference name to the table, so different from
  `alias`, this field is always shown.

  ```sql
  select a from foo               (eref: {aliasname: foo, columns: [a, b] })
  select a from foo as table_foo  (eref: {aliasnmae: table_foo, colnames: [a, b] })

  -- Note that you cannot select a as column a does not exist, it is called col1 now
  select col1 from foo as table_foo (col1) (eref {aliasnmae: table_foo, colnames: [col1, b] })
  select col1 from foo as table_foo (col1, col2) (eref {aliasnmae: table_foo, colnames: [col1, col2] })
  ```

  `alias` and `eref` are both indexed by attribute number (attnum from pg_attribute), a 
  dropped column will still be stored and its `attisdropped` will be set to true. Since
  they won't be physically removed (still taking the attnum) and `alias` and `eref` are
  indexed by attnum, Postgres appends name (an empty string) for dropped columns as well