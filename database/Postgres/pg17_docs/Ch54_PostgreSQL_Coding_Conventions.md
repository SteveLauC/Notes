> * 54.1. Formatting
> * 54.2. Reporting Errors Within the Server
> * 54.3. Error Message Style Guide
> * 54.4. Miscellaneous Coding Conventions

# 54.1. Formatting

# 54.2. Reporting Errors Within the Server

How to use `ereport()`:

`ereport()` accepts a severity level (required, see "src/include/utils/elog.h"
for the full list) and a number of auxiliary  functions, among which `errmsg()` 
is required, all other functions are all optional. Auxiliary functions can be 
written  in any order, but the typical convention is that `errcode()` and 
`errmsg()` should appear first if they present.

```c
ereport(ERROR,
        errcode(ERRCODE_DIVISION_BY_ZERO),
        errmsg("division by zero"));
```

For `ereport()` that are called during query execution, if the severity level is 
higher than or equal to `ERROR`, then the current txn will be aborted.

Here is the full list of the auxiliary functions:


* `errcode(sqlerrorcode)`: If this is not called, then the SQLSATATE error code defaults to

  * `ERRCODE_INTERNAL_ERROR` if the severity code is `ERROR` or higher
  * `ERRCODE_WARNING` if the severity code is `WARNING`
  * `ERRCODE_SUCCESSFUL_COMPLETION` for `NOTICE` and below
  
* `errmsg(const char * msg, ...)`

  `errmsg()` accepts runtime parameters: 

   ```c
   ereport(LOG, errmsg("hello, %s", "world"));
   ```
   
   Besides the format strings supported by `printf()`, `%m` is a special format 
   string that will be replaced by `"%s", strerror(errno)`.
   
   ```c
   report(ERROR,
                   (errcode(ERRCODE_EXTERNAL_ROUTINE_INVOCATION_EXCEPTION),
                    errmsg("could not open file \"%s\": %m", filename),
   ```
   
   You can see that `%m` does not require any corresponding entry in the parameter 
   list, we only have `filename` in the list.
   
   Note that the `msg` argument ("could not open file \"%s\": %m" in the above 
   example) of `errmsg()` will be treated as i18n message ID and passed to `gettext()`
   (GNU i18n toolkit) to looks for the localized message.

* `errmsg_internal(const char *msg, ...)`: Same as `errmsg()` except that `msg` 
  won't be as the i18n message ID and get translated.
  
  This should be used for “cannot happen” cases that are probably not worth 
  expending translation effort on.

* ``
   