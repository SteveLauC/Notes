# Notes




# Fields

> Fields valid in all kinds of RTEs
>
> They are put here rather than the end of the struct to make dumping legible

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
  > This is because when a rule gets created, it caches the Query of the rule, which
  > will contain a `RangeTblEntry` snapshot to the table used in it. This snapshot
  > won't be updated so it could be outdated, thus can have non-empty string for
  > dropped columns. You should `set debug_print_rewritten = true` to see the snapshot
  > query tree.
  >
  > ```sql
  > create table users (id int, name text, email text);
  > create table user_log (user_id int, old_name text);
  > CREATE RULE log_user_update AS ON UPDATE TO users DO ALSO INSERT INTO user_log (user_id, old_name) VALUES (OLD.id, OLD.name);
  > ALTER TABLE users DROP COLUMN email;
  > insert into users values (1, '');
  >
  > set debug_print_rewritten = true;
  >
  > -- trigger that rule, search for field 'email'
  > update users set name = 'new_name' where id = 1;
  > ```
  >
  > >The same comments apply to FUNCTION RTEs when a function's return type is 
  > > a named composite type.
  >
  > For a join (RTE_JOIN) range table entry, the dropped columns will be omitted 
  > in its `colnames`, tough stored rule will still behave in the way described in
  > the above comment.

* rtekind (RTEKind):

  Note that everything in `pg_class` is of kind `RTE_RELATION`, field `RangeTblEntry.relkind`
  (same as `pg_class.relkind`) can be used to distinguish the sub-classes:

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

* relid (Oid): OID of the relation

* inh (bool)

  * For RTE_RELATION: it is true for relations that should be expanded to include 
    inherience children
  * For RTE_SUBQUERY: The planner also set this to true if it contains `UNION ALL` 
    queries that it has flattened into pulled-up subqueries (creating a structure 
    much like the effects of inheritance)

* relkind (char): See the notes in `RangeTblEntry.rteKind`.

* rellockmode (int/LOCKMODE): lock level that query requires on the rel

* perminfoindex (Index, i.e., uint): Index to the RTEPermissionInfo entry

* tablesample (struct TableSampleClause)


-------------------------------------------------------------------------------

> Fields for RTE_SUBQUERY


-------------------------------------------------------------------------------

> Fields for RTE_JOIN

* jointype (JoinType): type of this join

* joinmergedcols (int): Number of "merged" columns. Merged columns are placed at
  the beginning of `joinaliasvars` (of type `struct CoalesceExpr`),  so this field
  also tells you how many columns at the beginning of the join's output correspond to these merged columns.
  
  What does "merged columns" mean? Natural join's performs equi-join on tables with
  **matching** columns (same name, compatible data type), since these columns have
  the same name and value (as it is an equi-join), Postgres does not display both
  these columns in the result, it "merge"s 2 columns into 1 column:

  ```sql
  postgres=# \d foo
                    Table "public.foo"
    Column    |  Type   | Collation | Nullable | Default 
  -------------+---------+-----------+----------+---------
  id          | integer |           |          | 
  description | text    |           |          | 

  postgres=# \d bar
                    Table "public.bar"
    Column    |  Type   | Collation | Nullable | Default 
  -------------+---------+-----------+----------+---------
  id          | integer |           |          | 
  description | text    |           |          | 

  -- This does an inner equi join on all matching columns (id, description)
  -- joinmergedcols = 2
  postgres=# select * from foo natural join bar;
  -- This does an left outer equi join on all matching columns (id, description)
  -- joinmergedcols = 2
  postgres=# select * from foo natural left join bar;
  -- This does an right outer equi join on all matching columns (id, description)
  -- joinmergedcols = 2
  postgres=# select * from foo natural right join bar;

  -- This does an inner equi join on column id, see that we have only 1 id column shown
  -- joinmergedcols = 1
  postgres=# select * from foo join bar using (id);
  id | description | description 
  ----+-------------+-------------
    1 | one         | 一
  (1 row)

  -- This does an inner equi join on column id and description
  -- joinmergedcols = 2
  postgres=# select * from foo join bar using (id, description)
  ```

* joinaliasvars (List<Node>): A list of nodes representing the output columns
  of this join, where node can be of type:
  
  * Var: when it is a simple column
  * Var with implicit type cast
  * CoalesceExpr(Var): when it is a merged column
  
  The first `joinmergedcols` entries are `CoalesceExpr`s, the remaining are all
  `Var`s.
  
  ```text
  | Merged columns | remaining columns form left relation | remaining columns from right relation |
  ```
  
* joinleftcols (List<int>): Physical column numbers of the columns that come 
  from the left relation and are included in the output.

* joinrightcols (List<int>): Same as `joinleftcols` except that this list contains
  the columns from the right input relation.

* join_using_alias (Alias): A niche feature introduced in SQL standard 2016 that
  allows you to alias all the columns invoked in "using" as a whole:
  
  ```sql
  postgres=# \d foo
                  Table "public.foo"
  Column    |  Type   | Collation | Nullable | Default 
  -------------+---------+-----------+----------+---------
   id          | integer |           |          | 
   description | text    |           |          | 
    
  postgres=# \d bar
                  Table "public.bar"
  Column    |  Type   | Collation | Nullable | Default 
  -------------+---------+-----------+----------+---------
   id          | integer |           |          | 
   description | text    |           |          | 
    
  postgres=# select using_alias.id from foo join bar using (id) as using_alias;
  id 
  ----
   1
  (1 row)
  ```
  
  In the above example `using_alias` is equivalent to tuple `(id)`

-------------------------------------------------------------------------------

> Function RTE

* functions (List<RangeTblFunction>)


-------------------------------------------------------------------------------

> Table function RTE


-------------------------------------------------------------------------------

> Values RTE


-------------------------------------------------------------------------------

> CTE RTE (RTE_CTE)
>
>
> ```sql
> with cte_foo as (select * from table)
> select * from cte_foo;
> ```
>
> `cte_foo` is a `RTE_CTE`

* ctename (string) 

* ctelevelsup (Index, uint): 

* self_reference (bool)


-------------------------------------------------------------------------------

> Fields valid in all RTEs:

* `lateral` (bool): This flag signals to the query planner that this FROM-clause
  entry cannot be evaluated **independently**.
  
  When it applies to `RTE_SUBQUERY`, it means that this subquery is correlated
  
  QUES: what does it mean when it appies to other `RTE_KIND`s

* inFromCl (bool): 
* securityQuals (List<?>):