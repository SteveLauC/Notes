If a relation is marked with one of the following attributes:

* `FOR UPDATE`
* `FOR NO KEY UPDATE`
* `FOR SHARE`
* `FOR KEY SHARE`

explicitly, Postgres adds a `RowMarkClause` in `Query.rowMarks` for it. It is
parser-time representation of the above clauses.

```sql
SELECT * FROM table 
FOR UPDATE;
```

If user explicitly adds an attribute for a subquery, then Postgres **recursively**
adds one `RowMarkClause` for all the `RTE_RELATION` and `RTE_SUBQUERY` entries. 
Except for the `RowMarkClause` added for the explicit attribute, other 
`RowMarkClause`s will have field `pushedDown` set to `true`.


```sql
CREATE TABLE a (id int primary key);
CREATE TABLE b (id int primary key, a_id int references a(id));
CREATE TABLE c (id int primary key, b_id int references b(id));


SELECT *
FROM ( -- outer subquery, sub1
    SELECT *
    FROM a
    JOIN ( -- inner subquery, sub2
        SELECT * FROM b JOIN c ON b.id = c.b_id
    ) AS sub2 ON a.id = sub2.a_id
) AS sub1
FOR UPDATE OF sub1;
```

Postgres adds 5 `RowMarkClause`s for the above query:

* sub1: pushedDown = false
* a: pushedDown = true
* sub2: pushedDown = true
* b: pushedDown = true
* c: pushedDown = true

