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