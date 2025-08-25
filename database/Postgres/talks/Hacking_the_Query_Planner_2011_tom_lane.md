1. > If the planner can’t generate the plan you want, you need to fix the 
   > mechanism.  If it generates and rejects the plan you want, you need 
   > to fix the cost estimation

   So true!

2. Cost estimation 

   ![theory](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-08-25%20at%201.56.05%E2%80%AFPM.png)
   ![reality](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-08-25%20at%201.56.29%E2%80%AFPM.png)

3. Size estimation

   * Result size for relation scans

     * row count: tuple density (`(# live tuples / # blocks)`) x `# blocks`
     * number of rows returned by a query: selectivity * table row count
     * average row width: sum of per-column average width

   * Result size for joins

     * row count: number of rows returned from Cartesian product * Join condition 
       selectivity
       
       > outer join needs special twiddling as it returns more rows

     * width: sum of the per-column average widths for variables **that are needed
       above the join**
    