#### Ch12: System and process information

> 1. Mainly focuses on the `/proc` virtual file system. 
>    1. `/proc/PID` dir for process info
>    2. `/proc/sys` `/proc/net` and `/proc/sysvipc` for system info
> 2. introduces `uname(2)`
<!-- > 3. syscall for retrieving system information. -->


1. the `/proc` virtual file system
   
   This file system is virtual because it does not reside on the disk, instead,
   it is automatically created when a process accesses them.

2. `/proc/PID` for processes information
   
   For each process on the system, the kernel creates a directory named the 
   process ID for it (/proc/PID).

   These are multiple files and directories under this process dir, containing 
   various information about this process.

   * cmdline: Command-line arguments delimited by `\0`
   * cwd: Symbolic link to current working directory
   * environ: Environment list NAME=value pairs, delimited by `\0`
   * exe: Symbolic link to the binary being executed
   * fd: A directory containing symbolic links to files opened by this process
   * maps: Memory mapping
   * mem: Process virutal memory(must lseek() to valid offset before I/O)
   * mounts: Mount points for this process
   * root: Symbolic link to the root directory (?)
   * status: Various information (e.g., process IDs, credentials(UID/GID), 
     memory usage, signals)
   * task: Contains one subdirectory for each thread in process

3. `/proc/PID/fd`

   This dir contains symbolic links to files opened by process `PID`. Each
   of these soft link has a name that matches the file descriptor.

   ```shell
   $ cd /proc/self/fd
   $ l
   Permissions Size User  Group Date Modified Name
   lrwx------@   64 steve steve  8 Sep 06:18  0 -> /dev/pts/8
   lrwx------@   64 steve steve  8 Sep 07:26  1 -> /dev/pts/8
   lrwx------@   64 steve steve  8 Sep 07:26  2 -> /dev/pts/8
   lrwx------@   65 steve steve  8 Sep 07:26  10 -> /dev/pts/8
   ```

   > [Ch5.md Note 16 `/dev/fd`](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch5.md)

4. `/proc/PID/task` dir 

   This directory contains one subdirectory for each thread, named `TID`

   > For a single-threaded program, the TID will have the same value as PID.
   > For a multi-threaded program, each thread will have a unique thread ID.

   ```shell
   $ cd /proc/self/task
   $ l
   Permissions Size User  Group Date Modified Name
   dr-xr-xr-x@    - steve steve  8 Sep 07:26  5058
   ```

   The `TID` number is same as the return value of `gettid(2)`:

   ```c
   #define _GNU_SOURCE
   #include <unistd.h>

   pid_t gettid(void);
   ```

   Under `/proc/PID/task/TID` is a set of files and directories exactly like
   those that those under `/proc/PID`.
   
   ```shell
   $ cd /proc/self/task/5058
   $ ls
   arch_status  children    cpu_resctrl_groups  exe      io                 loginuid   mounts     oom_adj        patch_state  sched      smaps         statm    wchan
   attr         clear_refs  cpuset              fd       ksm_merging_pages  maps       net        oom_score      personality  schedstat  smaps_rollup  status
   auxv         cmdline     cwd                 fdinfo   latency            mem        ns         oom_score_adj  projid_map   sessionid  stack         syscall
   cgroup       comm        environ             gid_map  limits             mountinfo  numa_maps  pagemap        root         setgroups  stat          uid_map
   ```

   Since threads **share** many resources, many file in this directory is the same for
   each thread of this process.


5. retrieve system information from `/proc`

   Various files and directories under `/proc` can be used to obtain the system
   information:

   * `/proc`: various system information
   * `/proc/net`: status information about networking and sockets
   * `/proc/sys/fs`: settings related to file system
   * `/proc/sys/kernel`: various general kernel settings
   * `/proc/sys/net`: networking and sockets settings
   * `/proc/sys/vm`: memory-management settings
   * `/proc/sysvipc`: information about system V IPC objects

6. `/proc` hierarchy
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-09-08_11-45-36.jpg)

7. `uname(2)`
   

   ```c
   #include <sys/utsname.h>

   int uname(struct utsname *buf);
   ```

   ```c
   struct utsname {
       char sysname[];    /* Operating system name (e.g., "Linux") */
       char nodename[];   /* Name within "some implementation-defined
                             network" */
       char release[];    /* Operating system release
                             (e.g., "2.6.28") */
       char version[];    /* Operating system version */
       char machine[];    /* Hardware identifier */
   #ifdef _GNU_SOURCE
       char domainname[]; /* NIS or YP domain name */
   #endif
   };
   ```

   The standard (SUSv3) make the length of these fields undefined. On Linux, 
   these fields are `65` (defined by constant `_UTSNAME_LENGTH`) bytes long 
   (including the terminating NUL).

   Some of these fields are also available through `/proc/sys/kernel`:
   * sysname: /proc/sys/kernel/ostype
   * release: /proc/sys/kernel/osrelease
   * version: /proc/sys/kernel/version


   ```c
   #include <stdio.h>
   #include <sys/utsname.h>
   
   int main(void)
   {
           struct utsname buf;
           if (-1 == uname(&buf)){
                   perror("uname(2)");
           }
   
           printf("sysname: %s\n", buf.sysname);
           printf("nodename (is hostname on Linux): %s\n", buf.nodename);
           printf("release: %s\n", buf.release);
           printf("version: %s\n", buf.version);
           printf("machine: %s\n", buf.machine);
           return 0;
   }
   ```

   ```shell
   $ gccs main.c && ./a.out
   sysname: Linux
   nodename (hostname): fedora
   release: 5.19.7-200.fc36.x86_64
   version: #1 SMP PREEMPT_DYNAMIC Mon Sep 5 14:50:12 UTC 2022
   machine: x86_64
   ```

   > This syscall is the mechanism behind `uname(1)`

   The `uts` in this syscall means `UNIX Time-Sharing System`
