# Overview

When the planner does join ordering, it will draw a tree, every RelOptInfo
is a unit in that tree.

# Notes

1. RelOptInfo could represent different types of relations:

   * base relation 

     It can be a table/sub-select/function, it is identified by one RT index (range 
     table entry index)

     Stored in `PlannerInfo->simple_rel_array`

     TODO: Gemini said that base relation can be other types as well, verify this 
     and update notes accordingly.

     | `RTEKind` (来源) | 是否可以成为 `base relation`? | 备注 |
     | :--- | :--- | :--- |
     | `RTE_RELATION` | **是** | 最常见的情况，一个物理表。 |
     | `RTE_SUBQUERY` | **是** | `FROM (SELECT ...)` |
     | `RTE_FUNCTION` | **是** | `FROM my_func(...)` |
     | `RTE_VALUES` | **是** | `FROM (VALUES ...)` |
     | `RTE_CTE` | **是** | `WITH ... SELECT ... FROM cte` |
     | `RTE_TABLEFUNC` | **是** | `FROM XMLTABLE(...)` |
     | `RTE_NAMEDTUPLESTORE` | 否 | 用于触发器中的 `NEW`/`OLD` 表，不参与主连接树。|
     | `RTE_JOIN` | 否 | `JOIN` 本身是**连接的结果**，而不是连接的**基本单元**。它的 `RelOptInfo` 是 `RELOPT_JOINREL`。 |
     | `RTE_RESULT` | 否 | 通常是为没有 `FROM` 子句的查询创建的占位符，不参与连接。 |

     
   * join relation

     Join of two or more base relations.
     
     It is identified by a set (order does not matter) of RT indexes to the 
     base relations that participate in the join, plus the RT indexes to outer
     join range table entries if there are outer joins.

     Stored in `PlannerInfo->join_rel_list`

   * Other relation:
     
     Similar to base relations, each other relation is also identified by one RT index.
     But they do not appear in the jointree.

     Currently, "the only kind of other relation is member relation", which can be:

     1. children tables when you scan a parent table. Parent table is a base relation, all
        the children tables are member relations

        ```sql
        create table employee (name text);
        create table leader (department text) inherits employee;

        insert into leader values ('leader_foo', 'sales');

        select * from employee
            name    
        ------------
        leader_foo
        (1 row)

        explain select * from employee;
                                        QUERY PLAN                                 
        ---------------------------------------------------------------------------
        Append  (cost=0.00..23.21 rows=881 width=32)
        ->  Seq Scan on employee employee_1  (cost=0.00..0.00 rows=1 width=32)
        ->  Seq Scan on leader employee_2  (cost=0.00..18.80 rows=880 width=32)
        (3 rows)
        ```

     2. UNION ALL 
       
        > QUES: I don't get this

   * Upper relation: Post scan/join relations

# Fields

* reloptkind: every `RelOptInfo` can have the following types:

  ```c
  typedef enum RelOptKind
  {
  	RELOPT_BASEREL,
   	RELOPT_JOINREL,
    /* The only other relation that is still in use */
   	RELOPT_OTHER_MEMBER_REL,
    /* RelOptInfos that describe post-scan/join processing steps */
   	RELOPT_UPPER_REL,

    /* QUES: ??? */
   	RELOPT_OTHER_JOINREL,
    /* QUES: ??? */
   	RELOPT_OTHER_UPPER_REL,
  } RelOptKind;
  ```
  
* relids (BitmapSet): indexes to the range table entries included in this relation

  * For base relation: It contains 1 rt index (relation/subquery/function) 
  * For join relation: It contains indexes to the range table entris that are
    involved in the join, as well as the indexes to the outer join range table 
    entries formed at or undre this `RelOptInfo`.
  * For other relation: It contains 1 rt index
  * For upper relation: this field is empty

* rows (double): estimiated number of rows that this "operator" will output

  > "operator": think about it, `RelOptInfo` could be a "scan" of base relation
  > and join operation, so it could output tuples.

* consider_startup (bool): 
  
  A flag that controls if the plans with cheap startup_costs should be kept.
  
  Normally, planner looks for paths whose total cost is low, it is not 
  interested in paths with low startup_cost but high total_cost. This flag 
  tells the planner that a path with low startup_cost has value and should be 
  kept even though its total_cost is high.

* consider_param_startup (bool): same as consider_startup, but is for parameterized paths.
  while consider_startup is for unparameterized paths.

  > QUES: What are parameterized paths?

* consider_parallel (bool): should planner generate parallel paths for this `RelOptInfo`

* reltarget (PathTarget): default target list for this relation

* pathlist (List<Path>): "Serialized" Paths will be stored in this field

  serialized paths: will be executed completely by the backend process and backend 
  process only, no worker processes needed.

* ppilist (List<ParamPathInfo>): For the parameterized Paths in pathlist, their 
  ParamPathInfo nodes will be stored here

* partial_pathlist (List<Path>): paths that will be executed by parallel workers.
  A partial path does partial computation, it needs a Gather or GatherMerge node
  to generate the final result.

* cheapest_startup_path (Path *): pointer to the unparameterized path that has 
  the lowest startup cost in pathlist.
  
  NULL if there is no unparameterized path

* cheapest_total_path (Path *): the pathlist member with lowest total cost among the 
  unparameterized paths. 
  
  Or if there is no unparameterized path, the path with  lowest total cost 
  among the paths with minimum parameterization

* cheapest_parameterized_path (List<Path>): best paths for their parameterizations;
  always includes cheapest_total_path, even if that's unparameterized

-------------------------------------------------------------------------------

> parameterization information needed for both base rels and join rels (see 
> also lateral_vars and lateral_referencers)

* direct_lateral_relids: 

* lateral_relids: 


-------------------------------------------------------------------------------

> Information of a base relation, not set for join relations

* relid (uint): range table entry index. For a base relation, field `relids` will
  only have 1 index, so it is equivalent to `relid`. `relid` is provided for 
  convenience

* reltablespace (Oid): OID of the tablespace that this relation belongs to

* rtekind

* min_attr (int16): 
  
  Postgres base relation can be:
  
  * a real table
  * subquery
  * function
  
  but they are all relations that contain columns/attributes.
  
  This field stores the lowest attribute number. For a real table, it would be 
  negative since it contains system columns. Otherwise, it should be positive 
  (not 0)

* max_attr (int16): highest attribute number

* attr_needed (Array<Relids>)

  This is an array indexed by range `[min_attr, max_attr]`, each element is 
  a Relids bitset. I.e., one bitmapset for each attribute
  
  If bit 0 is set to 1, it means that this attribute is needed as a part of target 
  list. 
  
  The document says that other bits track "the highest joinrel in which each 
  attribute is needed;". QUES: I don't understand why it needs multiple bits
  to track "the highest join relation"
  
* attr_widths (Array<int32>)
 
  An array of average estimated column width (in bytes), 0 means it is not computed
  and cached.
  
  It is also indexed by range `[min_attr, max_attr]`

* notnullattnums (bitset): A zero-based set contains attributes numbers of all the
  `NOT NULL` columns
  
  zero-based set: if a non-null column has attnum 1, 1 - 1 = 0 will be stored in 
  the set

* nulling_relids (bitset of RT indexes): RT indexes to the outer join relations 
  that can set this base relation to NULL
  
  For instance, `select * from t1 left join t2`, join relation `t1 left join t2`
  can set base relation t2 to NULL, so its RT index will be in this set.

* lateral_vars

* lateral_references

* indexlist (List<IndexOptInfo>): If this base relation is a table or partitioned 
  table, this field stores IndexOptInfo nodes for this table's indexes. Otherwise,
  it is `NIL`
  
* statlist (List<StatisticExtInfo>): Extended statistic information  

* pages (uint32): If this base relation is a table, this field stores the number 
  of pages it takes. Othersise, 0.

* tuples (double): Number of tuples this relation has (not considering restriction 
  or filtering) 

  For a subquery, tuples is not set immediately upon creation of the RelOptInfo 
  object; It is filled in when `set_subquery_pathlist()` processes the object.

* allvisfrac: All-visible fraction.

  What percentage of pages are marked all-visible? This is used to estimate the 
  cost of index-only scan.
  
  What does all-visible mean? See "database/Postgres/pg17_docs/Ch11_Indexes/11.9_Index-Only_Scans_and_Covering_Indexes.md"

* eclass_indexes: 

* subroot (PlannerInfo *): Will be set only if this base relation is a subquery. 
  Pointer to the  `PlannerInfo` created for this subquery.
  
  For a subquery, subroot is not set immediately upon creation of the RelOptInfo 
  object; It is filled in when `set_subquery_pathlist()` processes the object.

* subplan_params (List<PlannerParamItem>):  Will be set only if this base relation 
  is a subquery. List of PlannerParamItem to be passed to the subquery

* rel_parallel_workers (int32): When users modify a table via:

  ```sql
  ALTER TABLE ... SET (parallel_workers = N)` 
  ```
  
  value N will be stored here. Otherwise, -1.

* amflags (int32, bitmask): Bitmask of optional features supported by the table 
  AM

--------------------------------------------------------------------------------

> Information about foreign tables and foreign joins
>
>
> If the relation is either a foreign table or a join of foreign tables that
> all belong to the same foreign server and are assigned to the same user to
> check access permissions as (cf checkAsUser), these fields will be set:

* serverid (Oid): Oid of the server if this base relation is a foreign table. 
  Otherwise, it is InvalidOid

* userid (Oid): Oid of the user to check access (0/InvalidOid means the current user)

* useridiscurrent (bool): if true, the plannere assumes that the user is the current user.

  > QUES: why cannot the planner check this by comparing userid and the current user id?

* fdwroutine (FdwRoutine *): FDW hooks

* fdw_private (void *): Pointer to a heap allocation that FDW implementation can 
  use.


--------------------------------------------------------------------------------

> During planning join orders, if we know that this relation is unique when being
> joined to other relations, we cache this information in these 2 fields.
>
> Currently, Postgres only does this for base relation, 

* unique_for_rels (List<UniqueRelInfo>): The current relation is known unique to 
  these relations.

* non_unique_for_rels (Relids):  The current relation is known not unique to 
  these relations.

--------------------------------------------------------------------------------

> Used to optimize semi-joins when the current relation is RHS, convert semi-join
> to:
>
> 1. Deduplicate the RHS relation and store it in a temporary relation
>
>    > Deduplication is called "unique-ification" in Postgres's term. A deduplicated
>    > relation is unique-ified
>
> 2. Perform regular inner join between LHS and this temporary relation
>
> If this approach has lower cost

* unique_rel (RelOptInfo *): the deduplicated relation 

* unique_pathkeys (List<PathKey>):  If the deduplication is implemented using 
  sorting, this field stores the pathkeys that represent the ordering 
  requirements for the relation's output.
  
  > QUES: not totally understand pathkey when I wrote this

* unique_groupclause (List<SortGroupClause>): If the deduplication is implemented
  using hashing, this field stores the `GROUP BY` clauses.
  
  > TIL that
  >
  > ```sql
  > select distinct column from table
  > select column from table group by column
  > ```
  >
  > are equivalent, no wonder Postgres uses `SortGroupClause` to represent so many
  > clauses.

--------------------------------------------------------------------------------

> Scan and join restrictions

* baserestrictinfo (List<RestrictInfo>): For base relations, this field stores
  the 1 `RestrictInfo` for each non-join qualification clause that this relation
  participates.

* baserestrictcost (QualCost): For base relations, this field stores the 
  estimated cost of evaluating the `baserestrictinfo` clause for 1 tuple.

* baserestrict_min_security (uint): smallest security_level found in `baserestrictinfo`

* joininfo (List<RestrictInfo>): This field stores 1 `RestrictInfo` for each join
  clause that this relation participates. 

* has_elclass_joins (bool): 

--------------------------------------------------------------------------------

* consider_partitionwise_join (bool): If this is a partitioned relation, should we 
  consider partition-wise join?

--------------------------------------------------------------------------------
> inheritance links, if this is an otherrel (otherwise NULL)

* parent (RelOptInfo*): RelOptInfo of its dirent parent relation

* top_parent (RelOptInfo*)

  The inherience relationship can be expressed in a tree, this field stores the
  pointer to that tree's root (topmost parent).
  
* top_parent_relids (Relids): same as `top_parent->relids`, it is redundant, but 
  handy
  
  > QUES: why is this information so important?

--------------------------------------------------------------------------------

* part_schema (PartitionSchema): How is this relation partitioned?

* nparts (int): number of partitions. -1 means it is not set.

* boundinfo (PartitionBoundInfoData): partition bounds

* partbounds_merged (bool): True if partition bounds were created by `partition_bounds_merge()`

* partition_qual (List<?>): If this relation is not the root, i.e., it is not the
  patitioned  table itself, it is a partition. This field stores a list of 
  qualifications that it satisfies.

* part_rels (Array<RelOptInfo>): Partitions' `RelOptInfo` structures. Stored in the same 
  order as `boundinfo`

* live_parts (bitset): indexes to the `part_rels` array items that survice the 
  partition pruning

* all_parts (Relids): union of all the partition's `relids`

* partexprs

* nonnulable_partexprs

