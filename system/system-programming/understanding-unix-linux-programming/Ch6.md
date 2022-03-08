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
