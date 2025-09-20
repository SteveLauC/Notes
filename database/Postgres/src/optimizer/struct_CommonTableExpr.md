* ctename (string)

* aliascolnames

* ctematerialized (CTEMaterialize): Part of SQL syntax, user can use this to 
  control how Postgres will plan this CTE:

  ```c
  typedef enum CTEMaterialize
  {
      CTEMaterializeDefault,		/* no option specified */
      CTEMaterializeAlways,		/* MATERIALIZED */
      CTEMaterializeNever,		/* NOT MATERIALIZED */
  } CTEMaterialize;
  ```

  ```sql
  WITH w AS ( ... )
  WITH w MATERIALIZED AS ( ... )
  WITH w NOT MATERIALIZED AS ( ... )
  ```

  See also `SS_process_ctes()`.

* ctequery

* cterefcount (int): how many times has this CTE been reference in the query