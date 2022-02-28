1. 想要使用`gdb`进行调试时，`gcc`需要给出`-g`的参数，来让它在编译时产生一些`gdb`  
   可以进行使用的信息。
   
   > Produce debugging information in the operating system's native format 
   (stabs, COFF, XCOFF, or DWARF).  GDB can work with this debugging information. 
   On most systems that use stabs format, -g enables use of extra debugging 
   information that only GDB can use; this extra information makes debugging work 
   better in GDB but probably makes other debuggers crash or refuse to read the 
   program.  If you want to control for certain whether to generate the extra 
   information, use -gstabs+, -gstabs, -gxcoff+, -gxcoff, or -gvms (see below).

2. `run`命令，可以运行程序。直到断点
   > 可以简写为`r`

3. `quit`命令，退出`gdb`

4. `list`命令，可以查看文件内容

5. 添加断点，可以使用`break`命令来添加断点。有两种添加的方式:
   1. `break function_name`在函数处打断点
   2. `break line_number`在具体的第几行设置断点
   > 如果不知道自己要设置断点的代码在哪一行，可以使用`info`来查看。

6. `next`和`stop`命令可以单步往下执行，执行下一`行`的代码。两者的区别在于，`next`  
   遇到函数会`step over`，也就是直接执行完函数，而`step`遇到函数则会`step into`.   
   也就是会执行这个函数。

7. 在`gdb`中运行`shell`命令，和`vim`一样，使用`!cmd`来运行。也可以使用`shell cmd`  
   来执行。或者想短暂地进入`shell`而不退出`gdb`

   ```shell
   (gdb) shell
   ~  # 你现在处于shell中，可以执行shell命令
   ~ exit # 使用`exit`命令来退出`shell`，回到`gdb`
   (gdb) # 回到了`gdb`
   ```

8. `print expresssion`指令，展示`expression`的值。

9. 当`step into`一个函数时，想要`step out`使用`finish`命令。

10. `gdb`调试时获得的输出是可以保存到文件中的，这是`gdb`的日志功能。
   
   * `set logging on`: 将日志功能打开
   * `set logging off`: 关闭日志功能
   * `set logging file file_name`: `gdb`默认追加到当前路径的`gdb.text`中，如果不  
     想输出到这个文件可以指定其他的文件。
   * `set logging overwrite on/off`: 设置是否覆盖，默认是`off`即追加，使用`on`可  
     以做到覆盖。

11. watchpoint
   可以用来观察某个位置的值是否会变化，当变化时就会将其打印出来。

12. debug core file
   `gdb`除了debug编译好的二进制，还可以就是当程序崩溃时，debug core file。 

   ```shell
   # 使用方法
   gdb binary_file core_file
   ```
   > 在ubuntu上开启core dump，首先需要解除`ulimit`限制，使用`ulimit -c unlimited`  
   来解除core file文件大小的限制。然后关闭apport，因为ubuntu上的core file是被piped  
   给apport来统一处理的，关掉它可以让我们在当前执行二进制的工作路径中拿到core file.  
   可能关掉它是不好的，目前还不是很了解。

13. debug正在运行的进程
   可以使用:
   * gdb program port_num
   * gdb -p port_num
  
   > 在ubuntu20.04LTS上测试发现即使是自己的进程，也是无法attch给gdb的，这是由于  
   ubuntu做的系统保护，KernelHardening，一个解决办法是给gdb二进制程序设置为sticky  
   bit，使运行程序的人变为root，或者使用`sudo`。但这个办法不是永久性的，在reboot  
   后会失效，可以参见这里[link](https://blog.mellenthin.de/archives/2010/10/18/gdb-attach-fails-with-ptrace-operation-not-permitted/#comment-141535)来永久解决这个问题，但我并没有设置。

14. 让程序从`main`函数开始执行，使用`start`指令。让程序从第一条指令开始执行，使用  
   `starti`指令。

15. 使用`x`命令以特定形式，展示给定地址的内存内容，比如`x/i $rip`将`rip`寄存器  
   (PC指针)中的内容以指令(i instruments)的形式打印出来

16. info inferiors: this lists information (inferior ID, PID, program) on each 
    inferior currently being debugged by the active gdb
    > 其中的一个重要信息是我们可以拿到进程的PID

17. `x`命令，以特定形式打印出某个内存内容的值，用法是`x address`，比如:   
    * `x/i $rip`打印当前pc指针指向的内存，以指令形式打印出。
    
    形式:  
    ```
    o - octal
    x - hexadecimal
    d - decimal
    u - unsigned decimal
    t - binary
    f - floating point
    a - address
    c - char
    s - string
    i - instruction
    ```
