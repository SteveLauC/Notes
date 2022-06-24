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
   检查其是否出错需要先将errno设置为0，然后调用它，再看errno是否为0
