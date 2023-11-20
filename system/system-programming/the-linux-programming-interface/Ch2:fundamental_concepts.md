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
# 2.6 Programs
# 2.7 Processes

1. 进程的RUID，EUID以及saved-set-UID(以及RGIP, EGID, saved-set-GID)
   
   > For more info about process credentials, see
   > [Ch9](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch9.md)

   > 判断进程有没有权限做一个件事，是看其EUID和EGID的

   每一个进程都有一个RUID和RGID，继承自父进程 (login shell 从/etc/passwd中读登陆
   用户的uid和gid并设置为RUID和RGID)。EUID和EGID默认等于RUID和RGID，但
   是有时普通用户需要更高权限，此时可以更改EUID来获取更高权限。通过:
   1. [seteuid和setegid](https://man7.org/linux/man-pages/man2/seteuid.2.html)
   2. set-UID 和set-GID机制

   比如设置了set-UID位的可执行程序，在运行时，其EUID会被设置为可执行程序文件的
   UID而不是父进程的UID。(see Ch9 3/4)


   For info about `saved-set-UID`, see Ch9: 5.


2. init进程

    内核在启动时，会创建一个名为`init`的特殊进程，为所有进程的父进程
    其程序文件`/sbin/init`，其PID通常是1

    ```shell
    $ cd sbin
    $ ls -l |grep init
    lrwxrwxrwx     20 root root   19 Apr 04:12  init -> /lib/systemd/systemd
    ```

3. 子进程会默认继承父进程的环境变量

    当使用`exec()`对子进程进行替换时，可以使用参数选择换掉或者不换掉环境变量

4. 资源限制

    每个进程都会消耗资源，可以使用`setrlimit`函数来为进程可消耗资源设置上限。
    限制包括2项，soft limit以及hard limit，soft limit是用来设置进程消耗资源
    的上限的，而hard limit则是设置soft limit的上限的。非特选进程(EUID!=0)的
    soft limit可以设置为0到hard limit的任何值，但是其hard limit只能调低，不能
    调高。

    由`fork`创建的子进程，会继承其父进程对资源消耗的限制

    > shell资源消耗的限制可以使用`ulimit`命令进行调整

# 2.8 Memory Mappings
# 2.9 Static and Shared Libraries
# 2.10 Interprocess Communication and Synchronization
# 2.11 Signals
# 2.12 Threads
# 2.13 Process Groups and Shell Job Control
# 2.14 Sessions, Controlling Terminals, and Controlling Processes
# 2.15 Pseudoterminals
# 2.16 Date and Time
# 2.17 Client-Server Architecture
# 2.18 Realtime
# 2.19 The `/proc` file system
# 2.20 Summary


7. 文件的换行符及结束符

   * 换行符是`\n(ascii: 10)`
   * UNIX没有文件结束符的概念，读取文件时如无数据返回，则认定达到EOF
   > 在non-blocking的IO时，如果没有数据返回，则函数返回EOF

12. object library

    UNIX中有2中Object library

    1. static library(aka archives)

       起初是UNIX上唯一的library类型。在链接时进行调用，linker会将static library中
       的函数拷贝到最终生成的二进制文件中去。这样链接而成的程序称其是statically linked

       由于静态链接会对所需要的东西进行拷贝，所以其有以下2个缺点:
       
       1. 浪费磁盘和内存
       2. 如果static library被修改了，为了让最终生成的链接文件的被链接部分也被修改
       ，需要重新链接

    2. shared library

       shared libraryd就是为了解决static library的问题才出现的。首先说一下
       static library 为什么static，因为其是在编译时进行拷贝(从static library
       拷贝到最终的可执行文件，想一下rust的编译期的静态检查)。而shared library
       则是在运行时(runtime)才将所需要用到的函数加载进内存，而且倘若多个程序都
       链接了同一个shared library，那么只有一份 library 会被加载进入内存，供多
       个程序使用(节省内存大小)。在链接得到的最终binary里有的也只是所需被链接
       的函数的一个record而不是函数本身(节省磁盘)。由于是在运行时才加载入内存，
       更新的问题也就自动解决了

13. UNIX有众多的IPC方式，其中有一些的功能是重叠的。这是由于他们来源于不同的UNIX。
    比如，FIFO(named pipe)和UNIX domain socket是在相同主机不相关进程间进行通讯的
    方式，FIFO来源于system V，而socket来自BSD
    
14. signal的pending状态

    一个信号，在其generate到deliver中间的时间，我们称此signal处于pending状态。
    正常情况下，一个pending signal会立刻被deliver给进程，只要进程占据了CPU。然
    而如果一个signal被加入了signal mask，那么此signal一旦被产生就会一直处于
    pending状态，直到其被从signal mask中移除

15. thread的内存共享

    同一个process中的threads是共享heap和data area的，但是每一个thread都有自己独
    立的stack


16. process group and shell job control

    除了sh，其他的shell都有一个特性叫做shell job control。这一特性允许用户同时
    执行多个shell命令，并且这些被创建出来的众多命令子进程处于一个process group
    或者叫作job之中。每一个process group都有一个process group id，是其中的某一
    个进程(称此进程为process group leader)的PID

    > 目前还没感觉出这一特性的作用

17. session

    一个session是一组process group(job)，每一个session都有一个session id，是创建
    此session的process的PID

    > TLPI section 2.14 俺没读懂

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

19. real time operating system

    Linux本是一个分时系统，但其在不断的改进中，已经完全支持real time 
