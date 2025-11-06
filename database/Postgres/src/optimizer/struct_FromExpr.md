# fields

* fromlist

  List<Node>, `Node` could be of type:
  
  * `RangeTblRef`
  * `JoinExpr`
  
  ```sql
  -- Cases where it only contains RangeTblRef
    
  select * from table1;         -- [RangeTblRef(rtindex 1)]
  select * from table1, table2; -- [RangeTblRef(rtindex 1), RangeTblRef(rtindex 2)]
    
  -- [RangeTblRef(rtindex 2, kind RTE_SUBQUERY)]
  SELECT * FROM (SELECT id FROM table1) AS subquery;
  -- [RangeTblRef(rtindex1, kind RTE_RELATION), RangeTblRef(rtindex 2, kind RTE_SUBQUERY)]
  SELECT * FROM table2, (SELECT id FROM table1);
    
  -- Cases where it contains ONLY 1 JoinExpr
    
  -- [ JoinExpr { larg: RangeTblRef, rarg: RangeTblRef } ]
  select * from table1 join table2 on ...;
  -- [ JoinExpr { larg: JoinExpr { larg: RangeTblRef, rarg: RangeTblRef }, rarg: RangeTblRef } ]
  select * from table1 join table 2 on ... join table3 on ...; 
    
  -- Cases where it is a mix of RangeTblRef and JoinExpr
  
  -- [ JoinExpr { larg: RangeTblRef, rarg: RangeTblRef }, RangeTblRef ]
  SELECT * FROM table1 JOIN table2 ON ... , table3;
  ```

* quals