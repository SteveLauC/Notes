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

5. 读目录和读文件是比较像的，但不一样的是，文件夹的底层是结构化的数据，所以读的是一个又
   一个的结构体，和编写who命令时读取utmp文件更加相似。

6. linux的文件系统的元数据
   ```c
   struct dirent {
       ino_t          d_ino;       /* Inode number */
       off_t          d_off;       /* Not an offset; see below */
       unsigned short d_reclen;    /* Length of this record */ 
       unsigned char  d_type;      /* Type of file; not supported
                                      by all filesystem types */ // 这家伙居然不是被所有的文件系统支持
       char           d_name[256]; /* Null-terminated filename */
   };
   ```
7. 使用`readdir`读文件项不需要自己准备buffer
   ```c
   // list files in directory called filename
   void do_ls(char dirname[]) {
       DIR * dir_ptr = NULL;
       struct dirent * dirent_ptr = NULL;

	   if ((dir_ptr = opendir(dirname)) != NULL) {
		   while ((dirent_ptr=readdir(dir_ptr)) != NULL) {
		       printf("%s\n", dirent_ptr->d_name);
		   }
           closedir(dir_ptr);
	   }
   }
   ```

   > `readdir`返回的就是指向dirent结构体的指针，而且你不能去free这个指针。

8. rust std中读取文件夹中的项的操作，默认是不会去打印 `.` 和 `..`。
   
9. 文件夹的大小是以1024字节为单位的。

10. 编写`ls -l`: 
    需要的东西: mode link-number owner group size mtime filename  <br>
	其中，我们可以使用系统调用`(2)stat`来拿到所有的这些东西。

	```c
    struct stat {                                                                                                                                          
		dev_t     st_dev;         /* ID of device containing file */                                                                                       
		ino_t     st_ino;         /* Inode number */                                                                                                       
		mode_t    st_mode;        /* File type and mode */                                                                                                 
		nlink_t   st_nlink;       /* Number of hard links */                                                                                               
		uid_t     st_uid;         /* User ID of owner */                                                                                                   
		gid_t     st_gid;         /* Group ID of owner */                                                                                                  
		dev_t     st_rdev;        /* Device ID (if special file) */
		off_t     st_size;        /* Total size, in bytes */
		blksize_t st_blksize;     /* Block size for filesystem I/O */ 
		blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

		                  /* Since Linux 2.6, the kernel supports nanosecond                                                                                                    
						  precision for the following timestamp fields.                                                                                                      
						  For the details before Linux 2.6, see NOTES. */                                                                                                                                                                                                                                                    
    	struct timespec st_atim;  /* Time of last access */                                                                                                
		struct timespec st_mtim;  /* Time of last modification */                                                                                          
		struct timespec st_ctim;  /* Time of last status change */                                                                                                                                                                                                                                        
		#define st_atime st_atim.tv_sec      /* Backward compatibility */                                                                                  
		#define st_mtime st_mtim.tv_sec                                                                                                                    
		#define st_ctime st_ctim.tv_sec                                                                                                                    
	}; 
	```

11. `stat`函数的使用方法：
    ```c
	int stat(const char *pathname, struct stat *statbuf);
	```
	传入一个路径，以及事先准备好的buffer。

12. 在rust中，关于文件的元数据需要使用`std::fs::metadata()`拿到`std::fs::Metadata`
	来获取。`Metadata`这个结构体有一些方法用来或许元数据: 
	* pub fn accessed(&self) -> Result<SystemTime> 返回atime
	* pub fn modified(&self) -> Result<SystemTime> 返回mtime
	* pub fn created(&self) -> Result<SystemTime>  返回btime/birthtime 文件的创建时间
	* pub fn file_type(&self) -> FileType  用来判断文件的类型，但是返回的FileType结构
	  体目前也要调用函数来判断具体的文件类型，而且只能判断dir/file/symbollink
	* pub fn is_dir(&self) -> bool 判断是不是dir
	* pub fn is_file(&self) -> bool判断是不是常规的文件，排除dir/软连接
	* pub fn len(&self) -> u64 返回文件的大小
	* pub fn permissions(&self) -> Permissions 返回一个Permission的结构体，但这个结构
	  体的方法，只有`readonly`和`setreadonly`，对于UNIX来说用处不大。但是，这个结构体
	  实现了`std::os::unix::fs::PermissionsExt`这个trait
	  ```rust
	  pub trait PermissionsExt {
	      fn mode(&self) -> u32;           // 这是我们想要的
	      fn set_mode(&mut self, mode: u32);
	      fn from_mode(mode: u32) -> Self;
	  }
	  ```

	  > 可以发现，rust标准库中的这个元数据是考虑了跨平台特性的，对于UNIX的特殊特性，使
	    用trait进行了补丁。
	

