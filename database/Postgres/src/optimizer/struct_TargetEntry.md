
# Fields

* `xpr` (Expr): This field means `TargetEntry` is an expresssion

  > Strictly speaking, a TargetEntry isn't an expression node (since it can't
  > be evaluated by ExecEvalExpr).  But we treat it as one anyway, since in
  > very many places it's convenient to process a whole query targetlist as a
  > single expression tree.

* `expr` (Expr *): The expression it contains

* `resno` (AttNumber, aka, int16):
  
  * For `SELECT`, this records the index of the column, starting from 1
  * For `INSERT` and `UPDATE`: the attribute number of the destination column.
    So `resno` values in `Query.targetList` could be out-of-order or even duplicate:
    
    ```sql
    create table students (id int, name text, major text);
    insert into students (major, id, name) values ('math', 9, 'buzz');
    
    -- resno values: 3, 1, 2
    
    :targetList (
        {TARGETENTRY 
        :expr 
            {CONST 
            :consttype 25 
            :consttypmod -1 
            :constcollid 100 
            :constlen -1 
            :constbyval false 
            :constisnull false 
            :location 47 
            :constvalue 8 [ 32 0 0 0 109 97 116 104 ]
            }
        :resno 3 
        :resname major 
        :ressortgroupref 0 
        :resorigtbl 0 
        :resorigcol 0 
        :resjunk false
        }
        {TARGETENTRY 
        :expr 
            {CONST 
            :consttype 23 
            :consttypmod -1 
            :constcollid 0 
            :constlen 4 
            :constbyval true 
            :constisnull false 
            :location 55 
            :constvalue 4 [ 9 0 0 0 0 0 0 0 ]
            }
        :resno 1 
        :resname id 
        :ressortgroupref 0 
        :resorigtbl 0 
        :resorigcol 0 
        :resjunk false
        }
        {TARGETENTRY 
        :expr 
            {CONST 
            :consttype 25 
            :consttypmod -1 
            :constcollid 100 
            :constlen -1 
            :constbyval false 
            :constisnull false 
            :location 58 
            :constvalue 8 [ 32 0 0 0 98 117 122 122 ]
            }
        :resno 2 
        :resname name 
        :ressortgroupref 0 
        :resorigtbl 0 
        :resorigcol 0 
        :resjunk false
        }
    )   
    ```
    
    ```sql
    create table with_array (arr text[]);
    update with_array set arr[1]='a', arr[2]='b';
    
    -- resno values:  1 1
    
    :targetList (
        {TARGETENTRY 
        :expr 
            {SUBSCRIPTINGREF 
            :refcontainertype 1009 
            :refelemtype 25 
            :refrestype 1009 
            :reftypmod -1 
            :refcollid 100 
            :refupperindexpr (
            {CONST 
            :consttype 23 
            :consttypmod -1 
            :constcollid 0 
            :constlen 4 
            :constbyval true 
            :constisnull false 
            :location 26 
            :constvalue 4 [ 1 0 0 0 0 0 0 0 ]
            }
            )
            :reflowerindexpr <> 
            :refexpr 
            {VAR 
            :varno 1 
            :varattno 1 
            :vartype 1009 
            :vartypmod -1 
            :varcollid 100 
            :varnullingrels (b)
            :varlevelsup 0 
            :varreturningtype 0 
            :varnosyn 1 
            :varattnosyn 1 
            :location 22
            }
            :refassgnexpr 
            {CONST 
            :consttype 25 
            :consttypmod -1 
            :constcollid 100 
            :constlen -1 
            :constbyval false 
            :constisnull false 
            :location 29 
            :constvalue 5 [ 20 0 0 0 97 ]
            }
            }
        :resno 1 
        :resname arr 
        :ressortgroupref 0 
        :resorigtbl 0 
        :resorigcol 0 
        :resjunk false
        }
        {TARGETENTRY 
        :expr 
            {SUBSCRIPTINGREF 
            :refcontainertype 1009 
            :refelemtype 25 
            :refrestype 1009 
            :reftypmod -1 
            :refcollid 100 
            :refupperindexpr (
            {CONST 
            :consttype 23 
            :consttypmod -1 
            :constcollid 0 
            :constlen 4 
            :constbyval true 
            :constisnull false 
            :location 38 
            :constvalue 4 [ 2 0 0 0 0 0 0 0 ]
            }
            )
            :reflowerindexpr <> 
            :refexpr 
            {VAR 
            :varno 1 
            :varattno 1 
            :vartype 1009 
            :vartypmod -1 
            :varcollid 100 
            :varnullingrels (b)
            :varlevelsup 0 
            :varreturningtype 0 
            :varnosyn 1 
            :varattnosyn 1 
            :location 34
            }
            :refassgnexpr 
            {CONST 
            :consttype 25 
            :consttypmod -1 
            :constcollid 100 
            :constlen -1 
            :constbyval false 
            :constisnull false 
            :location 41 
            :constvalue 5 [ 20 0 0 0 98 ]
            }
            }
        :resno 1 
        :resname arr 
        :ressortgroupref 0 
        :resorigtbl 0 
        :resorigcol 0 
        :resjunk false
        }
    )
    ```
  
* `resname` (char *): name of the column, could be wrong or NULL

* `ressortgroupref` (Index, aka, uint): 0 means this column is not modified by a
  `ORDER BY, GROUP BY, PARTITION BY, DISTINCT, DISTINCT ON`. Otherwise, it is the
  index to the `SortGroupClause` entry stored in `Query.sortClause`.
  
* `resorigtbl` (Oid): OID of column's source table

* `resorigcol`(AttNumber, aka, int16): column number in the source table
  
* `resjunk` (bool): true means this is a junk column, should be removed from the
  final target list.