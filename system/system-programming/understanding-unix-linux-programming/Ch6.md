1. 终端驱动程序的不同模式，规范模式和非规范模式
   
   17.3 Two Styles of Input: Canonical or Not
   POSIX systems support two basic modes of input: canonical and noncanonical.

   In canonical input processing mode, terminal input is processed in lines 
   terminated by newline ('\n'), EOF, or EOL characters. No input can be 
   read until an entire line has been typed by the user, and the read function 
   (see I/O Primitives) returns at most a single line of input, no matter how 
   many bytes are requested.

   In canonical input mode, the operating system provides input editing facilities:
   some characters are interpreted specially to perform editing operations within 
   the current line of text, such as ERASE and KILL. See Editing Characters.
   The constants _POSIX_MAX_CANON and MAX_CANON parameterize the maximum number 
   of bytes which may appear in a single line of canonical input. See Limits for 
   Files. You are guaranteed a maximum line length of at least MAX_CANON bytes, 
   but the maximum might be larger, and might even dynamically change size.

   In noncanonical input processing mode, characters are not grouped into lines, 
   and ERASE and KILL processing is not performed. The granularity with which bytes 
   are read in noncanonical input mode is controlled by the MIN and TIME settings. 
   See Noncanonical Input.

   Most programs use canonical input mode, because this gives the user a way to 
   edit input line by line. *The usual reason to use noncanonical mode is when the 
   program accepts single-character commands or provides its own editing facilities.*

   > 确实是遇到过不支持`canonical mode`的时候

   The choice of canonical or noncanonical input is controlled by the ICANON flag 
   in the `c_lflag` member of struct termios. See Local Modes.

   ```shell
   stty -icanon # enter non-canonical mode
   stty icanon  # enter canonical mode
   ```
  
   > 这个代码的演示，和`echo`的例子一样，应该选择`bash`而不是`zsh`。在`bash`中确
   实可以观察到缓冲被关闭，无法使用退格键进行编辑。没有了缓冲自然就不能对其进行
   编辑，因为输入的字符早就被终端驱动程序送走了。
    
   > 在写自己的UNIX程序的时候，应该考虑使用哪种模式

2. 终端驱动程序的模式有3种:

   1. canonical mode/cooked mode: 就是平常用的
   2. non-canonical mode: 没有缓冲和编辑功能
   3. raw mode: 貌似是什么也没有的一种模式

   ```
   * raw    same as -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr 
   -icrnl -ixon -ixoff -icanon -opost -isig -iuclc -ixany -imaxbel -xcase min 1 time 0
   * -raw   same as cooked
   上面这个摘自`man stty`，说明我们可以通过stty进入`raw mode`
   ```

3. linux的`non-blocking IO`使用的是`O_NONBLOCK`这个标记，可以在使用`fcntl`函数
   来对`fd`进行操作
   
   ```c
   int terflags = fcntl(0, F_GETFD);
   terflags |= O_NONBLOCK;
   fcntl(0, F_SETFD, terflags);
   ``` 

4. 在c中检查某个字符串是否包含字符，可以使用`strchr/strrchr`

   ```c
   #include <string.h>

   char *strchr(const char *s, int c);
   char *strrchr(const char *s, int c);
   ```
   The strchr() function returns a pointer to the first occurrence of the character c in the string s.  
   The strrchr() function returns a pointer to the last occurrence of the character c in the string s.
    
   如果`c`并未出现在`s`中，则返回`NULL`

5. c中的char的大小写转换

   ```c
   #include <ctype.h>

   int toupper(int c);
   int tolower(int c);

   int toupper_l(int c, locale_t locale);
   int tolower_l(int c, locale_t locale);
   ```
   
   标准规定`c`这个参数要么是`EOF`，要么是可以表示的字符。
   If c is neither an unsigned char value nor EOF, the behavior of these 
   functions is undefined.
   这句话的意思是，如果`c`的值不是`-1..=255`的话，就是未定义行为吗? 
   如果其参数是`EOF`，则直接返回，否则返回相应转换后的结果。

   还有下面这点: 
   If the argument c is of type char, it must be cast to unsigned char, as in 
   the following example:

   ```c
   char c;
   ...
   res = toupper((unsigned char) c);
   ```
   This is necessary because char may be the equivalent signed char, in which 
   case a byte where the top bit is set would be sign extended when converting 
   to int, yielding a value  that  is  outside  the range of unsigned char.
   `gcc`的`char`默认是`signed char`，这个说的我感觉防的是`signed char`的第一个
   bit是1，并且后面7个比特不全为1的情况，也就是数值在`-128..-1`的情况，这种情况
   转为`int`后，符号扩展，前面3个字节都是全`1`，自然是在`-1..=255`以外，发生UB。
   先转换为`unsigned char`后，扩展到`int`发生的就是补0，其值就会在`0..=255`了。
   那么就避免了UB。

6. 当开启`non-blocking IO`的时候，如果`read`没有拿到输入的话，就返回0. getchar()
   没有拿到数据则返回EOF

7. rust中的`Optoin`和`Result`都是move语义的，`unwrap(self)`之类的函数会取走所有
   权

8. c中的`getchar()`在遇到EOF，返回EOF；non-blocking模式下没有遇到输入，返回EOF。
   而rust中的`std::io::Read`中的`bytes()`函数在使用`for _ in _`遍历时，遇到EOF
   遍历会自动停下来，但是non-blocking模式下没有输入，则会Error，其类型是`std::io
   ::ErrorKind::WouldBlock`

9. 修改tty的驱动配置使用的函数是: 

   ```c
   int tcgetattr(int fd, struct termios *termios_p);

   int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
   ```

   在rust中它有safe的binding crate，`termios` [link](https://crates.io/crates/termios)

10. 使用`ctrl-c`中断程序，则会使当前进程停止，这个停止进程的信号的快捷键，有`termios`
    中`c_cc[VINTR]`来决定
  
    > VINTR  (003, ETX, Ctrl-C, or also 0177, DEL, rubout) Interrupt character (INTR).
    Send a SIGINT signal.  Recognized when ISIG is set, and then not passed as input.

11. signal是一种有限形式的进程间通信。从最初的UNIX就有了这个东西，此信号的编码一般是2

12. `ctrl-c`是如何工作的:
    1. 用户输入`ctrl-c`
    2. 终端驱动程序收到字符
    3. 检查`c_lflag`中的`ISIG`被开启，并且查看`c_cc[VINTR]`中的字符是不是C
    4. 驱动程序调用信号系统
    5. 信号系统发送`SIGINT`到进程中
    6. 进程收到`SIGINT`
    7. 进程消亡

13. 进程对信号的处理，分为3种策略:
    1. 默认处理
    2. 忽略
    3. 遇到信号时，调用用户自定义的信号处理函数


    > 默认的处理方法是`1`，如果要调用`2/3`需要在代码中使用`man 2 signal`进行指定
   
    ```c
    #include <signal.h>
 
    typedef void (*sighandler_t)(int);  // 这个函数指针指向的函数是需要一个int，返回void
    // 第二个参数的int参数就是`signum`
 
    sighandler_t signal(int signum, sighandler_t handler);
    ``` 

    ```
    // 文档为了跨平台性，不建议你用这个syscall
    The behavior of signal() varies across UNIX versions, and has also varied 
    historically across different versions of Linux.  Avoid its use: use 
    sigaction(2) instead.  See Portability below.

    signal() sets the disposition of the signal signum to handler, which is 
    either SIG_IGN(忽略), SIG_DFL(默认处理), or the address of a programmer-defined
    function(用户自定义的处理函数)(a "signal handler").

    If the signal signum is delivered to the process, then one of the following 
    happens:

       * If the disposition is set to SIG_IGN, then the signal is ignored.

       * If the disposition is set to SIG_DFL, then the default action associated 
         with the signal (see signal(7)) occurs.(使用man 7 signal查看进程对不同信号
         的默认处理方式)

       * If  the  disposition  is set to a function, then first either the
         disposition is reset to SIG_DFL, or the signal is blocked (see 
         Portability below), and then handler is called with argument signum.
          If invocation of the handler caused the signal to be blocked, then the signal is unblocked upon return from the handler.

   The signals SIGKILL and SIGSTOP cannot be caught or ignored.(这两个信号貌似是
   没办法更改处理方式的)
   ```

   ```c
   /* Fake signal functions.  */
   // SIG_DFL和SIG_IGN都是fn ptr

   #define SIG_ERR  ((__sighandler_t) -1)  /* Error return.  */
   #define SIG_DFL  ((__sighandler_t)  0)  /* Default action.  */
   #define SIG_IGN  ((__sighandler_t)  1)  /* Ignore signal.  */
   ```
14. 在rust中写c的函数，需要`extern "C" fn foo()`这样的函数签名
