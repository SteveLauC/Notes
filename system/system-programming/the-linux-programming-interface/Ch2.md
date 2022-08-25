1. Linux的内核可执行文件采用`/boot/vmlinuz`或与之类似的文件名.

   > 在早期的UNIX中，内核可执行文件被称为`vmunix`，而在Linux中，将`unix`换为
   `linux`并将`x`改为`z`，表示其是已经被压缩的可执行文件

    ```shell
    $ cd /boot
    $ ls -l
    Permissions Size User Group Date Modified Name
    .rw-r--r--  264k root root  17 Jun 22:39  config-5.17.5-76051705-generic
    drwx------     - root root   1 Jan  1970  efi
    lrwxrwxrwx    34 root root  10 May 01:08  initrd.img -> initrd.img-5.17.5-76051705-generic
    .rw-r--r--  116M root root  23 Jun 16:27  initrd.img-5.17.5-76051705-generic
    lrwxrwxrwx    34 root root  10 May 01:08  initrd.img.old -> initrd.img-5.17.5-76051705-generic
    .rw-------  6.3M root root  17 Jun 22:39  System.map-5.17.5-76051705-generic
    lrwxrwxrwx    31 root root  10 May 01:08  vmlinuz -> vmlinuz-5.17.5-76051705-generic
    .rw-------   11M root root  17 Jun 22:39  vmlinuz-5.17.5-76051705-generic
    lrwxrwxrwx    31 root root  10 May 01:08  vmlinuz.old -> vmlinuz-5.17.5-76051705-generic
    ```

2. 在Linux上，`sh`是用`bash`仿真实现的

3. 用户的组ID是用户第一个组的组ID
   
   > For more information about group, see [Ch8](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch8.md)
    
    ```shell
    $ id
    uid=1000(steve) gid=1000(steve) groups=1000(steve),4(adm),27(sudo),121(lpadmin),999(docker)
    # 我在很多个组里面，但我的组ID是唯一的
    ```

    > 起初，一个用户只有一个组。BSD最先允许一个用户同时属于多个组，这一理念后来
    被其他的UNIX所效仿，并最终成为POSIX.1-1990标准

    > `/etc/group`文件格式，组名，有加密的密码，组ID，组成员的名字(`,`分隔)
    ```
    steve:x:1000:
    docker:x:999:steve
    ```

4. `/etc/passwd`文件格式

   > For more information about `passwd`, see [Ch8](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch8.md)

    ```
    steve:x:1000:1000:Steve:/home/steve:/usr/bin/zsh
    _flatpak:x:122:130:Flatpak system-wide installation helper,,,:/nonexistent:/usr/sbin/nologin
    ```

    * user name
    * x 代表着一个加密的密码被存储到了`/etc/shadow`文件之中
    * user id
    * group id
    * comment field: 注释字段，用来对用户补充一些额外信息，比如`flatpak`中写的是
    `system-wide installation helper`
    * home dir
    * shell

    > flatpak的homedir是`nonexistent`，login shell是`/usr/sbin/nologin`

    > [understanding-etcpasswd-file-format](https://www.cyberciti.biz/faq/understanding-etcpasswd-file-format/)

5. soft link

   在多数情况下，对soft link的调用会被自动替换为其指向的文件，这一过程会递归进
   行，直到所有的soft link均被替换。但为了应对循环引用，内核对替换的次数做了限
   制
   
6. 文件名

   在大多数Linux的文件系统中，文件名最多长`255`个字符。可以包含除了`/`和`\0`的
   所有字符。但只建议使用字母(26*2=52，大小写)、数字(10)、下划线、点以及`-`，
   这些总共65个符号，被SUSv3称为portable filename character set。此外还应避免`-`
   作为文件名的开始，避免被shell当成命令参数

7. 文件的换行符及结束符

   * 换行符是`\n(ascii: 10)`
   * UNIX没有文件结束符的概念，读取文件时如无数据返回，则认定达到EOF
   > 在non-blocking的IO时，如果没有数据返回，则函数返回EOF

8. 进程的RUID，EUID以及saved-set-UID(以及RGIP, EGID, saved-set-GID)
   
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


9. init进程

    内核在启动时，会创建一个名为`init`的特殊进程，为所有进程的父进程
    其程序文件`/sbin/init`，其PID通常是1

    ```shell
    $ cd sbin
    $ ls -l |grep init
    lrwxrwxrwx     20 root root   19 Apr 04:12  init -> /lib/systemd/systemd
    ```

10. 子进程会默认继承父进程的环境变量

    当使用`exec()`对子进程进行替换时，可以使用参数选择换掉或者不换掉环境变量

11. 资源限制

    每个进程都会消耗资源，可以使用`setrlimit`函数来为进程可消耗资源设置上限。
    限制包括2项，soft limit以及hard limit，soft limit是用来设置进程消耗资源
    的上限的，而hard limit则是设置soft limit的上限的。非特选进程(EUID!=0)的
    soft limit可以设置为0到hard limit的任何值，但是其hard limit只能调低，不能
    调高。

    由`fork`创建的子进程，会继承其父进程对资源消耗的限制

    > shell资源消耗的限制可以使用`ulimit`命令进行调整

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

    shared libraryd就是为了解决static library的问题才出现的。首先说一下static library
    为什么static，因为其是在编译时进行拷贝(从static library拷贝到最终的可执行文件，想
    一下rust的编译期的静态检查)。而shared library则是在运行时(runtime)才将所需要用到的
    函数加载进内存，而且倘若多个程序都链接了同一个shared library，那么只有一份library
    会被加载进入内存，供多个程序使用(节省内存大小)。在链接得到的最终binary里有的也只是
    所需被链接的函数的一个record而不是函数本身(节省磁盘)。由于是在运行时才加载入内存，
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
