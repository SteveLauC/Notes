#### Ch2: fundamental concepts

> * 2.1 The Core Operating System: The Kernel
> * 2.2 The Shell
> * 2.3 Users and Groups
> * 2.4 Single Directory Hierarchy, Directories, Links, and Files
> * 2.5 File I/O Model
> * 2.6 Programs
> * 2.7 Processes
> * 2.8 Memory Mappings
> * 2.9 Static and Shared Libraries
> * 2.10 Interprocess Communication and Synchronization
> * 2.11 Signals
> * 2.12 Threads
> * 2.13 Process Groups and Shell Job Control
> * 2.14 Sessions, Controlling Terminals, and Controlling Processes
> * 2.15 Pseudoterminals
> * 2.16 Date and Time
> * 2.17 Client-Server Architecture
> * 2.18 Realtime
> * 2.19 The `/proc` file system
> * 2.20 Summary


# 2.1 The Core Operating System: The Kernel


1. The Linux kernel executable is typically located at `/boot/vmlinuz`

   > Why it is called `vmlinuz`:
   >
   > On early UNIX implementations, the kernel was called `unix`, after implementing
   > virtual memory, this kernel was renamed to `vmunix`. So Linux kernel should be
   > named as `vmlinux` if we follow this mode, but since the kernel in Linux is 
   > compressed, we replace the last chapter `x` with `z`, so `vmlinuz`.

    ```shell
    $ l /boot | rg vmlinuz
    .rwxr-xr-x@     1  14M root root   1 Jan  2022  vmlinuz-0-rescue-8b475d2697dc4af8928c86a74a6d4825
    .rwxr-xr-x@     1  15M root root  25 Oct 08:00  vmlinuz-6.5.9-300.fc39.x86_64
    .rwxr-xr-x@     1  15M root root   2 Nov 08:00  vmlinuz-6.5.10-300.fc39.x86_64
    .rwxr-xr-x@     1  15M root root   8 Nov 08:00  vmlinuz-6.5.11-300.fc39.x86_64
    ```

2. Kernel mode and User mode

   On x86, there are 4 modes:

   1. 0 (kernel/supervisor mode)
   2. 1
   3. 2
   4. 3 (user mode)

   > For more information, see 
   > [Protection ring](https://en.wikipedia.org/wiki/Protection_ring)

   Linux, only uses the 0 and 3 modes, i.e., kernel mode and user mode.

   Areas of virtual memory will be marked if they are in kernel mode or user 
   mode, IIRC, with 32-bit Linux, 4 GiB of memory, the top 1 GiB is for the
   kernel

3. A process can do nothing without the assistance of kernel

   We say:

   * A process can access files
   * A process can do IPC
   * A processs can create a pipe
   * ...

   Yes, they can, but it has to be done with the help of the kernel.

   * A process can request the kernel to access files
   * A process can request the kernel to do IPC
   * A processs can request the kernel to create a pipe
   * ...

# 2.2 The Shell


1. What is `login shell`?

   Login shell is typically the first process executed with my UID, on desktop
   Linux, we don't have a login shell, it is replaced with the desktop environment
   process, e.g., GNOME shell.
   
   Well, I found that the first process started with my ID is not the GNOME 
   shell, but a systemd process...
   
   ```sh
   $ ps steve -p | grep systemd
   systemd(2735)-+-(sd-pam)(2741)
   ```
  
   When SSHing into a machine, the shell started is a login shell, but it is also
   not the first process started with me, it is still the `systemd` process:

   > Systemd is everywhere????
  
   ```sh
   $ ssh steve@x.x.x.x
  
   # if $0 starts with a `-`, then it is a login shell
   $ echo $0
   -zsh
   
   # PID of the current shell
   $ echo $$
   5760
   
   $ pstree steve -p 
   sshd(5742)───zsh(5760)───pstree(6760)
   
   systemd(1819)─┬─(sd-pam)(1828)
                   ├─dbus-broker-lau(5291)───dbus-broker(5292)
                   ├─pipewire(2084)─┬─{pipewire}(2093)
                   │                └─{pipewire}(2094)
                   ├─pipewire-pulse(2211)─┬─{pipewire-pulse}(2215)
                   │                      └─{pipewire-pulse}(2218)
                   └─wireplumber(2090)─┬─{wireplumber}(2095)
                                       ├─{wireplumber}(2098)
                                       ├─{wireplumber}(2099)
                                       ├─{wireplumber}(2103)
                                       └─{wireplumber}(2105)
   ```

2. On Linux, the `sh` shell is emulated through the `bash` shell.


# 2.3 Users and Groups

> See also Ch8

1. The GID of a user is the ID of his first group
   
   > For more information about group, see 
   > [Ch8](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch8.md)
    
   ```shell
   $ id
   uid=1000(steve) gid=1000(steve) groups=1000(steve),4(adm),27(sudo),121(lpadmin),999(docker)

   $ bat /etc/passwd | rg steve
   steve:x:1000:1000:steve:/home/steve:/usr/bin/zsh

   $ cat /etc/group | rg steve
   wheel:x:10:steve
   steve:x:1000:
   ```

   > At first, a user can ONLY belong to one group. BSD allowed that a user to 
   > be in multiple groups, this idea was adoped by other UNIX implementations
   > as well, and was finally standardized in the POSIX.1-1990 standard.

2. The format of `/etc/passwd`

   > For more information about `passwd`, see 
   > [Ch8](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch8.md)

    ```
    steve:x:1000:1000:Steve:/home/steve:/usr/bin/zsh
    _flatpak:x:122:130:Flatpak system-wide installation helper,,,:/nonexistent:/usr/sbin/nologin
    ```

    * user name
    * x if the actual password has been encrypted and stored in `/etc/shadow`
    * UID
    * GID
    * comment field, used to supplement some extra comments, for example, with
      flatpak, it is `system-wide installation helper`
    * Home dir
    * Default shell

    > The home dir of flatpak is `nonexistent`，login shell is `/usr/sbin/nologin`

    > [understanding-etcpasswd-file-format](https://www.cyberciti.biz/faq/understanding-etcpasswd-file-format/)

3. The format of `/etc/group`

    ```
    steve:x:1000:
    docker:x:999:steve
    ```

    * Group name
    * x if the password has been encrypted and stored in /etc/shadow
    * Group ID

      > Have never used group password
      >
      > See [Typical use case for a group password](https://unix.stackexchange.com/q/93123/498440)

    * Users that are in this group, comma-separated

4. The user with UID 0 is the superuser, and usually this user is called `root`.

# 2.4 Single Directory Hierarchy, Directories, Links, and Files

> See Ch18

1. Directory is a special kind of file whose contents are a table:

   | filename | inode |
   |----------|-------|
   | .        | 1     |
   | ..       | 2     |
   | foo      | 3     |
   | dir      | 4     |

2. An entry within a directory table is called a link, or hard link, and thus
   for the same file, you can have multiple links, in the same or different
   directories.

   > When the number of hard link gets decreased to 0, the file will be deleted.

   The hard link for a directory is at least 2, for the reason that it is recorded
   in its parent directory and itself (the dot `.`). Users are not allowed to create
   hard links for directory in case of breaking the file system.

   On BTRFS, the hard link of a directory will always be 1.

3. Soft link/symlink

   Most syscall will dereference symlink.

   And dereference is a recursive process, Linux (since kernel 4.2) allows 
   40 recursions at most.
   
4. Filename

   On most Linux file systems, filenames can be up to 2555 characters long. 
   Filenames may contain any characters except `/` and null characters (\0).
   However, it is advisable to employ only latters and digits, and `.`, `_`, 
   and `-` This 65-character set, is referred to in SUSv3 as the portable
   filename character set.

   > SUS is Single UNIX Specification

   And we should avoid filenames starting with a hyphen, cause it maybe treated
   as a command option.

   This restriction is defined by the constant: `PATH_MAX`:

   ```c
   #include <linux/limits.h>
   #include <stdio.h>
   #include <stdlib.h>

   int main(void) {
       printf("%d", PATH_MAX);

       exit(EXIT_SUCCESS);
   }
   ```
   ```sh
   # On BTRFS

   $ gcc main.c && ./a.out
   4096
   ```

5. Pathname
 
   For a pathname, the series of component filenames preceeding the final slash
   is sometimes referred to as the `directory` part of a pathname, while the
   name following the final slash is sometimes referred to as the file or the 
   base part of the pathname.

   See Ch18 for the C APIs:

   ```c
   dirname()
   basename()
   ```

6. For directories, `x` permission allows you to "enter" the directory, and access
   the stuff in it.

   > Here, by "stuff", I mean the files within this directory.

   The `r` and `w` permisssions enable read and write permissions on the directory
   (the map list) itself, not its files. In most cases, without the `x` permission,
   you can do nothing with this directory.

# 2.5 File I/O Model


1. UNIX processes interact with the outside world through file descriptors

   When you send text to the process from your keyboard, it receives it from
   the stdin, fd 0

   * terminal
   * file
   * devices
   * sockets
   * pipe

   > Well, you do have things other than fds, e.g., shard memory.

   What a beautiful abstraction!

2. A process can have limited number of file descriptors, we can get this limit
   at runtime with:

   ```rs
   use nix::sys::resource::{getrlimit, Resource};

   fn main() {
       let n_open_file = Resource::RLIMIT_NOFILE;
       let (cur, max) = getrlimit(n_open_file).unwrap();

       println!("{} {}", cur, max);
   }
   ```

   ```sh
   $ cargo r -q
   1024 524288
   ```

# 2.6 Programs

1. In C, we can access the command line arguments with the `argc` and `argv`
   arguments of the `main` function

   ```c
   int main(int argc, char *argv[])
   ```

   In Rust, we use `std::env::args()`.

2. How Rust implements `std::env::args()` on Linux/Glibc
 

   Glibc exposes all the `argc/argv/envp` arguments to the functions registered
   in the `.init_array` section:

   ```c
   void __libc_csu_init (int argc, char **argv, char **envp) {
     _init ();

     const size_t size = __init_array_end - __init_array_start;
     for (size_t i = 0; i < size; i++) {
         (*__init_array_start [i]) (argc, argv, envp);
     }
   }
   ```

   so in Rust std, we define two global variables to store the `argc` and 
   `argv`, then register a function to that section, where we will update 
   the global variables.

   ```rs
   use nix::libc::c_int;

   static mut ARGC: c_int = 0;

   // By registering, we are not actually register the function itself, we store
   // the function pointer at that section.
   #[link_section = ".init_array"]
   static ARGV_INIT_ARRAY: extern "C" fn(
       c_int,
       *const *const u8,
       *const *const u8,
   ) = {
       extern "C" fn init_wrapper(
           argc: c_int,
           _argv: *const *const u8,
           _envp: *const *const u8,
       ) {
           unsafe {
               ARGC = argc;
           }
       }
       init_wrapper
   };

   fn main() {
       unsafe {
           println!("{}", ARGC);
       }
   }
   ```

   > Functions registered in the `.init_array` section will be invoked sequentially
   > on process startup, in Rust, there is a crate 
   > [`rust-ctor`](https://github.com/mmastrac/rust-ctor/tree/master), which exactly
   > employs this, though the function registered by this crate does not have the
   > `argc/argv/envp` arguments.

3. How Rust implements `std::env::args()` on other libc

   The value of `envp` is stored in global variable `__envrion`, `argv` is directly
   before it, then `argc`.

   https://stackoverflow.com/a/58956461/14092446

4. Why would `std::env::arg_os()` allocate (return an owned `OsString`)

   Becausee it can be modified by other parts of the process, typically parts
   that are not Rust.

   > The problem start with FFI and other languages, which might modify 
   > argc, argv, *argv, and **argv while Rust is iterating. Or you might 
   > get a &[u8] that has been malloc()ed by C code, and free()d when you
   > later try to read it.

# 2.7 Processes

1. Process memory layout

   A process is logically divided into the following parts, known as `segments`:

   * Text: the instrctions of the program 
   * Data: the **static** variable used by the program
   * Heap: dynamic memory
   * Stack: for function frames

   These memory segments are initialized with their corresponding ELF sections,
   one can use the `size` command to inspect a binary's sections.

2. A process can create a child process by using the `fork(2)` syscall, which
   would duplicate itself.

   Then it can call `exec()` to run the needed program.

   > QUES: how does the first process get created? (the init process)
   >
   > See below, the init process.

3. Do we have kernel-space processes on Linux?

   NO, because Linux kernel is a Monolithic kernel, it has threads but not 
   processes. Microkernel, on the other hand, can have kernel-space processes.

   [Does kernel spawn some processes (not user process) that keep running in background?](https://stackoverflow.com/q/72923707/14092446)

4. Process terminaltion

   A process can be terminated in two ways:

   1. It asks for a terminal through the `_exit(2)` sycall or the libc 
      function
   2. Being killed by a signal

   In either cases, the terminated process will send termination status (a 
   nonnegative integer) to the parent process, the parent process can get 
   it through the `wait(2)` syscall.

   In the case of calling `_exit(2)`, the child process will choose this
   integer itself, in the other case, the status will be set according to the
   type of the signal that causes the death of the process.

   Conventionally, a termination status of 0 indicates that the process 
   succeeds, a nonzero value indicates that some error occurred.

  
5. The RUID/EUID/saved-set-UID(RGID/EGID/saved-set-GID and supplementary group IDs)
   of a process

   > For more info about process credentials, see
   > [Ch9](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch9.md)

   Every process will have its RUID and RGID, which are inherited from its parent
   process (the first process gets them from the `/etc/passwd` file). EUID and
   EGID, are set to its RUID and RGID, but can be changed if higher or lower
   priviledge is required:

   1. [seteuid and setegid syscall](https://man7.org/linux/man-pages/man2/seteuid.2.html)
   2. set-UID and set-GID bits

   For example, a program (compiled binary) with set-UID set, will have EUID that
   is equal to the user ID of the owner of that program.

   ```sh
   $ l $(which passwd)
   Permissions Links Size User Group Date Modified Name
   .rwsr-xr-x      1  68k root root  23 Mar 20:40  /usr/bin/passwd
   ```

6. Priviledged process

   Traditionally in UNIX world, a priviledged process is a process whose EUID
   is 0. 

   Linux has a better priviledge control as it has capabilities, a process can 
   perform an operation only if it has the corresponding capability. The super
   user (UID 0), has all the capabilities enabled.

7. The init process

    When kernel starts，it will create a special process named `init`, all the
    process will be created by either the `init` process or one of its 
    descendants.

    The `init` process usually has PID 1.

    ```sh
    $ pstree -p | head -1
    systemd(1)-+-ModemManager(1022)-+-{ModemManager}(1062)
    ```

    ```shell
    $ cd sbin
    $ ls -l |grep init
    lrwxrwxrwx     20 root root   19 Apr 04:12  init -> /lib/systemd/systemd
    ```

8. Daemon processes

   A daemon process is just a normal process, except that:

   1. It is long-lived, usually started at system boot
   2. It runs in the background with no terminal attached

   Name of a daemon process would usually end with `d`, like `syslogd`, `httpd`.

9. Resource limits

   A process would have limited resources, these limits can be queried with the
   `getrlimit(2)` syscall:

   ```rs
   use nix::sys::resource::{getrlimit, Resource};

   fn main() {
       let n_fd = Resource::RLIMIT_NOFILE;

       let (soft, hard) = getrlimit(n_fd).unwrap();

       println!("Soft: {} Hard: {}", soft, hard);
   }
   ```

   ```sh
   $ cargo r -q
   Soft: 1024 Hard: 1048576
   ```

   This syscall would return 2 values, one is the `soft limit`, the current
   limination of the corresponding resource, the other one is the `hard limit`,
   which is the limit of the `soft limit`.

   One can adjust these limits through `setrlimit(2)`, the soft limit can be 
   set to `[0, hardlimit]`, the hard limit can ONLY be decreased.

   > Modifications to a hard limit is a one way operation.

   Child process will inherient these resource limits form its parent process.

   > Within a shell, we can query them with the `ulimit` command.

# 2.8 Memory Mappings

1. The `mmap(2)` syscall can be used to create a new memory mapping in a process's
   virutal memory, mappings fall into 2 categories:

   1. File mapping

      A file mapping maps a region of a file into the calling proess's virutal 
      memory, once mapped, the file's contents can be accessed by operations on
      the bytes in the corresponding memory region, the pages of the mapping are
      automatically loaded from the file when accessed (required).
   
   2. Anonymous mapping

      Anonymous mapping does not have a corresponding file, instead, the pages
      are initialized to 0, and thus, anonymous can be used to implement 
      `maclloc(3)`

   The memory in one process's mapping may be shardd with mappings in otehr processes,
   this can occur either because two pocesses map the same regiion of a file or
   because a child created by `fork()` inherits a mapping from its parent.

   When sharing with other processes, modification made by one process can be seen
   by other processes and will be carried to the underlying file if the flag argument
   contains `MAP_SHARED`.

   Memory mapping can serve a variety of purposes, including:

   1. Create a process's text segment from the corresponding ELF file
   2. Allocating zero-initialized memory
   3. Memory-mapped I/O
   4. IPC


# 2.9 Static and Shared Libraries

1. Object library

   Object library is a file containing the compiled object code for a set of 
   functions that could be called from application programs (using C's ABI).

   There are 2 kinds of object library in the UNIX world:

   1. static library

      > This was the ONLY type of library on early UNIX systems. 

      To use a static library, we specify the that library in the link command 
      used to build a program.

      Here an example demostrating on how to make a C program a static library
      and use it:

      ```c
      // hello.c

      #include <stdio.h>

      void hello(void) {
          printf("Hello World\n");
      }

      // hello.h
      void hello(void);

      // main.c
      #include "hello.h"
      int main(void) {
          hello();
      }
      ```

      ```sh
      $ ls
      hello.c  hello.h  main.c

      # Compile `hello.c`, use `-c` to tell gcc to build the an intermediate 
      # object
      $ gcc -c hello.c

      $ l
      Permissions Links Size User  Group Date Modified Name
      .rw-r--r--      1   70 steve steve  1 Dec 16:45  hello.c
      .rw-r--r--      1   18 steve steve  1 Dec 16:41  hello.h
      .rw-r--r--      1 1.4k steve steve  1 Dec 16:46  hello.o
      .rw-r--r--@     1   52 steve steve  1 Dec 16:40  main.c

      # Extract `libhello.a` from `hello.o`
      $ ar rcs libhello.a hello.o

      $ l
      Permissions Links Size User  Group Date Modified Name
      .rw-r--r--      1   70 steve steve  1 Dec 16:45  hello.c
      .rw-r--r--      1   18 steve steve  1 Dec 16:41  hello.h
      .rw-r--r--      1 1.4k steve steve  1 Dec 16:46  hello.o
      .rw-r--r--      1 1.5k steve steve  1 Dec 16:46  libhello.a
      .rw-r--r--@     1   52 steve steve  1 Dec 16:40  main.c

      # Compile main.c with the `-c` option
      $ gcc -c main.c
      $ ls
      hello.c hello.h hello.o libhello.a main.c main.o

      # Let's gcc build the final binary
      # -L option: path of the library search directory
      # Since libhello.a is in the pwd, we pass a dot here
      $ gcc main.o -L . -lhello -o hello

      $ ./hello
      Hello World
      ```

      When linking, the linker will extract the object file from the static 
      library to the final binary file. A program linked in such way is called
      a statically linked program.

      Given that we have copied the object file while linking, static library
      has 2 obvious drawbacks:
       
      1. Heavy disk and ram usage
      2. If a linked library has been changed, we have to relink it again.

   2. shared library

      > QUES: I am not sure how to dynamically link with musl in C, from their
      >       doc, I need to install the gcc-musl wrapper, then everything would
      >       work out of box.

2. Why is statically linking against glibc strongly discouraged?

   [Why is statically linking glibc discouraged?](https://stackoverflow.com/q/57476533/14092446)

# 2.10 Interprocess Communication and Synchronization


13. UNIX有众多的IPC方式，其中有一些的功能是重叠的。这是由于他们来源于不同的UNIX。
    比如，FIFO(named pipe)和UNIX domain socket是在相同主机不相关进程间进行通讯的
    方式，FIFO来源于system V，而socket来自BSD

    
# 2.11 Signals


14. signal的pending状态

    一个信号，在其generate到deliver中间的时间，我们称此signal处于pending状态。
    正常情况下，一个pending signal会立刻被deliver给进程，只要进程占据了CPU。然
    而如果一个signal被加入了signal mask，那么此signal一旦被产生就会一直处于
    pending状态，直到其被从signal mask中移除

# 2.12 Threads

15. thread的内存共享

    同一个process中的threads是共享heap和data area的，但是每一个thread都有自己独
    立的stack

# 2.13 Process Groups and Shell Job Control

16. process group and shell job control

    除了sh，其他的shell都有一个特性叫做shell job control。这一特性允许用户同时
    执行多个shell命令，并且这些被创建出来的众多命令子进程处于一个process group
    或者叫作job之中。每一个process group都有一个process group id，是其中的某一
    个进程(称此进程为process group leader)的PID

    > QUES: 目前还没感觉出这一特性的作用
    >
    > Future Steve(23/12/1): this is specifically for shell, it totally makes
    > sense that you didn't realized what it is for. More details on
    > [Use and meaning of session and process group in Unix?](https://stackoverflow.com/q/6548823/14092446)

# 2.14 Sessions, Controlling Terminals, and Controlling Processes


17. session

    一个session是一组process group(job)，每一个session都有一个session id，是创建
    此session的process的PID

    > TLPI section 2.14 俺没读懂

# 2.15 Pseudoterminals

# 2.16 Date and Time

18. 进程占用的时间

    * real time: 从进程被创建到进程结束真正占用了多少时间
    * process time(cpu time): 指进程真正占用CPU的时间
        * system CPU time: 进程在kernel mode下执行所花时间
        * user CPU time: 进程在user mode下所花的时间

    > GNU的`time`命令有一个`-p`选项，可以使用这种时间的格式
    ```shell
    -p, --portability
           Use the following format string, for conformance with POSIX standard 1003.2:
                     real %e
                     user %U
                     sys %S
    ```

    需要注意的是，`bash`以及`zsh`都有一个内置命令`time`(不支持`-p`选项)，如果想
    使用GNU的，可以
    ```shell
    $ command time -p ls
    ```
# 2.17 Client-Server Architecture
# 2.18 Realtime


19. real time operating system

    Linux本是一个分时系统，但其在不断的改进中，已经完全支持real time 

# 2.19 The `/proc` file system
# 2.20 Summary


7. 文件的换行符及结束符

   * 换行符是`\n(ascii: 10)`
   * UNIX没有文件结束符的概念，读取文件时如无数据返回，则认定达到EOF
   > 在non-blocking的IO时，如果没有数据返回，则函数返回EOF





