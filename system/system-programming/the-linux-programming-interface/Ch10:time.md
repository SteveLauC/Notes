#### Ch10: Time

> 1. time formats: calendar time, broken-down time, printable format
> 2. various syscalls and library functions used to convert between these time
>    formats
> 3. how is timezone expressed on Linux, some timezone-related commands and 
>    a environment variable $TZ
> 4. how is locale expressed on Linux, how to get or set locale settings
> 5. 3 functions for setting system clock
> 6. what is process time(user and system time), 2 syscalls used to obtain process
>    time.

1. The UNIX Epoch: January 1, 1970, is approximately the date (November 3, 1971)
   when the UNIX system came into being.

2. UNIX system use `time_t` to represent the seconds since `Epoch`, on 32-bit
   systems, `time_t` is an alias to `i32`, so the latest time this can represent
   is `03:14:07 UTC on 19 January 2038`, this is called `Year 2038 problem`.

   On 64-bit systems, this is fine as `time_t` is an alias to `i64`.

   ```rust
   pub type time_t = i64;
   ```

   > Such time represented in an absolute seconds elapsed since EPOCH is called
   > calendar time.

3. use `gettimeofday(2)` and `settimeofday(2)` to get or set calendar time

   ```c
   #include <sys/time.h>
   
   int gettimeofday(struct timeval *restrict tv, struct timezone *restrict tz);
   int settimeofday(const struct timeval *tv, const struct timezone *tz);
   ```


   These two syscalls are used to get or set time as well as timezone (not anymore).

   NOTE: argument `tz` is **obsolete** and always should be specified as `NULL`.

   ```c
   struct timeval {
       time_t      tv_sec;     /* seconds since 00:00:00, 1 Jan 1970 UTC */
       suseconds_t tv_usec;    /* microseconds */
   };

   // obsolete
   struct timezone {
       int tz_minuteswest;     /* minutes west of Greenwich */
       int tz_dsttime;         /* type of DST correction */
   };
   ```

   > Order of magnitude (time):
   >
   > (1) second = (10^3) milisecond = (10^6) microsecond = (10^9) nanosecond
   >
   > [wikipedia](https://simple.wikipedia.org/wiki/Orders_of_magnitude_(time))

   > And even though `timeval` has `microsec` precision, whether it is supported
   > is hardware dependent.

4. use `time(2)` to fetch calendar time
   
   ```c
   #include <time.h>

   time_t time(time_t *tloc);
   ```

   Returns the seconds since Epoch on success, `-1` on error.
   If `tloc` is not `NULL`, then the result is also stored in the memory pointed
   by `tloc`.

   The **ONLY** error that could happen is that `tloc` points to an invalid address.
   > EFAULT: tloc points outside your accessible address space (but see BUGS).

   So if you use `time(2)` like this, no error will happen at all:

   ```c
   time_t seconds_since_epoch = time(NULL);
   assert(seconds_since_epoch != -1);
   ```

   > The functionality of `time(2)` and `gettimeofday(2)` overlaps since `time(2)`
   > is provided by early UNIX implemention, and `gettimeofday(2)` is derived
   > from BSD.

5. use `ctime(3)` to convert `time_t` to printable form.
   
   ```c
   #include <time.h>

   // return statically allocated string terminated by '\0' and NUL.
   char *ctime(const time_t *timep);
   // put result in memory pointed by `buf`, and return `buf`.
   char *ctime_r(const time_t *restrict timep, char *restrict buf);
   ```

   These syscalls will handle `timezone` and `DST` for us automatically.

   ```c
   #include <string.h>
   #include <assert.h>
   #include <stdio.h>
   #include <time.h>
   
   int main(void)
   {
       time_t now = time(NULL);
       char buf[100];
       assert(ctime_r(&now, buf) != NULL);
       // remove the tailing newline
       long len = strlen(buf);
       buf[len - 2] = '\0';
       printf("%s\n", buf);
       return 0;
   }
   ```
   ```shell
   $ gccs main.c && ./a.out
   Thu Sep  1 07:24:26 202
   ```

6. use `gmtime(3)` and `localtime(3)` to convert `time_t` to broken-down time.

   [What is broken-down time](https://www.gnu.org/software/libc/manual/html_node/Broken_002ddown-Time.html)

   ```c
   struct tm {
       int tm_sec;    /* Seconds (0-60) can be 60 to account for the leap seconds */
       int tm_min;    /* Minutes (0-59) */
       int tm_hour;   /* Hours (0-23) */
       int tm_mday;   /* Day of the month (1-31) */
       int tm_mon;    /* Month (0-11) */
       int tm_year;   /* Year - 1900 */
       int tm_wday;   /* Day of the week (0-6, Sunday = 0) */
       int tm_yday;   /* Day in the year (0-365, 1 Jan = 0) */
       int tm_isdst;  /* Daylight saving time */
   };
   ``` 

   When `_BSD_SOURCE` is defined, this struct also includes two additional fields,
   This is a BSD extension, present in 4.3BSD-Reno.

   ```
   long tm_gmtoff;           /* Seconds east of UTC */
   const char *tm_zone;      /* Timezone abbreviation */
   ```

   ```c
   struct tm *gmtime(const time_t *timep);
   struct tm *gmtime_r(const time_t *restrict timep, struct tm *restrict result);

   struct tm *localtime(const time_t *timep);
   struct tm *localtime_r(const time_t *restrict timep, struct tm *restrict result);
   ```

   Diff between `gmtime(3)` and `localtime(3)`:

   * The gmtime() function converts the calendar time timep to broken-down time
     representation, expressed in **Coordinated Universal Time (UTC)**.

     > `gm` means `Greenwich Mean Time`
     > 
     > [Diff between `GMT` and `UTC`](https://stackoverflow.com/questions/48942916/what-is-the-difference-between-utc-and-gmt)
     > 
     > I didn't get it, but seems these two things have the same time.


   * The  localtime()  function converts the calendar time timep to broken-down
     time representation, expressed **relative to the user's specified timezone**.


   ```c
   #include <string.h>
   #include <stdio.h>
   #include <time.h>
   
   int main(void)
   {
           time_t now = time(NULL);
           struct tm buf1;
           struct tm buf2;
   
           gmtime_r(&now, &buf1);
           localtime_r(&now, &buf2);
   
           printf("gmtime_r: day: %d hour: %d\n", buf1.tm_mday, buf1.tm_hour);
           printf("localtime_r: day: %d hour: %d\n", buf2.tm_mday, buf2.tm_hour);
           return 0;
   }
   ```
   ```shell
   $ date
   Thu Sep  1 07:57:02 AM CST 2022
   
   $ gccs main.c && ./a.out
   gmtime_r: day: 31 hour: 23
   localtime_r: day: 1 hour: 7
   ```

7. use `mktime(3)` to convert *local* broken-down time to calendar time:

   ```c
   #include <time.h>

   time_t mktime(struct tm *tm);
   ```

   This function ignores the `tm_wday` and `tm_yday` fields

   > If you pass `struct tm` return by `gmtime(3)` to `mktime(3)`, `mktime(3)`
   > will return an erroneous value.

8. use `asctime(3)` to convert Broken-down time to printable form
  
   ```c
   #include <time.h>

   char *asctime(const struct tm *tm);
   char *asctime_r(const struct tm *restrict tm, char *restrict buf);
   ``` 
   
   ```c
   #include <stdio.h>
   #include <time.h>
   
   int main(void)
   {
           time_t now = time(NULL);
           struct tm buf1;
           struct tm buf2;
   
           gmtime_r(&now, &buf1);
           localtime_r(&now, &buf2);
   
           printf("%s\n", asctime(&buf1));
           printf("%s\n", asctime(&buf2));
           printf("%s\n", ctime(&now));
           return 0;
   }
   ```
   ```shell
   $ gccs main.c && ./a.out
   Thu Sep  1 01:57:57 2022
   
   Thu Sep  1 09:57:57 2022
   
   Thu Sep  1 09:57:57 2022
   ```

9. use `strftime(3)` to convert `broken-down time` to printable form with
   **more precision**

   ```c
   #include <time.h>

   size_t strftime(char *restrict s, size_t max, 
                   const char *restrict format, 
                   const struct tm *restrict tm);
   ```

10. use `strptime(3)` to convert `printable form` to `broken-down time`

    > reverse of `strftime(3)`

    ```c
    #define _XOPEN_SOURCE       /* See feature_test_macros(7) */
    #include <time.h>

    char *strptime(const char *restrict s, 
                   const char *restrict format,
                   struct tm *restrict tm);
    ```

    The glibc implementation of `strptime(3)` will not modify those fields of the
    `tm` structure that are not initialized by specifiers in `format`. This is not
    portable, so we should ensure that all the fields of `tm` are initialzed validly
    in portable programs.

    `tm_isdst` field is not set.

11. time-related syscalls and library functions

    ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-09-04%2015-23-03.png)


12. Timezone

    Timezone info can be volatile so that rather than hanling it directly in program, 
    the system maintains this information in files in standard formats.

    And the files reside under `/usr/share/zoneinfo`, each file in this directory
    contains information about the timezone reginme in a particular country or
    region.

    > `/usr/share/zoneinfo` can be changed to another directory via `$TZDIR`, this
    > is a GNU extension.

    ```
    $ l /usr/share/zoneinfo|head -5
    Permissions Size User Group Date Modified Name
    drwxr-xr-x@    - root root  29 Aug 07:42  Africa
    drwxr-xr-x@    - root root  29 Aug 07:42  America
    drwxr-xr-x@    - root root  29 Aug 07:42  Antarctica
    drwxr-xr-x@    - root root  29 Aug 07:42  Arctic
    ```

    The format of above file can be found at `man 5 tcfile`, and these files are
    built using `zic(8)`, the timezone compiler. `zdump(8)` command can be used
    to display the current time corresponding to a specific timezone file:

    ```shell
    $ zdump Cuba
    Cuba  Sat Sep  3 23:01:09 2022 CDT
    ```

    Local timezone is stored in file `/etc/localtime`, which is a softlink to
    the timezone file under `/usr/share/zoninfo`:

    ```shell
    $ l /etc/localtime
    Permissions Size User Group Date Modified Name
    lrwxrwxrwx@   35 root root  25 Aug 06:05  /etc/localtime -> ../usr/share/zoneinfo/Asia/Shanghai
    ```

13. obtain timezone info in a program

    ```c
    #include <time.h>

    void tcset(void)
    ``` 

    This function will set 3 global variables:

    ```c
    extern char *tzname[2];
    extern long timezone;
    extern int daylight;
    ```

    The mechanism behind this function: it first query `$TZ` environment variable,
    if this is not set, then it uses `/etc/timezone`.

    If `$TZ` is empty (set but empty) or it's format is not valid, then `UTC` will
    be used.

    Those time-related functions (e.g. ctime(), localtime()) will call `tzset`
    automatically. **So you don't need to do this manually.**

14. change timezone for program - change $TZ

    > Normally, you don't need to modify this variable unless you are operating
    > a remove machine whose timezone differs from your localhost.

    ```shell
    $ date
    Sun Sep  4 11:28:00 AM CST 2022
    $ TZ=":Cuba" date
    Sat Sep  3 11:28:13 PM CDT 2022
    ``` 

    These are two ways to change `$TZ` in SUSv3, first is demonstrated above, a colon 
    following the relative path (from /usr/share/zoneinfo) of the target timezone file.

    The second way is rather cumbersome. You can refer to 
    [this](https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html)

15. locale: conventions on how to express things

    > For example, in US, date is expressed as `MM/DD/YY`. While in China, we
    > use `YY/MM/DD`.

    Ideally, all programs should respect this.

    As same as timezone, locale settings is volatile, which should be maintained
    by the OS. And it is.

    The setting files reside under `/usr/share/locale` or `/usr/lib/locale` (distro
    dependent)

    ```shell
    $ l /usr/share/locale | head -5
    Permissions Size User Group Date Modified Name
    drwxr-xr-x@    - root root   9 Aug 21:57  aa
    drwxr-xr-x@    - root root   9 Aug 21:57  ab
    drwxr-xr-x@    - root root   9 Aug 21:57  ace
    drwxr-xr-x@    - root root   9 Aug 21:57  ach
    ```

    Each directory under `/usr/share/locale` is named in such convention:

    ```
    language[_territory[.codeset]][@modifier]
    ```

    * language: a two-letter ISO language code
    * territory: two-letter ISO country code
    * codeset: character-encoding set
    * modifier: used to distinguish multiple locales whose `language/territory/
      codeset` are all same

    ```shell
    drwxr-xr-x@    - root root  25 Aug 14:27  zh_CN
    drwxr-xr-x@    - root root   9 Aug 21:57  zh_CN.GB2312
    drwxr-xr-x@    - root root  25 Aug 14:54  zh_CN.UTF-8
    drwxr-xr-x@    - root root   5 May 05:27  zh_Hans
    drwxr-xr-x@    - root root   5 May 05:27  zh_Hant
    drwxr-xr-x@    - root root   9 Aug 21:57  zh_HK
    drwxr-xr-x@    - root root   9 Aug 21:57  zh_TW
    drwxr-xr-x@    - root root   9 Aug 21:57  zh_TW.Big5
    drwxr-xr-x@    - root root  25 Aug 14:54  zh_TW.UTF-8
    ```

16. list current lcoale settings
    
    ```shell
    $ locale
    LANG=en_US.UTF-8
    LC_CTYPE="en_US.UTF-8"
    LC_NUMERIC="en_US.UTF-8"
    LC_TIME="en_US.UTF-8"
    LC_COLLATE="en_US.UTF-8"
    LC_MONETARY="en_US.UTF-8"
    LC_MESSAGES="en_US.UTF-8"
    LC_PAPER="en_US.UTF-8"
    LC_NAME="en_US.UTF-8"
    LC_ADDRESS="en_US.UTF-8"
    LC_TELEPHONE="en_US.UTF-8"
    LC_MEASUREMENT="en_US.UTF-8"
    LC_IDENTIFICATION="en_US.UTF-8"
    LC_ALL=
    ```

    `locale -a` can list all the locales supported by current system.

    `LANG` and `LC_ALL` both means `all`, if one of them is set to a value,
    then all variables will have that value. But they have different priorities.

    priority of those locale-related environmen variables
    
    ```
    LC_ALL > LC_* > LANG
    ```

    If a variable with higher priority is set, the the less priviledged
    ones will be shadowed. We use take `LANG` as the default setting,
    if none of `LC_*` is set, then they all have the value of `LANG`.

17. set or get the locale of current process

    ```c
    #include <locale.h>

    char *setlocale(int category, const char *locale);
    ```

    ```
    Category            Governs
    LC_ALL              All of the locale
    LC_ADDRESS          Formatting of addresses and geography-related items (*)
    LC_COLLATE          String collation
    LC_CTYPE            Character classification
    LC_IDENTIFICATION   Metadata describing the locale (*)
    LC_MEASUREMENT      Settings related to measurements (metric versus US customary) (*)
    LC_MESSAGES         Localizable natural-language messages
    LC_MONETARY         Formatting of monetary values
    LC_NAME             Formatting of salutations for persons (*)
    LC_NUMERIC          Formatting of nonmonetary numeric values
    LC_PAPER            Settings related to the standard paper size (*)
    LC_TELEPHONE        Formats to be used with telephone services (*)
    LC_TIME             Formatting of date and time values
    ```
    
    If `locale` is a empty string (""), then the locale is set according to the values
    of the current environment variables. The process will know nothing about
    these environement variables unless we manually do this.

    If `locale` is `NULL`, then the locale is just queried, not modified.
 
    When we say specify or set a locale, we are talking about setting locale
    to a subdir of `/usr/share/locale`. If the new locale value does not match
    any item in `/usr/share/locale`, then the new locale will be stripped in
    the following orders to get a match:

    1. codeset
    2. normalized codeset
    3. territory
    4. modifier

    If no one matches after all these 4 strippings, `setlocale()` will report an
    error.

    We can test this:

    ```c
    #include <locale.h>
    #include <stdio.h>
    
    int main(void)
    {
            char *old_locale = setlocale(LC_ALL, NULL);
            printf("old_locale: \"%s\"\n", old_locale);
    
            char *new_locale = "wc_CN.utf-8";
            char *res = setlocale(LC_ALL, new_locale);
            if (res == NULL) {
                    printf("\"%s\" is not a valid locale\n", new_locale);
            }
            return 0;
    }
    ```

    ```shell
    $ gccs main.c && ./a.out
    old_locale: "C"
    "wc_CN.utf-8" is not a valid locale
    ```

    `setlocale(LC_ALL, "C")` will be called at the startup of a program as you 
    can see from the above code snippet.

18. adjust the system clock

    > Normally, one should not directly modify the system clock. Network Time
    > Protocal daemon is responsible for this.

    ```c
    (2)
    #include <sys/time.h>

    int settimeofday(const struct timeval *tv, const struct timezone *tz);
    ```

    One can specify the new time value in `tv`, `tz` should be set to NULL as 
    it is obsolete. And whether the microsecond field is supported is hardware-
    dependent. (came from BSD)

    ```c
    (2) Linux-specific
    #include <time.h>

    int stime(const time_t *t);
    ```

    Set new time with the precison of only 1 second.
    

    ```c
    (3)
    #include <sys/time.h>

    int adjtime(const struct timeval *delta, struct timeval *olddelta);
    ```

    This is the preferred way. One can set the amount the time to be adjusted
    in `delta`. After the call, if `delta` is positive, then a small amount of
    time is added each second until `delta` is all added. If `delta` is negative,
    a converse work will be done.

    Since the `delta` is gradually added, these is possibility that only partial
    time is adjusted at the time of the call. If so, The remaining time amount 
    will be stored in `olddelta`.

    ```c
    struct timeval delta;
    delta.tv_sec = 1000;
    delte.tv_usec = 0;
    adjtime(&delta, NULL);

    // the remaining part of that 1000 secs will be stored in `olddelta`.
    struct timeval olddelta;
    adjtime(NULL, &olddelta);
    ```

    If one don't case about that remaining time, `olddelta` can be set to `NULL`.
    If one just wanna query the remaining time instead of adjusting it, one can
    use `adjtime(NULL, &buf)`.

19. software clock (jiffy)
    
    > man 7 time

    The accuracy of various time-out syscalls (select(2), sigtimedwait(2)) is 
    limited to the resolution of software clock, which meansures time in 
    `jiffies`, which is defined by the kernel constant `HZ`. And `HZ` is a 
    configurable.


20. process time: the CPU time consumed by a process since it is created

    And process time can be divided into two categories:

    1. *User* CPU time: time spent in user mode
    2. *System* CPU time: time spent in kernel mode

    ```shell
    $ /usr/bin/time -p ./my_program
    real 33.48 # longer than process time(`user` + `sys`)
    user 218.16
    sys 13.00
    ```

    Diff between real time and process time:

    > The actual definiton of `real time` is just the duration since some fixed
    > point. When that fixed point is set to `EPOCH`, we call this `real time`
    > `calendar time`. If we say `real time` of a process, we are talking about
    > its lifetime.

    Linux is a time-sharing OS, which means each process is executed in the unit
    called time slice. During a period of time, multiple processes will be executed.
    So the real time is the duration between start and end (or lifetime) of that
    process. While the process time is a small period in real time when this 
    process owns the CPU.

    For example, our time slice is 1 sec, and we have 5 seconds to exectute processes
    A and B. Let's assume both processes are finished after 5 seconds and the
    execution order is

    ```
    A - start of A
    B - start of B
    A
    B - end of B
    A - end of A
    ```
    The real time of A is 5 seconds and process time is 3 seconds.

21. syscalls used to obtain process time

    1. times(2)

       ```c
       #include <sys/times.h>
   
       clock_t times(struct tms *buf);
       ```
   
       ```c
       struct tms {
           clock_t tms_utime;  /* user CPU time */
           clock_t tms_stime;  /* system CPU time */
           clock_t tms_cutime; /* user CPU time of all (waited for) children */
           clock_t tms_cstime; /* system CPU time of all (waited for) children */
       };
       ```

       This fcuntion stores the process time consumed so far in `buf`, and return
       an elapsed time since an arbitrary point (the standard deliberately
       does not specify what this point is).

       If you wanna use the return value, since you don't know what that arbitrary
       point is, you should call this twice and calculate the time difference
       between them.

       The `clock_t` type in this syscall is an integer type (i64 under Linux amd64)
       in the unit called `clock ticks`. We can use `sysconf(_SC_CLK_TCK)` to 
       obtain the number of `clock ticks` per second to convert the value to the
       number of seconds.

       And the return value of this function is **unreliable** as it can overflow
       the range of `clock_t`. A reliable way is using `gettimeofday(2)`.

       `buf` can be set to `NULL` under Linux, this is not permitted in other UNIX
       implementations.
  
    2. clock(3)
       
       ```c
       #include <time.h>

       clock_t clock(void);
       ```

       > Even though both these functions use `clock_t`, but they are different!
       >
       > WTF!!!
       >
       > This is the result of historically conflicting definitions of `clock_t`.

       This function returns the elapsed process time in units of `CLOCK_PER_SEC`.
       If you wanna turn this value into the number of seconds, use 
       `clock()/CLOCK_PER_SEC`.

       > The accuracy of function is limited by `software clock`, see Note 19.

       Under Linux, the return value of `clock(3)` does not include the time
       consumed by child processes. While on some UNIX implementations, they
       did this.
