```sql
SELECT a FROM foo WHERE NULL = ANY (SELECT b FROM bar) 
```

the jointree part `NULL = ANY (SELECT b FROM bar)` can be optimized to `NULL`

call chain
1. subquery_planner()
2. preprocess_qual_conditions()
3. preprocess_expressions()
4. eval_const_expressions() traverses the SubLink's expressions (`testexpr`)
5. simplify_function() 
