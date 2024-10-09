> This chapter is about:
>
> * Before I read it
>
>   * Introduction to HNSW index
>   * How does pgvector store the HNSW index within Postgres
>
> * After I read it
>   * How does pgvector store the HNSW index within Postgres

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
>  
>   * What HNSW is and how it works
>   * How does pgvector store the HNSW index within Postgres
>   * [Difference between IVFFlat index](https://tembo.io/blog/vector-indexes-in-pgvector)
>     * build time
>     * size on disk
>     * Query performance
>     * If it is sensitive to update
>
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case

> What have you learned from it
>
> *
> *

# Overview of Postgres Storage

1. In a Postgres page, why do we have `ItemId`
   
   For heap storage, A tuple is referred by `(PageID, ItemId)`, the actual
   tuples can be moved within the page, e.g., in the vacuum process, dead
   tuples will be removed, and the tuples within this page will defragmented,
   meaning that tuples will be moved. With another layer of indirection `ItemId`,
   such a move won't break the tuple identifier.

# pgvector HNSW index page layout

1. Every page of the HNSW index has a Postgres page header:

   ```c
   typedef struct PageHeaderData
   {
      /* XXX LSN is member of *any* block, not only page-organized ones */
      PageXLogRecPtr pd_lsn;		/* LSN: next byte after last byte of xlog
                          * record for last change to this page */
      uint16		pd_checksum;	/* checksum */
      uint16		pd_flags;		/* flag bits, see below */
      LocationIndex pd_lower;		/* offset to start of free space */
      LocationIndex pd_upper;		/* offset to end of free space */
      LocationIndex pd_special;	/* offset to start of special space */
      uint16		pd_pagesize_version;
      TransactionId pd_prune_xid; /* oldest prunable XID, or zero if none */
      ItemIdData	pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* line pointer array */
   } PageHeaderData;
   ```

## Metadata Page

1. Metadata page is the first page (page 0) in the index file, which contains
   the metadata

## Index Page