#### Ch4: File I/O The Universal I/O Model

1. 3个标准的文件描述符，与其说是默认打开的，更准确地说是继承父进程的

   这3个fd，除了用`0/1/2`来表示，在代码中，可以使用其`POSIX name`来指定，只需
   `#include <unistd.h>`就可以

    ```c
    #include <stdio.h>
    #include <unistd.h>

    int main(void) {
        printf("stdio: %d\n", STDIN_FILENO);
        printf("stdout: %d\n", STDOUT_FILENO);
        printf("stderr: %d\n", STDERR_FILENO);

        return 0;
    }
    ```

    ```shell
    $ gcc main.c && ./a.out
    stdio: 0
    stdout: 1
    stderr: 2
    ```

2. `freopen` library function

    此函数可以实现重定向，比如`freopen("tmp_file", "w", stdout)`可以将`stdout`重
    定向到`tmp_file`，`freopen("/dev/tty", "w", stdout)`可以再弄回来。

    不过使用`freopen`再弄回去貌似有问题

    > [link](https://stackoverflow.com/a/1908758/14092446)

3. 对于有的设备的某些特有属性，`open/read/write`这些通用的IO做不到的，可以使用
`ioctl`

    [understanding unix linux programming:Ch5 22](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/understanding-unix-linux-programming/Ch5.md)

4. `open`系统掉用在新建文件的时候所给出的`mode`参数只是请求而不是命令，最终新建
   的文件的权限是什么，还得看`umask`

    > int open(const char *pathname, int flags, mode_t mode);

    [UULP Ch3 22](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/understanding-unix-linux-programming/Ch3.md)

5. `open(2)`的`flag`参数的可选值
    
    | Flag(20 in total)     | Purpose        |
    |----------|----------------|
    |O_RDONLY  | open for reading only|
    |O_WRONLY  | open for writing only|
    |O_RDWR    | open for reading and writing|
    | =========|======|
    |O_CLOEXEC | set the close-on-exec flag|
    |O_CREAT|  create file if it does not already exist|
    |O_DIRECTORY| fail if pathname is not a dir(service opendir(3))|
    |O_EXCL| with O_CREAT: create file exclusively|
    |O_NOCTTY|don't let `pathname` become the controlling terminal|
    |O_NOFOLLOW| don't deref symbolic link|
    |O_TMPFILE(linux-specifi)||
    |O_TRUNC|truncate existing file to zero length|
    |=|=|
    |O_APPEND|writes are always appended to end of file|
    |O_ASYNC| generate a signal when I/O is possible|
    |O_DIRECT |FILE I/O bypasses buffer (kernel) cache|
    |O_DSYNC|provide `synchronized I/O data intergrity`|
    |O_LARGEFILE|used on 32-bit systems to open large files|
    |O_NOATIME| don't update atime on read(2)|
    |O_NONBLOCK or O_NDELAY| open in non-blocking mode|
    |O_PATH||
    |O_SYNC or O_FSYNC(just an alias to O_SYNC ) or O_RSYNC (O_RSYNC is not supported under Linux, just an alias to O_SYNC)|make file writes synchronous (synchronized I/O file integrity completion)|

    > File writes are asynchronous by default cause the existance of `buffer cache`.
    >
    > To find out what are `synchronized I/O data integrity completion` and 
    > `synchronized I/O file integrity completion`, see 
    > [Ch13: 6](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch13.md)

6. 5中的`flags`可以被大致分为3组:
    1. file access mode flags: `O_RDONLY/O_WDONLY/O_RDWR` 可以在`fcntl`中使用
    `F_GETFL`(get flag)来拿到
    2. file creation flags: 不能被拿到或者修改
    3. open file status flags: 可以使用`fcntl`中的`F_GETFL/F_SETFL`来拿或者修改
    
    > status flag is the only category of flag that can be modified. Attempts to
    modify other flags are ignored

7. `O_ASYNC`这个用来激活`signal-dirven I/O`的flag在`open(2)`中使用是没有用的，
   若想用只可以在`fcntl(2)`中使用`F_SETFL`来激活

   > see `man 2 open` BUGS for more info

8. 如果在使用`open(2)`时创建了新文件但没有给定`mode`参数，新文件的mode则会被随
   机设置一个stack上的值
   
9. `O_NOATIME`要求进程的EUID(实际上是进程的fs的user ID)必须和被访问文件的owner是
   同一个人，或者要求进程是priviledged
   
10. `O_TRUNC`和`O_RDONLY`一起使用，Linux允许这种操作(很多UNIX和Linux在这方面行为
    相同)，但是`SUSv3`标准并没有对这种情况下的行为进行指定

11. `write(2)`只写部分数据的原因(返回值`<`count)参数，对于`磁盘文件`:
    1. 磁盘满了
    2. 进程的资源限制中文件大小的限制达到了

    > RLIMIT_FSIZE The maximum size of file the process can create. Trying to 
    write a larger file causes a signal: SIGXFSZ. 

12. 对于每一个打开的文件，kernel都会记录一个`file offset`，有时也被称为`read-write
    offset or pointer`。是一个相对于`文件开始`的字节偏移量

    ```c
    #include <sys/types.h>
    #include <unistd.h>

    off_t lseek(int fd, off_t offset, int whence);
    // l代表的是返回值是long，早期的UNIX有seek调用
    ```

    可以使用`lseek`函数来调整

    `whence`参数:
    ```
    SEEK_SET The file offset is set to offset bytes.

    SEEK_CUR The file offset is set to its current location plus offset bytes.

    SEEK_END The file offset is set to the size of the file plus offset bytes.
    ```

    在早期的UNIX里面，没有`SEEK_*`的这种宏，而是`0/1/2`的magic number。在一些早
    期的BSD中，这些值有别的名字`L_SET/L_INCR/L_XTN`

    ```c
    // from ""
    /* values for the whence argument to lseek.  */
    #ifndef	_stdio_h		/* <stdio.h> has the same definitions.  */
    # define seek_set	0	/* seek from beginning of file.  */
    # define seek_cur	1	/* seek from current position.  */
    # define seek_end	2	/* seek from end of file.  */
    # ifdef __use_gnu
    #  define seek_data	3	/* seek to next data.  */
    #  define seek_hole	4	/* seek to next hole.  */
    # endif
    #endif
    ```

    注意一下`SEEK_END`是将偏移量设到`文件大小`+offset，`lseek(fd, 0, SEEK_END)`此
    时指针指到的并不是文件的内容，是文件最后一个字节的后一个字节

    使用`lseek`单纯地改变内核中对`fd`的offset的记录，并不会造成任何的磁盘访问

    pipe, FIFO, socket以及terminal都是不可以seek的

13. hole, sparse file
    Linux允许`lseek`超过文件的大小，如果此时使用`write`进行写，就会产生洞。在洞
    里进行读会返回0。洞的存在意味着一个文件的文件大小可以超过它真实地利用地磁盘
    空间大小

    就比如，你创建了一新文件，使用`lseek(fd, 5, SEEK_END)`来将指针超过文件6个字
    节处，然后在这里开始`write(fd, "hello", 5)`，那么最终文件的大小就是`10`

    ![illustration](https://github.com/SteveLauC/pic/blob/main/lseek_demo.jpeg)

    sparse file是那种有很多空字节(0)的文件，与其真实地在磁盘上存储这么多无用的0
    ，不如在文件的元数据上标记一下哪里到哪里是0，这样就可以不在磁盘上真实地分配
    了。达到节省空间的作用

    GNU的cp现在可以检测sparse file，并在复制时使dest文件也是sparse file。我写的
    用`read`实现的cp没有这个功能。

    ```
    --sparse=WHEN
              control creation of sparse files. See below

    By default, sparse SOURCE files are detected by a crude heuristic and the 
    corresponding DEST file is made sparse as well.  That is the behavior selected 
    by --sparse=auto. Specify --sparse=always to create a sparse DEST file 
    whenever the SOURCE file contains a long enough sequence of zero bytes.  
    Use --sparse=never to inhibit creation of sparse files.

    这个选项就是gnu cp检测sparse file的开关，其默认值是`auto`，也就是在复制的时
    侯检测到sparse file则拷贝时dest文件也是sparse file。`always`值可以使其在遇到
    连续的真实分配在磁盘上的0(不是sparse file)时，复制出sparse file。如果使用
    `never`则不管原文件是真实分配的0或是sparse file，dest都会是真实分配的0
    ```

    举个例子说明GNU cp的此项功能
    1. 创建一个sparse file(大文件则比较好观察)
    2. 使用gnu cp和自己写的copy对其进行复制
    3. 观察上述2产物的block占用

    ```c
    #include <assert.h>
    #include <stdio.h>
    #include <fcntl.h>
    #include <stdlib.h>
    #include <unistd.h>
    
    int main(void)
    {
    	int fd = creat("sparse_file", 0666);
    
    	lseek(fd, 10000, SEEK_SET);
    	assert(5==write(fd, "hello", 5));
    	close(fd);
    	return 0;
    }
    ```

    ```shell
    $ cp sparse_file gnu
    $ cp sparse_file my_cp
    $ stat gnu
      File: sparse_file_copied_using_gnu_cp
      Size: 10005           Blocks: 8          IO Block: 4096   regular file
    Device: 803h/2051d      Inode: 3541392     Links: 1
    Access: (0664/-rw-rw-r--)  Uid: ( 1000/   steve)   Gid: ( 1000/   steve)
    Access: 2022-07-08 11:11:21.946126365 +0800
    Modify: 2022-07-08 11:11:21.946126365 +0800
    Change: 2022-07-08 11:11:21.946126365 +0800
    Birth: 2022-07-08 11:11:21.946126365 +0800
    $ stat my_cp
      File: sparse_file_copied_using_my_copy
      Size: 10005           Blocks: 24         IO Block: 4096   regular file
    Device: 803h/2051d      Inode: 3541434     Links: 1
    Access: (0664/-rw-rw-r--)  Uid: ( 1000/   steve)   Gid: ( 1000/   steve)
    Access: 2022-07-08 11:11:35.402828404 +0800
    Modify: 2022-07-08 11:12:17.016997942 +0800
    Change: 2022-07-08 11:12:17.016997942 +0800
    Birth: 2022-07-08 11:11:35.402828404 +0800
    ```

14. text file busy
    出现这种错误说明binary在运行时被修改

    [link](https://stackoverflow.com/questions/16764946/what-generates-the-text-file-busy-message-in-unix)
