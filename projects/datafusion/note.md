1. How to enable filter pushdown for `ParquetExec`

   ```rs
   <ListingTable as TableProvider>::supports_filter_pushdown()                      [Condition]
   <ListingTable as TableProvider>::scan()
   ParquetFormat::create_physical_plan()
   ParquetFormat.enable_pruning OR config.execution.parquet.pruning(true)           [Condition] 
   ParquetExec::new()
   ParquetExec.pushdown_filters OR config.execution.parquet.pushdown_filters(false) [Condition]
   <ParquetExec as ExecutionPlan>::execute()
   <ParquetOpener as FileOpener>::open
   ```

   For the steps marked with `[Condition]`, the condition has to be satisfied. 

2. 
