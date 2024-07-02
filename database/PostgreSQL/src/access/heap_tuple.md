> This chapter is about:
>
> * Before I read it
>
>   * How Postgres stores **heap** tuples (data)
>
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>
>   * Heap tuple format
>   * How var-len fields are stored
>     1. Without TOAST
>     2. With TOAST
>   * How NULL is represented
>   * Alignment requirements for different types
>   * Is padding added before data or after data

# Navigation

* `src/include/access/htup_details.h` heap tuple header definition (includes `htup.h`)
* `src/include/access/htup.h` heap tuple definition
* `src/include/access/heapam.h`: defines heap access methods (CRUD)
* `src/backend/access/heapam.c`: implements heap access methods (CRUD)

# `src/include/access/htup_details.h`   


