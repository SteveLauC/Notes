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
   *while no characters have been read.*这句话的很半句很关键，只有当`仅有EOF`时
   才返回NULL。

   >  Reading stops after an EOF or a newline

   这个函数当遇到`EOF`或者`newline`时都会返回，但只有当遇到`仅EOF`时才返回`NULL`。
   所以上面的代码片段中的这种写法，是loop until EOF.

   |contents|return value|
   |--------|------------|
   |xxx<NEWLINE>| valid ptr|
   |xxx<EOF>|valid ptr|
   |<NEWLINE>|valid ptr|
   |<EOF>|NULL|


3. 原来rust里的`raw string`中的`#`可以不限数量的，只要双引号前后的`#`数量是一样的
   就可以。

   ```rust
   let s1 = r"raw string";
   let s2 = r#"raw string"#;
   let s3 = r##"raw string"##;
   let s4 = r###"raw string"###;
   ```

4. 发现`std::io::Read`里的函数都没有处理`newline`，如果想要遇到`newline`就使函数返回
   只能使用`std::io::BufRead`里的函数了。
