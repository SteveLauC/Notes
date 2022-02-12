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
   |xxx`NEWLINE`| valid ptr|
   |xxx`EOF`|valid ptr|
   |`NEWLINE`|valid ptr|
   |`EOF`|NULL|


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

5. 如果要在rust中模拟c函数`fgets`的行为，可以使用`std::io::BufRead::read_until`，并
   将`delimiter`设为`newline`。理论上和`fgets`的行为应该是一致的。

   |input|return value|
   |-----|------------|
   |xx`newline`|Ok(3)|
   |xx`EOF`|Ok(2)|
   |`newline`|Ok(1)|
   |`EOF`|Ok(0)|

   和`2`中的图表对比，发现蛮一致的。

6. 目录并不知道一个文件是设备文件还是磁盘文件，因为目录仅仅是文件名和inode的映射。但是
   inode节点知道，磁盘文件的inode节点中存储了数据块的编号，设备文件的inode节点中存储的
   是驱动程序的地址。在`stat`结构体中的`st_mode`字段可以区别是什么文件。

7. 磁盘文件与设备文件具有相似之处，可以进行读写(如果有权限的话)。他们也有不同之处，比如
   都具有各自独有的属性。

8. 文件(磁盘)连接的属性: (介绍2点)
   1. 缓冲
   2. 自动添加模式: 自动添加模式指的是，`O_APPEND`这个参数，即对文件的写是追加，需要注意
   的是，追加是由两部组成的，先lseek指针到文件尾，然后再写，当`O_APPEND`被开启时，这两个
   子步骤会变为一个原子操作，由此可以免疫当不同文件都追加时可能会造成的race condition.

   > The file is opened in append mode.  Before each write(2), the file offset is 
   positioned at the end of the file, as if with lseek(2).  The modification of 
   the file offset and the write operation are performed as a single atomic step.

   > 在rust的`pub fn append(&mut self, append: bool) -> &mut Self`也写到了For most 
   filesystems, the operating system guarantees that all writes are atomic: no 
   writes get mangled because another process writes at the same time.

   与磁盘文件有关的属性被编码在`int`变量中，可以使用`fcntl`函数进行抓取。注意属性是与连接
   相关的，每一个连接都是一个文件描述符，也就是和`fd`相关的。

   下面给出关闭缓冲的代码:
   ```c
   #include <stdio.h>
   #include <fcntl.h>
   #include <stdlib.h>

   int main(){
       // get file descriptor
       int fd = open("./main.c", O_WRONLY); 
       if (fd == 0) {
            fprintf(stderr, "cannot open file");
            perror(NULL);
            exit(-1);
       }

       // fetch configuration
       int cfg = fcntl(fd, F_GETFD); 
       // change configuration
       cfg|=O_SYNC;                                // set O_APPEND cfg|=O_APPEND
       // send it back
       int res = fcntl(fd, F_SETFD, cfg);

       if (res == 1) {
            perror("setting sync");
       }
    }
    ```
   > 上面给出的代码是使用`fcntl`来操纵文件描述符的，我们也可以在使用`open`打来文
   件时就对文件描述符进行操作。

9. `O_EXCL`，这个flag用于避免不同进程同时创建相同名字的文件。一般来说，在创建文件
   时会先使用`stat`查看文件是否存在，不存在则调用`open`创建。但当两个进程都在这样
   做，且`stat`和`open`没有构成原子操作时，就会出问题，`O_EXCL`会令两个函数成为原
   子操作。

10. 打开文件的一些`flag`在rust中独立成为了构造函数`OpenOption`，其他的没有的可以使
   用`std::os::unix::fs::OpenOptionsExt`里面的`fn custom_flags(&mut self, flags: 
   i32) -> &mut Self;`来配置，flags的类型是i32，rust并没有给你这些配置的数字宏，需
   要引入`libc`
