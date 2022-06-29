1. syscall调用的步骤(i386)
    1. invoke the wrapper function from the c library
    2. the wrapper function copies the provided arguments from the stack to some spicific
    registers
    3. the wrapper function copies the corresponding syscall number to a spicific
    register
    4. the wrapper function executes a `trap(0x80)` machine instruction, which 
    causes the processor
    to switch from user mode to kernel mode, and execute the code located in the trap vector
    5. the kernel invokes the `system_call()(located in the assembler file 
    arch/i386/entry.S)`, this will:
        1. saves register values onto the kernel stack
        2. checks the validity of the system call number
        3. checks the validity of the arguments(if it has) and execute the syscall
        4. restore registers values from the kernel stack and places the syscall 
        return value on the stack
        5. return to the wrapper function and switch the processor back to user mode
    6. the wrapper function checks the return value of the syscall, if any error 
    happened, set `errno` to the corresponding value
    7. the wrapper function returns to the calling function

2. 大多数library function并没有调用syscall，而有的比如`printf`则调用了

   > 隐藏在`malloc/free`下面的系统调用是`brk`

3. Linux上的standard c library不止`glibc`。比如，有用于嵌入式设备的消耗更少内存的
   `uClibc` 和 `diet libc`

4. 如何得知`glibc`的版本

    ```shell
    $ ldd --version
    ldd (Ubuntu GLIBC 2.35-0ubuntu3) 2.35
    Copyright (C) 2022 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    Written by Roland McGrath and Ulrich Drepper.
    ```

    ```c
    // 这种方法是在编译期做的，如果是在一台主机上编译，将binary放到另一个主机上
	// 宏，预处理器做的
    运行则无用，打印的是编译机器上的版本号
    #include <stdio.h>

    int main(void) {
        printf("%d.%d\n", __GLIBC__, __GLIBC_MINOR__);
        return 0;
    }
    ```

    ```c
    #include <stdio.h>
    #include <gnu/libc-version.h>

    int main(void) {
        const char * version = gnu_get_libc_version();
        printf("%s", version);
        return 0;
    }
    ```

5. 使用系统调用时应该及时地检查其返回值来查看其是否出错。少数的系统调用绝不会失
   败，比如`getpid`

   ```
   ERRORS
          These functions are always successful.
   ```

   失败的syscall会对errno进行赋值，而成功的syscall则不会。所以errno的值应该是上
   一个失败的syscall设置的，我们在`#include <errno.h>`来检查errno时应先确保此syscall
   确实失败了(对errno进行了赋值)

   极少数的syscall可以在成功时返回`-1`(比如`getpriority`)，对于这种syscall，想要
   检查其是否出错需要先将errno设置为0，然后调用它，再看errno是否为0，如果非0则是
   出错了


6. 常用的头文件以及其用途
    
    ```c
    #ifndef TLPI_HDR_H
    #define TLPI_HDR_H

    #include <sys/types.h>       /* Prevent accidental double inclusion */
    #include <stdio.h>           /* Type definitions used by many arguments */
    #include <unistd.h>          /* Prototypes of commonly used library functions, plus EXIT_SUCCESS and EXIT_FAILURE */
    #include <errno.h>           /* Prototypes for many syscalls */
    #include <string.h>          /* Commonly used string-handling functions */

    #endif
    ```

7. 在c中创建一个临时文件
    
    ```c
    #include <stdio.h>
    FILE *tmpfile(void);
 
    DESCRIPTION
    The tmpfile() function opens a unique temporary file in binary read/write (w+b) mode.
    The file will be automatically deleted when it is closed or the program terminates.
 
    RETURN VALUE
    The  tmpfile() function returns a stream descriptor, or NULL if a unique filename cannot 
    be generated or the unique file cannot be opened.  In the latter case, errno is set to
    indicate the error.
    ```

8. diff between `exit()` and `_exit()`

    exit() flushes io buffers and does some other things like run functions registered by atexit(). exit() invokes _end( )

    _exit() just ends the process without doing that. 
    
    > `_exit()` 和 `_Exit()`是完全相同的，前者来自POSIX，后镇来自C99

10. 在`fork`得到的子进程中，退出子进程应该使用`exit()`还是`_exit()`
    
    应该使用`_exit()`，因为:
    1. `fork`得到的子进程继承了父进程的buffer(c的buffer，而不是os的)
    2. `exit()`会调用其handler函数，从而对父进程的外部数据发生冲突

    > [link](https://stackoverflow.com/q/5422831/14092446)

    > 举个例子的代码

    ```c
    #include <stdio.h>
    #include <unistd.h>

    int main(void) {
        printf("hi");
        fork();
        // return 会将buffer都给flush掉
        // 所以`hi`会被输出两遍
        return 0;
    }
    ```

11. POSIX thread api在错误的时候并不返回`-1`，而是errno的值
    On success, pthread_create() returns 0; on error, it returns an error number,
    and the contents of *thread are undefined.

    我们可以使用如下的代码来诊断POSIX thread程序的错误
    ```c
    errno = pthread_create(&thread_n, NULL, func, &arg);
    if (errno != 0) {
        errExit("pthread_create");
    }
    ```

12. 关于errno
    正常的errno都是正数，可以使用`$ errno -l`来查看

13. c的`variadic arguments`，当一个函数长这样子，它就是`variadic function`，
    `void foo(TYPE fixed_argument, ...)`，可以使用`#include <stdarg.h>`中的
    宏来访问所有的参数

    > 使用`man stdarg`查看更多信息

    
    ```c
    #include <stdarg.h>

    void va_start(va_list ap, last);
    type va_arg(va_list ap, type);
    void va_end(va_list ap);
    void va_copy(va_list dest, va_list src);
    ```

    使用方法，需要先使用创建一个`va_list`类型的变量，然后交给`va_start`宏进行
    初始化(可能va_list内部有指针吧)，`va_start`的`last`是ariadic function的
    fixed_argument的参数名(last的意思是最后一个明确的已知的参数)。然后连续掉用
    `va_arg`来遍历参数，参数的类型要传给`type`参数，并且传入的参数必须是正确的。
    需要提醒的是，并不能在`va_arg`函数中知道有多少个参数，调用的人需要在
    `fixed_arguments`中将其表示出来(比如printf中使用format specifier的来显示)。
    在最后需要使用`va_end`来将其结束

    
    示例程序:
    ```c
    #include <stdarg.h>
    #include <stdio.h>
    #include <unistd.h>

    int my_printf(char * formatter, ...);

    int main(void) {
        my_printf("dsc", 1, "hello world", '!');
        return 0;
    }

    /*
     * purpose: a simple `printf` fork
     *
     * action: iterate over the `formatter` arguments and print the arguments
     *
     * arguments: string only contains formatter letters(d: int, s: char *, c: char )
     *
     * return: 0 on success, -1 on errno
    */
    int my_printf(char * formatter, ...) {
        va_list list; 
        va_start(list, formatter);

        char * p = formatter;
        while (*p != '\0') {
            switch (*p) {
                case 'd':
                    printf("%d", va_arg(list, int));
                    break;
                case 's':
                    printf("%s", va_arg(list, char *));
                    break;
                case 'c':
                    printf("%c", va_arg(list, int));
                    break;
            }
            p +=1;
        }
        fflush(stdout);

        va_end(list);
        return 0;
    }
    ```
