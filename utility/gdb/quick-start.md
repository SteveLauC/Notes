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
