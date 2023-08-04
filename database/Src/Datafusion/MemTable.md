1. These are three structures that make `MemTable` a table source:


   * Memtable: implements `TableProvider`
   * Memexec: implements `ExecutionPlan`
   * MemoryStream: implements `SendableRecordBatchStream` and `futures::stream::Stream`

2. You can convert SQL to logical plan via


   ```rust
   let ctx = SessionContext::new();
   ctx.register_table("table_name", Arc::new(table))?;
   let sql = "select * from table_name";

   let state = &ctx.state();
   let logical_plan = state.create_logical_plan(sql)?;
   ```

   > Under the hood, it parses the SQL statement to AST (statement) and
   > call `state.statement_to_plan(ast: Statement)`

   To convert logical plan to physical plan:

   ```rust
   let physical_plan = state.create_physical_plan(logical_plan)?;
   ```

3. To support another query language, one can implement a function that translate
   its AST to a logical plan.

4. Basic stack trace behind

   ```rust
   let df = ctx.sql("select * from mem limit 1");
   df.show().await?;
   ```

   ```rust
   let sql = "select * from mem limit 1";
   let statement = ctx.state().sql_to_statement(sql, "MySQL")?;
   let logical_plan = ctx.state().statement_to_plan(statement).await?;
   let physical_plan = ctx.state().create_physical_plan(&logical_plan).await?;
   let steam = physical_plan::execute_stream(physical_plan, ctx.task_ctx())?;
   let batches = collect(steam).await?;
   let result = datafusion::arrow::util::pretty::pretty_format_batches(
       batches.as_slice(),
   )?;
   println!("{}", result);
   ```

   BTW, `DataFrame` is basically a wrapper for logical PLan:

   ```rust
   #[derive(Debug, Clone)]
   pub struct DataFrame {
       session_state: SessionState,
       plan: LogicalPlan,
   }
   ```

5. 
