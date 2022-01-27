1. ls命令，如果给了参数，并且给的参数是文件夹，那么它会列出文件夹里面的文件及文
   件夹；如果给的参数是文件名，那么会给出此文件的信息；如果给出一些模糊搜索，那
   会将当前工作路径下复合模式匹配的文件或文件夹打印出来。

2. 在vim中可以打开一个文件夹，`vim dir`

   ```shell
   " ============================================================================
   " Netrw Directory Listing                                        (netrw v156)
   "   /home/Goldname/myDirectory
   "   Sorted by      name
   "   Sort sequence: [\/]$,\<core\%(\.\d\+\)\=\>,\.h$,\.c$,\.cpp$,\~\=\*$,*,\.o$,\
   "   Quick Help: <F1>:help  -:go up dir  D:delete  R:rename  s:sort-by  x:special
   " ==============================================================================
   ../
   ./
   <...list of sub-directories and files within the directory...>)"
   ```

   这是vim的插件`netrw`的功能。

3. linux上的(2)read不可以读文件夹，而有的unix的read是可以的。

   在早些年，UNIX只有一种统一的文件系统，所以目录的格式，其在磁盘上的组织格式是很统
   的，那时并没有必要去提供一种专门的系统调用来读文件夹。而到后来，文件系统百花齐放
   ，此时再让人去触碰目录的raw bytes就是很危险的事了。内核有必要负起责任，提供一种统
   一的接口，将不同文件系统的实现细节隐藏起来，就有了后来`readdir`之类的syscall。虽
   然有了统一的接口，但有的UNIX，其read函数仍然可以读取目录，这是这种UNIX的历史遗留
   问题，而linux比较晚，没有这个历史问题，所以其read不可以读文件夹。

4. `opendir(const char * name)`用来打开文件夹，其参数是一个字符串，也就是文件夹的名
   字。除了使用文件夹的名字；还可以使用文件描述符，`fdopendir(int fd)`，如果使用这
   个函数的话，就需要先使用`open`来获取文件描述符。

   > 不知道用`fdopendir`时，后面关闭文件夹还用不用`closedir`，还是仅使用`close`.

   > 在GNU/Linux的man上，`After a successful call to fdopendir(), fd is used inter-
     nally by the implementation, and should not otherwise be used by the application.`
	 不让你用，自然也就不能用close去关闭这个fd了

   > 在Open Group的man上，Upon successful return from fdopendir(), the file descriptor
     is under the control of the system, and if any attempt is made to close the file 
     descriptor, or to modify the state of the associated description, other than by means 
   	 of closedir(), readdir(), readdir_r(), rewinddir(), or seekdir(), the behavior is 
	 undefined. Upon calling closedir() the file descriptor shall be closed.
