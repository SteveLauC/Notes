1. 想要使用`gdb`进行调试时，`gcc`需要给出`-g`的参数，来让它在编译时产生一些`gdb`  
   可以进行使用的信息。
   
   > Produce debugging information in the operating system's native format  
   (stabs, COFF, XCOFF, or DWARF).  GDB can work with this debugging information.  
   On most systems that use stabs format, -g enables use of extra debugging  
   information that only GDB can use; this extra information makes debugging work  
   better in GDB but probably makes other debuggers crash or refuse to read the  
   program.  If you want to control for certain whether to generate the extra  
   information, use -gstabs+, -gstabs, -gxcoff+, -gxcoff, or -gvms (see below).  
