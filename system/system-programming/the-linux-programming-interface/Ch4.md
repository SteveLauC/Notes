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
    
    | Flag     | Purpose        |
    |----------|----------------|
    |O_RDONLY  | open for reading only|
    |O_WRONLY  | open for writing only|
    |O_RDWR    | open for reading and writing|
    | =========|======|
    |O_CLOEXEC | set the close-on-exec flag|
    |O_CREAT|  create file if it does not already exist|
    |O_DIRECT |FILE I/O bypasses buffer (kernel) cache|
    |O_DIRECTORY| fail if pathname is not a dir(service opendir(3))|
    |O_EXCL| with O_CREAT: create file exclusively|
    |O_LARGEFILE|used on 32-bit systems to open large files|
    |O_NOATIME| don't update atime on read(2)|
    |O_NOCTTY|don't let `pathname` become the controlling terminal|
    |O_NOFOLLOW| don't deref symbolic link|
    |O_TRUNC|truncate existing file to zero length|
    |=|=|
    |O_APPEND|writes are always appended to end of file|
    |O_ASYNC| generate a signal when I/O is possible|
    |O_DSYNC|provide synchronized I/O data intergrity|
    |O_NONBLOCKING| open in non-blocking mode|
    |O_SYNC|make file writes synchronous|

6. 5中的`flags`可以被大致分为3组:
    1. file access mode flags: `O_RDONLY/O_WDONLY/O_RDWR` 可以在`fcntl`中使用
    `F_GETFL`(get flag)来拿到
    2. file creation flags: 不能被拿到或者修改
    3. open file status flags: 可以使用`fcntl`中的`F_GETFL/F_SETFL`来拿或者修改

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
