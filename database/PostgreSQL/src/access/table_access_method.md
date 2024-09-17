> This chapter is about:
>
> * Before I read it
>
>   * How Postgres does table CRUD
>
> * After I read it
>
>   * The interfaces of Postgres's storage engine

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
>
>   1. Figure out the semantics of these interfaces, including their arguments 
>      and return value.
>   2. Take a glance at how the `heap` AM implements them
>   3. ?Be able to write my own custom storage engine via `pgrx`
>
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case

> What have you learned from it
>
> *
> *

# Navigation

* `src/include/access/tableam.h`: Table AM interfaces definitions
* `src/backend/access/table/tableam.c`
* `src/backend/access/heap/heapam_handler.c`: impl `TableAmRoutine` with functions 
* `src/backend/access/heap/heapam.c`: the actual function code used in `heapam_handler.c`

# `src/include/access/tableam.h`

1. `TableAmRoutine`

   This types defines various call backs a implementor need to fill in.

   * `type`: it has to be `T_TableAmRoutine`  
   * `slot_callbacks`: I am not sure about what does slot mean here, I guess it
     refers to a unit in the buffer pool, and since most AM impls will bring 
     their own buffer impl, we set this to `TTSOpsVirtual`, which basically means
     a virtual slot.
   * `scan_begin`: Begins a scan of the relation specified by `rel`.

     ```c
     TableScanDesc (*scan_begin) (Relation rel,
					 Snapshot snapshot,
					 int nkeys, struct ScanKeyData *key,
					 ParallelTableScanDesc pscan,
					 uint32 flags);
     ``` 

   * `scan_end`: Do resource de-allocation 
   * 