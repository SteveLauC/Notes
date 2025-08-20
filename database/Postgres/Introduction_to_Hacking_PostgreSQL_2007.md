# Query processing stages

> [The Path of a Query](https://www.postgresql.org/docs/17/query-path.html)

1. Traffic Cop

   Traffic cop, dispatches request to proper module. This contains the postgres 
   backend main handler, as well as the code that makes calls to the parser, 
   optimizer, executor, and commands functions.

   * `src/backend/tcop`
   * `src/include/tcop`

2. Parser (SQL String -> parsetree list)

   A SQL string could contain multiple statements: `SELECT 1; SELECT 2; SELECT 3`.
   Every statement is parsed into a `struct RawStmt`(called parsetree).  So the parser converts a SQL query
   string to a list of parsetree.

3. Analyzer (aka, Binder, a parsetree -> a `struct Query`(called a Query tree))

   Resolve column references, metadata (schema/table/column) check, produces
   query 
   
   code: `parser/analyze.c`

4. Rewriter (`struct Query` -> `[struct Query]`, a Query Tree -> a Query Tree list)

   Rule and view support.

   Rewriter rewrites the query using [rules](https://www.postgresql.org/docs/current/sql-createrule.html) 
   and expands views if used. (Views are implemented using the rules mechanism.)
   
   This is NOT rule-based query optimization. 
   
   code: `src/backend/rewrite`
   
5. Planner/Optimizer (Query Tree -> Plan Tree)

   Planner takes a query and generates a plan (Physical plan), Postgres does CBO
   in this stage.
   
   file: `src/backend/optimizer`
   
6. Executor (execute the plan)

# Postgres object systems

> A convention that makes C object-oriented

Postgres uses a simple object system with support for single inheritance. The 
root of the class hierarchy is `struct Node` (src/include/nodes/nodes.h):

```c
/*
 * The first field of a node of any type is guaranteed to be the NodeTag.
 * Hence the type of any node can be gotten by casting it to Node. Declaring
 * a variable to be of Node * (instead of void *) can also facilitate
 * debugging.
 */
typedef struct Node
{
	NodeTag		type;
} Node;
```

inheritance means the child type should inherit all its parent type's fields,
Postgres emulates this by requiring that a child type's first field should be
its parent type. Here is an example of the `Plan` type, the parent type of
all the physial plan types:

```c
/* ----------------
 *		Plan node
 *
 * All plan nodes "derive" from the Plan structure by having the
 * Plan structure as the first field.  This ensures that everything works
 * when nodes are cast to Plan's.  (node pointers are frequently cast to Plan*
 * when passed around generically in the executor)
 *
 * We never actually instantiate any Plan nodes; this is just the common
 * abstract superclass for all Plan-type nodes.
 * ----------------
 */
typedef struct Plan
{
	pg_node_attr(abstract, no_equal, no_query_jumble)

	NodeTag		type;


/*
 * ==========
 * Scan nodes
 *
 * Scan is an abstract type that all relation scan plan types inherit from.
 * ==========
 */
typedef struct Scan
{
	pg_node_attr(abstract)

	Plan		plan;
	/* relid is index into the range table */
	Index		scanrelid;
} Scan;
```

By doing so, you can cast `Child *` as `Parent *` since the first `sizeof(sturct Child)` 
bytes of `Child *` is valid for the parent type.

C compilers never reorder struct fields, so this trick will work.

To construct a `Node` (Or any child Node types), use the `makeNode()` macro:

```c
// src/include/nodes/nodes.h

/*
 * newNode -
 *	  create a new node of the specified size and tag the node with the
 *	  specified tag.
 *
 * !WARNING!: Avoid using newNode directly. You should be using the
 *	  macro makeNode.  eg. to create a Query node, use makeNode(Query)
 */
static inline Node *
newNode(size_t size, NodeTag tag)
{
	Node	   *result;

	Assert(size >= sizeof(Node));	/* need the tag, at least */
	result = (Node *) palloc0(size);
	result->type = tag;

	return result;
}

#define makeNode(_type_)		((_type_ *) newNode(sizeof(_type_),T_##_type_))
```

To check if a node is of type `A`, use `IsA()`

# Memory management

1. Postgres's memory context is similar to arena

2. Memory contexts are arranged in a tree, deleting/resetting a node does the same
   to its children nodes.

