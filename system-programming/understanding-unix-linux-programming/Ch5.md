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

11. 回车(carriage return `\r`: ascii 13)和换行(new line`\n`: ascii 10)的区别  

    在历史上，那种打字机器，光标可以左右移动，也可以上下移动(纸固定)，回车就是将光
    标移动到最左边，而换行则是移动到下一行(无左右移动)。  
    
    到了现在的计算机，`\n`则是既有回车又有换行，所以在UNIX系统中，标志文件的一行结
    束使用的是`\n`，`\r`则什么都不是。在c语言中(后来语言大多受他影响也是这样)，`\r`
    仍然是将光标移动到最左边的功能，而`\n`则是回车加换行。

12. 如果是连续地从`stdin`读取字节，可以使用`std::io::Read`中的`bytes()`函数

13. listchar这个程序说明了几件事:
    1. 我们输入的字符，只有在摁下回车后才发送给了listchar程序，说明终端的输入是
    缓冲的
    2. 我们摁下的return键是回车(ascii 13)，而listchar接受到的则是ascii 10
    3. 当listchar程序向终端发送ascii 10时，终端的光标做出的反应则是既回车又换行
    (和11点中记录的一致。)

    > 这些东西都是由终端驱动程序完成的，对普通用户来说是透明的。

14. stty - change and print terminal line settings 
    stty程序可以用来查看或者修改终端驱动程序的行为。

    比如使用`stty -a`命令来查看所有关于终端驱动程序的设置:

    ```shell
    ➜  02.listchars git:(main) stty -a
    speed 38400 baud; rows 44; columns 198; line = 0; 
    intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>; eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R; werase = ^W; lnext = ^V; discard = ^O;
    min = 1; time = 0;
    -parenb -parodd -cmspar cs8 -hupcl -cstopb cread -clocal -crtscts
    -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr icrnl ixon -ixoff -iuclc -ixany -imaxbel -iutf8
    opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
    isig icanon iexten echo echoe echok -echonl -noflsh -xcase -tostop -echoprt echoctl echoke -flusho -extproc
    ```

    比如在上面，`^C`是中断，`^D`是EOF，这些是控制字符。

    `echo`是个布尔值，代表回显的打开与否，布尔值的变量，前面有`-`则表示布尔值为
    假，相应功能关闭，没有`-`则表示布尔真，相应功能打开。

    除了可以从配置变量类型的角度区分(控制字符是char类型，有的是布尔类型)，还可以
    从功能上，有的控制输入，有的控制输出。比如`icrnl`指的是在输入中，将回车改为换
    行，`olcuc`指的是在输出中，将所有小写字符变为大写字符(lc to uc)。

    使用`stty`修改终端驱动程序配置的命令示例:

    ```shell
    stty -echo # 关闭回显，zsh不适用，切换bash测试
    stty olcuc # 使输出全部转为大写字母
    ```
    
15. 编写stty程序
    编写控制tty驱动程序的程序和编写控制文件连接设置是类似的，都是先拿到这个设置，
    然后对设置进行修改，再将设置送回去。在文件连接设置那里，我们拿到的设置是一个
    `int`，而stty的设置我们拿到的，则是一个结构体。

    ```rust
    #[repr(C)]
    pub struct termios {
        pub c_iflag: tcflag_t,
        pub c_oflag: tcflag_t,
        pub c_cflag: tcflag_t,
        pub c_lflag: tcflag_t,
        pub c_line: cc_t,
        pub c_cc: [cc_t; 32],
        pub c_ispeed: speed_t,
    r   pub c_ospeed: speed_t,
    }
    ```

    这是结构体的完整样子，使用`man termios`并没有给出结构体的定义，所以我直接从
    libc这个crate那里拿过来了。

    我们估计用不到这么多的字段，我们会用到前4个字段，`i/o/c/l`分别意味着`input/
    output/control/local`，是stty设置属性的4种分类。其类型`tcflag_t`是`unsigned
    int`的别名。

    所以写`stty`的过程蛮清晰，我们先准备好一个结构体buffer，然后调用`int tcgetattr(
    int fd, struct termios *termios_p);`拿到这个配置，放到我们的buffer中，然后要知道你
    要修改的设置属于4类中的哪类，然后对其相应的字段进行修改，再使用`int tcsetattr(int 
    fd, int optional_actions, const struct termios *termios_p);`将其送回内核。注意看这
    2个函数的参数，都有一个参数是`fd`，和文件连接一样，都是面向`fd`的。

    > 蛮有意思的是，这个get和set的系统调用就像是搬运工一样，从内核态搬运到用户态然后再
    搬运回去。

    `tcsetattr`里的`optional_actions`参数用来设置，你做出的更改何时生效：
    * TCSANOW the change occurs immediately.立刻生效

    * TCSADRAIN the change occurs after all output written to fd has been 
    transmitted. This option should be used when changing parameters that 
    affect output.
    等待直到驱动程序队列中的所有输出都被传送到终端，然后进行驱动程序的更新

    * TCSAFLUSH the change occurs after all output written to the object referr
    ed by fd has been transmitted, and all input that has been received but not 
    read will be discarded before the change is made.
    等到直到驱动程序队列中的所有输出都被传送出去，然后释放所有队列中的输入数据，
    并进行一定的变化。

    > 没有搞懂后面两个选项，什么是fd以及object referred by fd？

    最后的两个字段，`c_ispeed`和`c_ospeed`是输入和输出的波特率


16. rust中的bitwise not是`!`，而c中的则是`~`

17. `termios.h`中有这两个函数可以用来查看输入输出的波特率

    ‵‵`c
    speed_t cfgetispeed(const struct termios *termios_p);
    speed_t cfgetospeed(const struct termios *termios_p);
    ```
    其实就是直接访问了后两个字段。
    
    > The speed values are stored in the struct termios structure, but don’t try 
      to access them in the struct termios structure directly. Instead, you should 
      use the following functions to read and store them: 
      
    文档中说不要去直接访问这两个speed_t变量，而是调用wrapper函数

    > The functions cfsetospeed and cfsetispeed report errors only for speed values 
    that the system simply cannot handle. If you specify a speed value that is 
    basically acceptable, then those functions will succeed. But they do not check 
    that a particular hardware device can actually support the specified speeds—in 
    fact, they don’t know which device you plan to set the speed for. If you use 
    tcsetattr to set the speed of a particular device to a value that it cannot 
    handle, tcsetattr returns -1.

18. gnu对bsd是有一定的兼容性的，比如波特率的文档中写道``
    > Portability note: In the GNU C Library, the functions above accept speeds 
    measured in bits per second as input, and return speed values measured in 
    bits per second. Other libraries require speeds to be indicated by special 
    codes. For POSIX.1 portability, you must use one of the following symbols 
    to represent the speed; their precise numeric values are system-dependent, 
    but each name has a fixed meaning: B110 stands for 110 bps, B300 for 300 bps, 
    and so on. There is no portable way to represent any speed but these, but 
    these are the only speeds that typical serial lines can support.
    B0  B50  B75  B110  B134  B150  B200
    B300  B600  B1200  B1800  B2400  B4800
    B9600  B19200  B38400  B57600  B115200
    B230400  B460800
    BSD defines two additional speed symbols as aliases: EXTA is an alias for 
    B19200 and EXTB is an alias for B38400. These aliases are obsolete. 
 
    上面这句话，说`EXTB`和`EXTA`是两个过时的常量，但你在`termios.h`中还是可以找
    到这两个常量:

    ```c
    /* c_cflag bit meaning */
    #define  B0 0000000     /* hang up */
    #define  B50    0000001
    #define  B75    0000002
    #define  B110   0000003
    #define  B134   0000004
    #define  B150   0000005
    #define  B200   0000006
    #define  B300   0000007
    #define  B600   0000010
    #define  B1200  0000011
    #define  B1800  0000012
    #define  B2400  0000013
    #define  B4800  0000014
    #define  B9600  0000015
    #define  B19200 0000016
    #define  B38400 0000017
    #ifdef __USE_MISC
    # define EXTA B19200
    # define EXTB B38400
    ```

19. `termios`中`c_cc`数组是`unsigned char`的长为32的数组，正常是可以存放`ascii char`
    但里面存储的并不是相应的`ascii char`的数值编码，而是相对于`A`的ascii编码的值的差+1。
    比如，`intr`中断使用的字符是`C`，但里面存储的值是`3`，编码减掉`A`的编码再+1.
    
    > 那么你想要将这个char拿出来的话，就需要`termios.c_cc[INTR]+'A'-1`，再使用
    `%c`格式控制符号打印出来

20. 在rust中可以从`u8`转换为`char`，因为`char`实现了`From<u8>`
    > impl From<u8> for char

21. rust中的char是u32类型，所有的char都是`u32`，但反过来不是。所以`from_u32()`函
    数返回的是`Option<char>`。但是`From<u8>`这个trait里面的`from`函数返回的直接 
    就是char

22. rust错误代码[E0116](https://doc.rust-lang.org/error-index.html#E0116)
    不能给不在当前crate定义的类型实现`impl`块
   
    To fix this problem, you can either:
    1. define a trait that has the desired associated functions/types/constants 
    and implement the trait for the type in question 
    2. define a new type wrapping the type and define an implementation on the 
    new type

    Note that using the type keyword does not work here because type only introduces a type alias:

23. rust中如果要用函数来初始化一个常量，那么此函数必须是`const`的。[E0015]
