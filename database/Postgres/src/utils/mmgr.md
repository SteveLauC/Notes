1. The current memory context 

   > It is just a pointer, just a real memory context, it points to real memory 
   > contexts.

   It is denoted by the global variable `CurrentMemoryContext`.  `palloc()` allocates
   from this context.  `MemoryContextSwitchTo(new_ctx)` updates the current memory
   context to `new_ctx` and returns the previous one.
   
   It exists because we don't want to add an extra `mctx` argument to every 
   function call.  And the philosophy here is that `CurrentMemoryContext` should 
   point to a ctx with a short lifetime. (for what? Per the README, to avoid
   potential memory leak)

2. Different behaviors between `palloc` and `malloc`

   1. `palloc()` and its friends would `elog(ERROR)` upon OOM, which should abort
      the process.  `malloc` returns NULL.  However,  `palloc_extended()` can be 
      configured to return NULL upon OOM with the `flag` argument.
   
      ```c
      /*
       * Flags for MemoryContextAllocExtended.
       */
      #define MCXT_ALLOC_HUGE			0x01	/* allow huge allocation (> 1 GB) */
      #define MCXT_ALLOC_NO_OOM		0x02	/* no failure if out-of-memory */
      #define MCXT_ALLOC_ZERO			0x04	/* zero allocated memory */
      ```
      
   2. Some `malloc()` impl does not allow `malloc(0)`, while glibc allow this.
      `palloc(0)` is allowed as well. 
      
   3. `free(ptr)` and `realloc(ptr, size)` can set `ptr` to NULL, `free(NULL)`
      is no-op, and `reallo(NULL, size)` is equivalent to `malloc(size)`.
      
      `pfree()` and `prealloc()` do not accept NULL.

3. `pfree(ptr)` and `realloc(ptr, size)` do not depend on memory context.  If
   the `ptr` argument is owned by a memory context that is not the current
   memory context, these 2 functions still work.
   
4. Postgres has a lot of memory contexts, to make their management easier, they
   are arranged in a tree.
   
   > Let me quote the doc:
   >
   > If all contexts were independent, it'd be hard to keep track of them,
   > especially in error cases.  That is solved by creating a tree of
   > "parent" and "child" contexts.  When creating a memory context, the
   
   > QUES: I don't understand why it should be a tree. Gemini does not know this
   > either.
   >
   > When I wrote C code, I do remember that I forgot to free the memory when 
   > code goes wrong, but how MemoryContext will help in this case? 
   >
   > Future steve: you are right
   
   
   When you create a new MemoryContext using `AllocSetContextCreateInternal()`,
   you have to tell this function which MemoryContext is this new mctx's parent
   node.
  
   ```c
   MemoryContext
   AllocSetContextCreateInternal(MemoryContext parent,
							  const char *name,
							  Size minContextSize,
							  Size initBlockSize,
							  Size maxBlockSize)
   ``` 
   
   The root of this tree is `TopMemoyContext`
   
5. Globally known contexts

   * TopMemoryContext: Root of the MemoryContext tree.  This context itself is 
     a **Permanent** context (`'static`), Valid for the entire lifetime of the 
     backend process.
     
     So you mainly allocate things that should live forever in this context. Or 
     you can allocate stuff with shorter lifespan, but you do need to clean it.
     
     Avoid making `CurrentMemoryContext` point to this since it should have a
     short lifespan.
   
   * ErrorContext: Permanent context, used for error recovery, will be reset at
     the end of the recovery.  It is a child of the `TopMemoryContext`:
     
     ```c
     /*
	  * First, initialize TopMemoryContext, which is the parent of all others.
	 */
	 TopMemoryContext = AllocSetContextCreate((MemoryContext) NULL,
											 "TopMemoryContext",
											 ALLOCSET_DEFAULT_SIZES);
	 CurrentMemoryContext = TopMemoryContext;
     ErrorContext = AllocSetContextCreate(TopMemoryContext,
										 "ErrorContext",
										 8 * 1024,
										 8 * 1024,
										 8 * 1024);
     ```
     
     An out-of-memory error could be reported because we reserved some memory 
     in `ErrorContext`. 
     
     Postgres will switch to it in `errfinish()`.
     
   * CacheMemoryContext: Permanent context, used to allocate various caches, such 
     as relcache.
   
   * PostmasterContext: this is the postmaster's normal working context.
   
     ```c
     /*
	  * By default, palloc() requests in the postmaster will be allocated in
	  * the PostmasterContext, which is space that can be recycled by backends.
	  * Allocated data that needs to be available to backends should be
	  * allocated in TopMemoryContext.
	 */
	 PostmasterContext = AllocSetContextCreate(TopMemoryContext,
	      "Postmaster",
          ALLOCSET_DEFAULT_SIZES);
     ```
     
     Backend processes forked by Postmaster inherient this MemoryContext, but they
     
     ```c
     /*
	  * If the PostmasterContext is still around, recycle the space; we don't
	  * need it anymore after InitPostgres completes.
	 */
	 if (PostmasterContext)
	 {
	 	MemoryContextDelete(PostmasterContext);
		PostmasterContext = NULL;
	 }
     ```
     
   
   * MessageContext: store the message passed from the frontend. It has the same
     lifetime of a iteration in `PostgresMain()`
   

   
   