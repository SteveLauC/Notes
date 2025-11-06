# Sublinks types

 *	EXISTS_SUBLINK		EXISTS(SELECT ...)
 *	ALL_SUBLINK			(lefthand) op ALL (SELECT ...)
 *	ANY_SUBLINK			(lefthand) op ANY (SELECT ...)
 *	ROWCOMPARE_SUBLINK	(lefthand) op (SELECT ...)
 *	EXPR_SUBLINK		(SELECT with single targetlist item ...)
 *	MULTIEXPR_SUBLINK	(SELECT with multiple targetlist items ...)
 *	ARRAY_SUBLINK		ARRAY(SELECT with single targetlist item ...)
 *	CTE_SUBLINK			WITH query (never actually part of an expression), if it gets materialized (`SubPlan.subLinkType`)

# Fields

* `sublinkId` (int, in range `[1, n]`): Only set for `MULTIEXPR_SUBLINK`. Otherwise, 0

* `testexpr` (Node, expression): `lefthand` expression.

  Initially, this is the raw form of the `lefthand` expression. Then the analyzer
  converts it into a complete boolean expression that compares the `lefthand` 
  value(s) to `PARAM_SUBLINK` nodes representing the output columns of the
  `subselect`.
  
  Only non-NULL if this is a `ALL/ANY/ROWCOMPARE` sublink

* `operName` (List<>): name of the `op` operator, see above

  Only non-NULL if this is a `ALL/ANY/ROWCOMPARE` sublink

* `subselect`: the sub-SELECT

  Initially, this field contains a raw parsetree. Then the analyzer
  converts it into a parsetree `Query`