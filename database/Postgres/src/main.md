> path: src/backend/main/main.c

1. The main function of the backend process mainly does one thing, parse the 
   dispatch option (binary's first option, the second argument, `argv[1]`) to
   one of the following values:

   ```c
   /*
    * Returns the matching DispatchOption value for the given option name.  If no
    * match is found, DISPATCH_POSTMASTER is returned.
   */
   DispatchOption parse_dispatch_option(const char *name);
   ```

   ```c
   // path: src/include/postmaster/postmaster.h

   /*
    * These values correspond to the special must-be-first options for dispatching
    * to various subprograms.  parse_dispatch_option() can be used to convert an
    * option name to one of these values.
   */
   typedef enum DispatchOption
   {
       DISPATCH_CHECK,
       DISPATCH_BOOT,
       DISPATCH_FORKCHILD,
       DISPATCH_DESCRIBE_CONFIG,
       DISPATCH_SINGLE,
       DISPATCH_POSTMASTER,		/* must be last */
   } DispatchOption;
   ```

   then call the corresponding role's main function:

   ```c
       switch (dispatch_option)
       {
           case DISPATCH_CHECK:
               BootstrapModeMain(argc, argv, true);
               break;
           case DISPATCH_BOOT:
               BootstrapModeMain(argc, argv, false);
               break;
           case DISPATCH_FORKCHILD:
   #ifdef EXEC_BACKEND
               SubPostmasterMain(argc, argv);
   #else
               Assert(false);		/* should never happen */
   #endif
               break;
           case DISPATCH_DESCRIBE_CONFIG:
               GucInfoMain();
               break;
           case DISPATCH_SINGLE:
               PostgresSingleUserMain(argc, argv,
                                   strdup(get_user_name_or_exit(progname)));
               break;
           case DISPATCH_POSTMASTER:
               PostmasterMain(argc, argv);
               break;
       }
    ```


