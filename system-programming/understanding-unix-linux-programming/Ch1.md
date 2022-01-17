1. 程序对设备的操作都是通过内核进行的。在编写普通程序时，程序享受内核提供的服务，这对程序员来说是透明的，但
   对系统编程来说，要做的，就是去了解内核如何给程序提供服务，提供怎样的服务。

2. UNIX并不提供恢复被删除文件的功能，其中一个原因是UNIX是一个多用户系统，当一个文件被删除以后，它所占用的存
   储空间可能立即分配给其他用户的文件，所以成功恢复的可能性很低。

3. 可以通过CTRL+D来退出bc计算器，书上说dc是bc的后端，bc是dc的预处理器，dc接受逆波兰式，而bc用的则是人类习惯
   的中缀表达式。
   > 现在的bc貌似独立于dc了

4.fgets() reads  in  at  most one less than size characters from stream and stores them into the buffer
  pointed to by s.  Reading stops after an EOF or a newline.  If a newline is read, it  is  stored  into
  the buffer.  A terminating null byte ('\0') is stored after the last character in the buffer.
  ```C
  char *fgets(char *s, int size, FILE *stream);
  ```
  fgets() returns s on success, and NULL on error or when end of file occurs while  no  characters  have
  been read.
  这个函数，最多读取size-1个char到buffer里，然后最后给buffer追加一个'\0'来表示字符串的结束。如果SIZE-1个
  char没有读到就遇到了EOF或者'\n'则会提前终止。如果buffer不够大，放不下SIZE个char，程序会崩溃。
