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

   > * raw    same as -ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr 
   -icrnl -ixon -ixoff -icanon -opost -isig -iuclc -ixany -imaxbel -xcase min 1 time 0
    * -raw   same as cooked
    上面这个摘自`man stty`，说明我们可以通过stty进入`raw mode`

