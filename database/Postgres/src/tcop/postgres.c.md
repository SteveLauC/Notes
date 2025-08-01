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

5. Every backend process will store a `struct PGPROC` in the shared memory

6. `exec_simple_query(const char *query_string)` is the function that both `psql`
   and the standalone `postgres` backend use to execute queries.