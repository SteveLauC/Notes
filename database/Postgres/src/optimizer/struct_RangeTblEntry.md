# Notes




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

  > dropped columns
  > 
  > For RTE_RELATION RTEs, `alias` and `eref` are both indexed by attribute number 
  > (attnum from pg_attribute), a dropped column will still be stored in pg_attrihute 
  > and its `attisdropped` will be set to true. Since they won't be physically 
  > removed (still taking the attnum) and `alias` and `eref` are indexed by attnum, 
  > dropped columns will be shown in `alias` and `eref` as well if needed, Postgres 
  > stores an empty string for them.
  >
  > ```sql
  > create table test (a int, b int, c int);
  > alter table test drop column b;
  >
  > select * from test;
  > :alias <> 
  > :eref {ALIAS :aliasname test :colnames ("a" "" "c") }
  > 
  > select * from test as alias_test (c1);
  > :alias {ALIAS :aliasname alias_test :colnames ("c1") }
  > :eref {ALIAS :aliasname alias_test :colnames ("c1" "" "c") }
  >
  > select * from test as alias_test (c1, c2)
  > :alias {ALIAS :aliasname alias_test :colnames ("c1" "" "c2") }
  > :eref {ALIAS :aliasname alias_test :colnames ("c1" "" "c2") }
  > ```
  >
  > > Note however that a stored rule may have nonempty colnames for columns 
  > > dropped since the rule was created (and for that matter the colnames might 
  > > be out of date due to column renamings). 
  >
  > This is because when a rule gets created, it cache the Query of the rule, which
  > would contain a RangeTblEntry snapshot to the table used in it. This snapshot
  > won't be updated so it could be outdated, thus can have non-empty string for
  > dropped columns. You should `set debug_print_rewritten = true` to see tha snapshot
  > query tree.
  >
  > ```sql
  > set debug_print_rewritten = true;
  > create table users (id int, name text, email text);
  > CREATE RULE log_user_update AS ON UPDATE TO users DO ALSO INSERT INTO user_log (user_id, old_name) VALUES (OLD.id, OLD.name);
  > ALTER TABLE users DROP COLUMN name;
  > ```
  >
  > >The same comments apply to FUNCTION RTEs when a function's return type is 
  > > a named composite type.
  >
  > For a join (RTE_JOIN) range table entry, the dropped columns will be omitted 
  > in its `colnames`, tough stored rule will still behave in the way described in
  > the above comment.

* rtekind (RTEKind):

  Note that everything in pg_class is of kind `RTE_RELATION`, field `RangeTblEntry.relkind`
  (same as pg_class.relkind) can be used to distinguish the sub-classes:

  * r = ordinary table 
  * i = index 
  * S = sequence 
  * t = TOAST table
  * v = view
  * m = materialized view
  * c = composite type
  * f = foreign table
  * p = partitioned table
  * I = partitioned index

  But I think RangeTblEntry should be a data source, some relkinds cannot be data 
  source, e.g., index and composite type, so I expect to see partial relkinds in
  this field.
  

  ```c
  typedef enum RTEKind
  {
        RTE_RELATION,				/* ordinary relation reference */
        RTE_SUBQUERY,				/* subquery in FROM */
        RTE_JOIN,					/* join */
        RTE_FUNCTION,				/* function in FROM */
        RTE_TABLEFUNC,				/* TableFunc(.., column list) */
        RTE_VALUES,					/* VALUES (<exprlist>), (<exprlist>), ... */
        RTE_CTE,					/* common table expr (WITH list element) */
        RTE_NAMEDTUPLESTORE,		/* tuplestore, e.g. for AFTER triggers */
        /*
         * It serves 2 purposes:
         * 
         * 1. It represents an empty FROM clause, e.g., `SELECT 1`, added to 
         *    rtable by `replace_empty_jointree()`
         * 2. 
         */
        RTE_RESULT,
        RTE_GROUP,					/* the grouping step */
  } RTEKind;
  ```

-------------------------------------------------------------------------------

> Fields for RTE_RELATION

* relkind (char): See the notes in `rteKind`.


-------------------------------------------------------------------------------

> Fields for RTE_SUBQUERY


-------------------------------------------------------------------------------

> Fields for RTE_JOIN

* joinaliasvars (List<Var>)