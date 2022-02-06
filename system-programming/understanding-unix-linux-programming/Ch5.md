1. 设备文件也有inode编号，设备文件的inode节点中存储了指向内核中此设备驱动程序的
   指针。

   ```shell
   ➜  pts l 
   total 0
   drwxr-xr-x  2 root  root      0 Jan 21 12:18 .
   drwxr-xr-x 21 root  root   4.6K Feb  2 18:43 ..
   crw--w----  1 steve tty  136, 1 Feb  5 20:12 1
   crw--w----  1 steve tty  136, 2 Feb  5 21:21 2
   crw--w----  1 steve tty  136, 3 Feb  2 21:56 3
   crw--w----  1 steve tty  136, 4 Feb  5 21:11 4
   crw--w----  1 steve tty  136, 5 Feb  5 21:21 5
   c---------  1 root  root   5, 2 Jan 21 12:18 ptmx
   ```
   在通常文件显示文件大小的地方，设备文件显示的不是大小。而是该设备的主设备号和
   从设备号，主设备号确定处理该设备的子程序(驱动程序)，而从设备号是参数，需要传
   递到子程序中。如上面代码中的`136,x`就是这样的`主设备号,从设备号`

2. c中的`char *fgets(char *s, int size, FILE *stream);`函数，如果代码中这样写:

   ```c
   while(fgets(buf, BUFSIZE, stdin) != NULL) {
       // some code...
   }
   ```

   > fgets() returns s on success, and NULL on error or when end of file occurs 
   while no characters have been read. 

   那说明这是一直读，直到遇到EOF。

   >  Reading stops after an EOF or a newline

   虽然遇到换行函数会返回，但不会返回NULL
