#### Ch11: System Limits and Options

> 1. the standard minimal limit
> 2. implementation limit
> 3. get the standard minimal limit at compile time
> 4. get implementation limit at compile and run time
> 5. get system options at compile and run time

1. For each `limit`, SUSv3 **requires** that all implementations support a 
   minimum limit (minimum value of its `upper limit`). 

   If an application restricts itself to the minimum limit, then it will be 
   portable to all implementations as this is guaranteed to be supported by all
   impls. (See Note 3 the values of `MQ_PRIO_MAX` and `_POSIX_MQ_PRIO_MAX` then
   you will get it.)

   Those `minimum limit` normally can be found in `limits.h`, and with a name
   of `_POSIX_XXX_MAX`. The `MAX` suffix may be bewildering, but the standard
   states this limit is the **minimum upper limit**.

   In some cases, `maximum limit (maximum value of its lower limit)` is also
   provided with the name `XX_MIN`.

   > `minimum limit` and `maximum limit` are the standard stuffs.

   Each limit has a `name`, with the name `XXX_MAX`, defined in `limits.h`,
   and is guaranteed that `XXX_MAX >= _POSIX_XXX_MAX` (This verify that if one
   application uses the minimum limit defined by SUSv3, then this program
   is totally portable for the reason that `_POSIX_XXX_MAX` is definitely 
   supported by the impl as `XXX_MAX>=_POSIX_XXX_MAX`).

   > The name of limit, `XXX_MAX` is the distro stuff.

2. Limit category

   SUSv3 divides the limits into 3 categories:

   1. runtime invariant values
   2. pathname variable values
   3. runtime increasable values

   > Honestly, I didn't get how this is devided.

3. Runtime invariant values (possibly indeterminate)

   If this is defined in `limits.h`, then it is fixed. If not, this may be 
   **indeterminate** and relies on the amount of avaiable memory.

   Regardless of whether it is defined in `limits.h`, an application can use 
   `sysconf(3)` to obtain it.

   Example: `MQ_PRIO_MAX`

   the max priority of messages in POSIX message queues. The standard sets 
   `_POSIX_MQ_PRIO_MAX` to be `32`. On Linux, `MQ_PRIO_MAX` is set as `32768`
   in `limits.h` and we can also use `sysconf(_SC_MQ_PRIO_MAX)` to query this.
  
   ```c
   #include <stdlib.h>
   #include <unistd.h>
   #include <stdio.h>
   #include <limits.h>
   
   int main(void)
   {
            printf("_POSIX_MQ_PRIO_MAX: %d\n\n", _POSIX_MQ_PRIO_MAX);
            
            printf("MQ_PRIO_MAX from limits.h: %d\n", MQ_PRIO_MAX);
            long limit_at_runtime = sysconf(_SC_MQ_PRIO_MAX);
            if (limit_at_runtime == -1) {
                    perror("sysconf");
                    exit(EXIT_FAILURE);
            }
            printf("MQ_PRIO_MAX obtained at runtime: %ld\n", limit_at_runtime);
    
            return 0;
   }
   ```
   
   ```shell
   $ gccs mian.c && ./a.out
   _POSIX_MQ_PRIO_MAX: 32

   MQ_PRIO_MAX from limits.h: 32768
   MQ_PRIO_MAX obtained at runtime: 32768
   ```
   
4. Pathname variable values

   This category is the limits about pathnames.

   Each limit may be a constant for a implementation, or can vary from one file
   system to another.

   If it is related to pathname, we can use `pathconf/fpathconf` to query it.

   Example: NAME_MAX

   Maximum size of a *filename*.

   The standard defines `_POSIX_NAME_MAX` to be `14`(the old system V fs limit),
   and Linux defines `NAME_MAX` to be `255`.

   ```c
   #include <unistd.h>
   #include <stdio.h>
   #include <limits.h>
   
   int main(void)
   {
           printf("_POSIX_NAME_MAX from limits.h: %d\n", _POSIX_NAME_MAX);
           printf("NAME_MAX from limits.h: %d\n", NAME_MAX);
   
           printf("\nIf on some implementation, this is fs (or path) dependent.\n");
           printf("We can use `pathconf/fpathconf` to query it\n");
   
           long m1 = pathconf("/", _PC_NAME_MAX);
           long m2 = pathconf("/home", _PC_NAME_MAX);
   
           printf("_PC_NAME_MAX of \"/\": %ld\n", m1);
           printf("_PC_NAME_MAX of \"/home\": %ld\n", m2);
           printf("\n> Seems this is not fs/path related under Linux\n");
           return 0;
   }
   ```
   ```shell
   $ gccs main.c && ./a.out
   _POSIX_NAME_MAX from limits.h: 14
   NAME_MAX from limits.h: 255
   
   If on some implementation, this is fs (or path) dependent.
   We can use `pathconf/fpathconf` to query it
   _PC_NAME_MAX of "/": 255
   _PC_NAME_MAX of "/home": 255
   
   > Seems this is not fs/path related under Linux
   ```

5. Runtime increasable values
  
   A runtime increasable value is a limit that has a fixed minimum value for a
   particular implementation, and the system running this implementation can
   increase this at runtime.

   Example: 

   `NGROUPS_MAX`: the maximum number of supplementary groups to which a user
   belongs.

   The standard has `_POSIX_NGROUPS_MAX` to be set as `8`

   ```c
   #include <unistd.h>
   #include <stdio.h>
   #include <limits.h>
   
   int main(void)
   {
           printf("_POSIX_NGROUPS_MAX from limits.h: %d\n", _POSIX_NGROUPS_MAX);
           printf("NGROUPS_MAX from limits.h: %d\n", NGROUPS_MAX);
           long runtime_value = sysconf(_SC_NGROUPS_MAX);
           printf("NGROUPS_MAX obtained at runtime: %ld\n", runtime_value);
           return 0;
   }
   ```

   ```shell
   $ gccs main.c && ./a.out
   _POSIX_NGROUPS_MAX from limits.h: 8
   NGROUPS_MAX from limits.h: 65536
   NGROUPS_MAX obtained at runtime: 65536
   ```

6. limit name prefixed with `SC` or `PC`

   If the limit name is prefixed with `SC`, then this is for `sysconf`.
   `PC` is for `pathconf/fpathconf`

7. obtain limits using shell command

   ```
   $ getconf LIMIT [pathname]
   ```

   If the limit you wanna query is path related, you must supply the `pathname`
   option.

   ```shell
   $ getconf ARG_MAX
   2097152
   
   $ getconf NAME_MAX /home
   255
   
   $ getconf NAME_MAX /
   255
   ```

8. retrieve limit at runtime using `sysconf(3)`
   
   ```c
   #include <unistd.h>

   long sysconf(int name);
   ```

   return `-1` on both error and cases where `name` can not be determined. So 
   one may need to determine whether it is on error using the value of `errno`, 
   to do this, set `errno` to 0 before the call.

   And thank God, we do have a rusty encapsulation for this in `nix`:

   ```rust
   /// Get configurable system variables (see
   /// [sysconf(3)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/sysconf.html))
   ///
   /// Returns the value of a configurable system variable.  Most supported
   /// variables also have associated compile-time constants, but POSIX
   /// allows their values to change at runtime.  There are generally two types of
   /// sysconf variables: options and limits.  See sysconf(3) for more details.
   ///
   /// # Returns
   ///
   /// - `Ok(Some(x))`: the variable's limit (for limit variables) or its
   ///     implementation level (for option variables).  Implementation levels are
   ///     usually a decimal-coded date, such as 200112 for POSIX 2001.12
   /// - `Ok(None)`: the variable has no limit (for limit variables) or is
   ///     unsupported (for option variables)
   /// - `Err(x)`: an error occurred
   pub fn sysconf(var: SysconfVar) -> Result<Option<c_long>> {
       let raw = unsafe {
           Errno::clear();
           libc::sysconf(var as c_int)
       };
       if raw == -1 {
           if errno::errno() == 0 {
               Ok(None)
           } else {
               Err(Errno::last())
           }
       } else {
           Ok(Some(raw))
       }
   }
   ```

   And the standard requires that the value retrieved from `sysconf` will be constant
   in the lifetime of this process. But under Linux, there are some exceptions. For
   some special limits, one can use `setrlimit(2)` to change it.

9. obtain file-related limits at runtime using `pathconf(3)` and `fpathconf(3)`

   ```c
   #include <unistd.h>
   
   long fpathconf(int fd, int name);
   long pathconf(const char *path, int name);
   ```

   Both return the corresponding limit value on success, and `-1` if limit is
   indeterminate or an error occurred.

   Unlike `sysconf(3)`, the standard doesn't require the return value remains
   constant in the lifetime of the calling process since the relevant file-system
   may be mounted or dismounted in that duration.

10. when a limit is indeterminate, what should we do
    
    1. use the standard minimal limit `_POSIX_XX_MAX`
    2. retry
    3. estimate the limit using `sysconf` and `pathconf`
       > How? These functions return -1 if `name` is indeterminate
    4. use GNU [`autoconf`](https://www.gnu.org/software/autoconf/)

11. system options
    
    The standard specifies various options a UNIX implementation may support.
    Whether these options are supported can be retrieved at compile and run 
    time.

    query support at compile time, these are constants defined in `unistd.h`
    to indicate whether an option is supported.
    1. If the constant has value -1, then it's not supported.
    2. has value 0, then we need to query it at runtime using `sysconf/pathconf`
    3. has value greater than 0, is supported.

    ```c
    #include <unistd.h>
    #include <stdio.h>
    
    int main(void)
    {
            if (_POSIX_JOB_CONTROL > 0) {
                    printf("job control is supported\n");
            } else if (_POSIX_JOB_CONTROL == 0) {
                    long res = sysconf(_SC_JOB_CONTROL);
                    if (res > 0) {
                            printf("job control is supported\n");
                    } else {
                            printf("job control is not supported\n");
                    }
            } else {
                    printf("job control is not supported\n");
            }
            return 0;
    }
    ```
  
