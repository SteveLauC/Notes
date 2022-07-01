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

