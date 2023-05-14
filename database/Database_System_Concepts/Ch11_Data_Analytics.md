> * 11.1 Overview of Analytics
> * 11.2 Data Warehousing
> * 11.3 Online Analytical Processing
> * 11.4 Data Mining

1. The term `data analytics` refers broadly to the processing of data to infer
   patterns, correlations, or models for prediction.

   Here, each decision is low-value, but with high volumes the overall value of 
   making the right decisions is very high.

# 11.1 Overview of Analytics
1. Big companies usually gather data from multiple sources into one location, 
   referred to as `data warehouse`.

   During gathering, the steps of collecting data, cleaning/deduplicating the data,
   and loading the data into a warehouse are referred to as `extract, transform and
   load(ETL) tasks`.

# 11.2 Data Warehousing
1. A `data warehouse` is a repo of information gathered from multiple sources,
   stored under a unified schema, at a single site.

## 11.2.1 Components of a Data Warehouse
1. Components of a Data Warehouse
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/components_of_data_warehouse.jpg)

2. Key parts of a Data Warehouse
   
   1. When and how to gather data
      
      * For `source-driven architecture`, we can gather data:
        1. Continuously (as transaction processing takes place)
        2. Periodly (nightly, in a batch)
      * For `destination-driven architecture`, the `Data Warehouse` periodly sends
        requests for new data.
   2. What schema to use 
      
      Data sources that have been constructed independetly are likely to have
      different schemas. In fact, they may even use different `Data Model`s.

      Part  of the task of a warehouse is to perform schema integration and to
      convert data to the integrated schema before they are stored.

   3. Data transformation and cleansing

   4. How to propagate updates
      Data stored in `Data Warehouse` are not same as the data at the data source
      (due to data transformation and cleansing), propagating updates is not easy.

      > This is similar to the `view-maintainance` problem.

   5. What data to summarize
      In some cases, the data derived from a OLTP system is too large to be stored
      in a OLAP system, which means we need to summorize the data and only store
      the data that are really useful.

## 11.2.2 Multidimensional and Warehouse Schemas

1. `fact table` and `dimension table`
   
   The relations in a `data warehouse` can be classified into two categories:
   1. fact table: a fact table is usually a table that has multiple foreign keys
      pointing to other tables

   2. dimension table: tables that are pointed by the foreign keys in the `fact 
      table`.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/star_schema_for_data_warehouse.jpg)

2. `measure attribute`s and `dimension attribute`s in `fact table`

   * `Meature attribute` stores quantitative information, which can be aggregated
   upon.

   * `Dimension attribute`s are dimensions which can be grouped and viewed.

3. star schema

   The schema appeared in `1`. With a fact table, multiple dimension tables, and
   foreign keys from the fact table to the dimension tables is called `star schema`

   > In complex `star schema`, there may be multiple layers of dimension tables, 
   > i.e., `dimension table` may have a foreign key pointing to another table,
   > this is called `snowflake schema`.
   >
   > ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-14%2018-58-30.png)

   > Some complex schema may have more than one `fact table`s.

## 11.2.3 Database support for Data Warehouses

1. Diff between OLTP and OLAP systems
   
   * OLTP: 
     1. Queries are frequent but very small, involving updates in addition to
        reads. 
     2. Row oriented

   * OLAP: 
     1. Fewer queries but each query accesses a much larger amount of data.
     2. Data stored in a data warehouse are typically never updated.
     3. Column oriented

   > And thus, systems that are designed for OLTP are not suitable for OLAP.

## 11.2.4 Data Lakes

1. What is `Data Lakes`

   `Data Warehouse` does a lot of work to ensure there is a consistent schema
   for all the data sources to ease the job of querying the data.

   `Data Lake`s do not require font-end effort to process the data, but they
   do require more effort when creating queries.

# 11.3 Online Analytical Processing
# 11.4 Data Mining
