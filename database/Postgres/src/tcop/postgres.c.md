1. `static int InteractiveBackend(StringInfo inBuf)` is the function that takes user 
   input and places it in the buffer `inBuf`.

   It is only used in single-user mode.

2. `ReadCommand()` is the dispatcher for reading from input, input can be

   1. The socket connection
   2. stdin


   ```c
   /* ----------------
   *		ReadCommand reads a command from either the frontend or
   *		standard input, places it in inBuf, and returns the
   *		message type code (first byte of the message).
   *		EOF is returned if end of file.
   * ----------------
   */
   static int
   ReadCommand(StringInfo inBuf)
   {
       int			result;

       if (whereToSendOutput == DestRemote)
           result = SocketBackend(inBuf);
       else
           result = InteractiveBackend(inBuf);
       return result;
   }
   ```

3. `PostgresMain()` is the main entry point for backend process:

   ```c
   /* ----------------------------------------------------------------
    * PostgresMain
    *	   postgres main loop -- all backends, interactive or otherwise loop here
    *
    * dbname is the name of the database to connect to, username is the
    * PostgreSQL user name to be used for the session.
    *
    * NB: Single user mode specific setup should go to PostgresSingleUserMain()
    * if reasonably possible.
    * ----------------------------------------------------------------
   */
   void
   PostgresMain(const char *dbname, const char *username)
   ```

   If this backend process runs in single-user mode, then the call chain is 
   
   ```
   PostgresSingleUserMain() -> PostgresMain()
   ```

   If it is spawned by the postmaster, then the call chain is:


   ```
   BackendMain() -> PostgresMain()
   ```

   > BackendMain() is defined in `src/backend/tcop/backend_startup.c`

   It calls `ReadCommand()` in a for loop:

   ```c
   for (;;)
   {
        // ...

		/*
		 * (3) read a command (loop blocks here)
		 */
		firstchar = ReadCommand(&input_message);

        // ...
   }
   ```

4. `PostgresSingleUserMain()` calls `process_shared_preload_libraries()` to load shared 
   objects that need to be loaded when process start up.

   ```c
   /*
   * process any libraries that should be preloaded at postmaster start
   */
   void
   process_shared_preload_libraries(void)
   {
       process_shared_preload_libraries_in_progress = true;
       load_libraries(shared_preload_libraries_string,
                   "shared_preload_libraries",
                   false);
       process_shared_preload_libraries_in_progress = false;
       process_shared_preload_libraries_done = true;
   }
   ```

   Users can specify the objects they wanna load in `postgresql.conf`:

   ```text
   #shared_preload_libraries = ''		# (change requires restart)
   ```

5. Most backend processes will store a `struct PGPROC` in the shared memory

   ```c
   /*
    * MyBackendType indicates what kind of a backend this is.
    *
    * If you add entries, please also update the child_process_kinds array in
    * launch_backend.c.
    */
    typedef enum BackendType
    {
    	B_INVALID = 0,
    
    	/* Backends and other backend-like processes */
    	B_BACKEND,
    	B_DEAD_END_BACKEND,
    	B_AUTOVAC_LAUNCHER,
    	B_AUTOVAC_WORKER,
    	B_BG_WORKER,
    	B_WAL_SENDER,
    	B_SLOTSYNC_WORKER,
    
        // backend type for single-user mode
    	B_STANDALONE_BACKEND,
    
    	/*
    	 * Auxiliary processes. These have PGPROC entries, but they are not
    	 * attached to any particular database, and cannot run transactions or
    	 * even take heavyweight locks. There can be only one of each of these
    	 * running at a time, except for IO workers.
    	 *
    	 * If you modify these, make sure to update NUM_AUXILIARY_PROCS and the
    	 * glossary in the docs.
    	 */
    	B_ARCHIVER,
    	B_BG_WRITER,
    	B_CHECKPOINTER,
    	B_IO_WORKER,
    	B_STARTUP,
    	B_WAL_RECEIVER,
    	B_WAL_SUMMARIZER,
    	B_WAL_WRITER,
    
    	/*
    	 * Logger is not connected to shared memory and does not have a PGPROC
    	 * entry.
    	 */
    	B_LOGGER,
    } BackendType;
    ```

6. `exec_simple_query(const char *query_string)` is the function that both `psql`
   and the standalone `postgres` backend use to execute queries.
   
   # Stages
   
   1. Parser
   
      `List * pg_parse_query(const char *query_string)`, which invokes 
      `List * raw_parser(const char *str, RawParseMode mode)` from `src/backend/parser/parser.c`
   
   2. Analyzer & Rewriter
      
      `pg_parse_query()` returns a list of raw statement as the input `query_string`
      may contain multiple SQL statements.  
      
      ```c
      parsetree_list = pg_parse_query(query_string);
	  printf("DBG: %d statements\n", list_length(parsetree_list));
      ```
      
      Each statement will be analyzed and rewritten separately.  A list of `querytree`
      is returned because analyzer and rewriter could expand the query:
      
      ```c
      querytree_list = pg_analyze_and_rewrite_fixedparams(parsetree, query_string,
															NULL, 0, NULL);
      ```
      
      * Analyzer
      
        ```c
        // src/backend/parser/analyze.c
        
        /*
         * parse_analyze_varparams
         *
         * This variant is used when it's okay to deduce information about $n
         * symbol datatypes from context.  The passed-in paramTypes[] array can
         * be modified or enlarged (via repalloc).
         */
        Query *
        parse_analyze_varparams(RawStmt *parseTree, const char *sourceText,
						Oid **paramTypes, int *numParams,
						QueryEnvironment *queryEnv)
        
        /*
         * transformTopLevelStmt -
         *	  transform a Parse tree into a Query tree.
         *
         * This function is just responsible for transferring statement location data
         * from the RawStmt into the finished Query.
         */
        Query *
        transformTopLevelStmt(ParseState *pstate, RawStmt *parseTree)
        
        /*
         * transformOptionalSelectInto -
         *	  If SELECT has INTO, convert it to CREATE TABLE AS.
         *
         * The only thing we do here that we don't do in transformStmt() is to
         * convert SELECT ... INTO into CREATE TABLE AS.  Since utility statements
         * aren't allowed within larger statements, this is only allowed at the top
         * of the parse tree, and so we only try it before entering the recursive
         * transformStmt() processing.
         */
        static Query *
        transformOptionalSelectInto(ParseState *pstate, Node *parseTree)
        
        /*
         * transformStmt -
         *	  recursively transform a Parse tree into a Query tree.
         */
        Query *
        transformStmt(ParseState *pstate, Node *parseTree)
        ```
      
      * Rewriter
      
        `List * pg_rewrite_query(Query *query)` invokes `List * QueryRewrite(Query *parsetree)`
        from `src/backend/rewrite/rewriteHandler.c`.
      
   3. Plan
      
      ```c
      plantree_list = pg_plan_queries(querytree_list, query_string,
										CURSOR_OPT_PARALLEL_OK, NULL);
      ```
    
   