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

	> 可以发现，rust标准库中的这个元数据是考虑了跨平台特性的，对于UNIX的特殊特性
	  ，使用trait进行了补丁。
	
	```rust
	pub trait MetadataExt {
	    fn dev(&self) -> u64;
	    fn ino(&self) -> u64;
	    fn mode(&self) -> u32;   // mode
	    fn nlink(&self) -> u64;  // 连接数
	    fn uid(&self) -> u32;    // owner
	    fn gid(&self) -> u32;    // group
	    fn rdev(&self) -> u64;
	    fn size(&self) -> u64;   // size
	    fn atime(&self) -> i64;  // atime
	    fn atime_nsec(&self) -> i64;
	    fn mtime(&self) -> i64;  // mtime
	    fn mtime_nsec(&self) -> i64;
	    fn ctime(&self) -> i64;  // ctime
	    fn ctime_nsec(&self) -> i64;
	    fn blksize(&self) -> u64;
	    fn blocks(&self) -> u64;
	}
	```
	> 这个trait，或许才是stat结构体的相对应的东西

	> `std::os::unix::fs`中的trait，已经见到3个了，`OpenoOptionExt`，`Permissio-
	  nsExt`，`MetadataExt`

13. 在rust中，如何利用Result这个结构:
    * 如果你`Ok<T>`和`Err<E>`中的T/E都要用到，那么就需要使用
	  ```rust
	  match res {
	  	  Ok(t) => {
		      // do something with t
	      },
		  Err(e) => {
		  	  // do something with e
		  }
	  }
	  ```
	* 如果只需要用到一个
	  ```rust
	  if let Ok(t) = res {
	      // do something with t
	  }
	  ```

	  ```rust
	  if let Err(e) = res {
	      // do something with e
	   }
	  ```
    * 如果什么也不用到，仅仅是查看一下这个res是ok还是err
	  ```rust
	  if res.is_ok() {
	      // do something
	  }
	  ``` 

	  ```rust
	  if res.is_err() {
	      // do something
	  }
	  ```

14. `mode_t`是`stat`结构体中`st_mode`的类型，是`unsigned int/u32`的别名，但其实
    其中只用到了16个bit，4个用来标识文件的类型，2^4可以标识16种文件类型，目前已
	经用了7种，3个是特殊位，9个用来标识权限。

	[link](https://stackoverflow.com/questions/9602685/why-does-mode-t-use-4-byte)

	但为什么要32bit呢？the reason is that using a 16-bit value here would have 
	no benefit at all. Data structures on x86 are 32-bit aligned, so wherever 
	this 16-bit value would be stored (inside a struct, on the stack, on the 
	heap, in a register), it would take up 32-bits anyway in most cases. However
	, 32-bit values might be processed more efficiently by the CPU, which is much
	more important than space these days.

	
	文件类型 <br>
	/* File types.  */
	```c
	// 注意，不同的文件类型是不可以同时出现了，所以是2^4=16种
	// 而不是4种。在判断时，不可以直接用
	// ```c
	// if (st_mode & __S_IFDIR) {
	//     printf("is a dir\n");
	// }
	// ```
	// 而应该用
	// ```c
	// if ((st_mode & __S_IFMT) == __S_IFDIR) {
	//     printf("is a dir\n");
	// }
	// ```
	```

	```c
	#define	__S_IFMT	0170000	/* These bits determine file type.  */ // 总的掩码
	```

	```c
	* #define	__S_IFDIR	0040000	/* Directory.  */           (00)0 100 
	* #define	__S_IFCHR	0020000	/* Character device.  */    (00)0 010
	* #define	__S_IFBLK	0060000	/* Block device.  */        (00)0 110
	* #define	__S_IFREG	0100000	/* Regular file.  */        (00)1 000
	* #define	__S_IFIFO	0010000	/* FIFO.  */                (00)0 001
	* #define	__S_IFLNK	0120000	/* Symbolic link.  */       (00)1 010
	* #define	__S_IFSOCK	0140000	/* Socket.  */              (00)1 100
	```

	特殊位 <br>
	// 其他的12位都是可以同时出现的，故可以直接用下面的宏进行`&`来查看权限的有无
	```c
	* #define	__S_ISUID	04000	/* Set user ID on execution.  */             100 
	* #define	__S_ISGID	02000	/* Set group ID on execution.  */            010
	* #define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */ 001 
	```

	owner的权限 <br>
	```c
	* #define	__S_IREAD	0400	/* Read by owner.  */        100 
	* #define	__S_IWRITE	0200	/* Write by owner.  */       010
	* #define	__S_IEXEC	0100	/* Execute by owner.  */     001
	```

15. 掩码的概念，在一串二进制流中，将不需要的位置0，需要的置1，这就是掩码。上面的
    这些宏都是掩码。

16. stat这个文件中除了上述的掩码宏，还有直接给你准给好的用来判断的函数宏。
    ```c
	/* Test macros for file types.	*/

	#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))

	#define	S_ISDIR(mode)	 __S_ISTYPE((mode), __S_IFDIR)
	#define	S_ISCHR(mode)	 __S_ISTYPE((mode), __S_IFCHR)
	#define	S_ISBLK(mode)	 __S_ISTYPE((mode), __S_IFBLK)
	#define	S_ISREG(mode)	 __S_ISTYPE((mode), __S_IFREG)
	#ifdef __S_IFIFO
	# define S_ISFIFO(mode)	 __S_ISTYPE((mode), __S_IFIFO)
	#endif
	#ifdef __S_IFLNK
	# define S_ISLNK(mode)	 __S_ISTYPE((mode), __S_IFLNK)
	#endif
	```

	所以在写判断一个文件是不是文件夹时，使用如下代码:
	```c
	if (S_ISDIR(mode)) {
		printf("is a dir\n");
	}
	```

17. 拿到了UID/GID，怎么拿到用户名和组名呢
    ```c
	// 通过UID就可以拿到下面的结构体
	struct passwd *getpwuid(uid_t uid);
	``` 
	```c
	struct passwd {
	    char   *pw_name;       /* username */ // 我们要的用户名
	    char   *pw_passwd;     /* user password */
	    uid_t   pw_uid;        /* user ID */
	    gid_t   pw_gid;        /* group ID */
	    char   *pw_gecos;      /* user information */
	    char   *pw_dir;        /* home directory */
	    char   *pw_shell;      /* shell program */
	};
	```

	```c
	struct group *getgrgid(gid_t gid);
	```

	```c
	struct group {ERRORS
       These functions are always successful.
失败的syscall会对errno进行赋值，而成功的syscall则不会。所以errno的值应该是上
一个失败的syscall设置的，我们在#include <errno.h>来检查errno时应先确保此syscall
确实失败了(对errno进行了赋值)

极少数的syscall可以在成功时返回-1(比如getpriority)，对于这种syscall，想要
检查其是否出错需要先将errno设置为0，然后调用它，再看errno是否为0，如果非0则是
出错了
	    char   *gr_name;        /* group name */
	    char   *gr_passwd;      /* group password */
	    gid_t   gr_gid;         /* group ID */
	    char  **gr_mem;         /* NULL-terminated array of pointers
	                               to names of group members */
	};
	```

18. `stdio.h`中的`sprintf()`函数
    可以将任何东西拼成字符串，和rust中的`format!()`有点类似。

19. set user id bit and set group id bit

    The Unix access rights flags setuid and setgid (short for "set user ID" and 
	"set group ID")[1] allow users to run an executable with the file system 
	permissions of the executable's owner or group respectively and to change 
	behaviour in directories. They are often used to allow users on a computer 
	system to run programs with temporarily elevated privileges in order to 
	perform a specific task. While the assumed user id or group id privileges 
	provided are not always elevated, at a minimum they are specific.

	The flags setuid and setgid are needed for tasks that require different 
	privileges than what the user is normally granted, such as the ability 
	to alter system files or databases to change their login password.[2] 
	Some of the tasks that require additional privileges may not immediately be 
	obvious, though, such as the ping command, which must send and listen for 
	control packets on a network interface.

	这两个bit的作用是为了给用户临时的更高的权限.如果一个程序的这个bit设置的话，
	运行它的人如果不是文件的所有人或者所属组中的人，可以临时地变成所有人或者所
	属组。

	典型用例就是`passwd`这个用来修改用户密码的程序

	```shell
	➜  /etc l|grep passwd
	-rw-r--r--   1 root root   2.8K Jan 21 12:56 passwd
	```

	存储密码的数据库只有root才可以写，那么一般用户如何修改自己的密码的，passwd程序
	的set user id就是被设置的，可以使一般用户短暂地变为root

	```shell
	➜  bin l|grep passwd
	-rwsr-xr-x  1 root root      67K Jul 15  2021 passwd
	```

	```c
	// 检测SUID是否被设置
	#include <sys/stat.h>
	#include <stdio.h>
	#include <stdlib.h>

	int main(){
	    struct stat buf;
		if (stat("/usr/bin/passwd", &buf) == -1) {
	        perror(NULL);
	        exit(-1);
		}

		if (buf.st_mode & S_ISUID) {
		    printf("SUID bit is set\n");
		}
		return 0;
	}
	```

	在linux上，确实被设置了；但在macOS上，没有被设置。macos这是什么鬼。

20. 当SUID被设置时，owner中的执行权限变为了's'；当SGID被设置时，group的执行权限变
    为了's'；当sticky bit别设置时，other的执行权限变为了't';
 ^a5a1a3
21. 文件一旦被创建，其文件类型就不可以被更改。
	比如我们先使用`creat()`创建一个reg文件，然后调用`chmod`来修改它，在其文件类型
	那4位上做一些改变。

	感觉实验不用做了，chmod只能更改permission.

	> creat的mode参数是必须提供的，The  mode  argument specifies the file mode 
      bits be applied when a new file is created.  This argument must be sup‐
      plied when O_CREAT or O_TMPFILE is specified in flags; if neither O_CREAT
	  nor O_TMPFILE is specified, then mode is ignored.
	  因为我们使用的creat函数就是open加了O_CREAT|O_WRONLY|O_TRUNC这3个flag罢了。

	```c
	#include <stdio.h>
	#include <stdlib.h>
	#include <fcntl.h>


	int main(){
    	int fd = creat("/home/steve/Desktop/test", 0040000);
		if (fd == -1) {
	        perror(NULL);
	        exit(-1);
		}
	}
	```
	`0040000`是dir，但是没有任何的权限和特殊位，结果是:
	```shell
	----------  1 steve steve    0 Jan 30 15:19 test
	```
	我们创建的是一个普通文件，并没有成功创建目录。尝试使用了各种文件类型的编码，最
	后创建的都是regualr file，看来creat只能用来创建reg file。创建其他类型的文件需要
	使用其他的syscall

	


22. 当使用`int creat(const char *pathname, mode_t mode)`来创建文件时，可以请求将
    新文件的文件权限设置为mode。**不过只是请求，而不是命令**，最后新文件的文件权限还
	要看`新建文件掩码`这个变量，从请求权限中去掉umask才是最后得到的权限

	在rust中，使用`std::fs::OpenoOption::open()`创建文件，默认的权限是`0x666`，如
	果使用了`OpenoOptionExt'中的mode函数请求设置权限，这两种操作均会和umask相减，
	得到最终的权限。

	```shell
	# Linux 上默认的啊掩码
	# 使用umask命令来查看umask
	$ umask
	002
	```

	> 在macOS上试了下，发现umask默认是022

	syscall中的`chmod(mode)`在rust中与之对应的是`std::os::unix::fs::PermissionsExt`
	中的`fn set_mode(&mut self, mode: u32)`.

23. 通过不同的系统调用创建不同类型的文件，在创建时指定3个特殊位，和9个权限位。随后
    可以通过chmod来修改这12位。

24. 通过使用`(2)umask`系统调用来修改当前进程的`umask`这个变量。有意思的是，这个函数
    不会失败。

25. The umask is used by open(2), mkdir(2), and other system calls that create
    files to modify the permissions placed on newly created files or directories.  
	Specifically, permissions in the umask are turned off from the mode argument 
	to open(2) and mkdir(2).

	> chmod不会受umask影响

26. 文件所有者，一般情况下，一个调用了creat的程序，谁运行这个程序，创建的文件的owner
	就是谁。 但当文件的SUID被set时，这个程序的所有者是谁，新建的文件的owner就是谁。

	```c
	#include <stdio.h>
	#include <stdlib.h>
	#include <fcntl.h>


	int main(){
		int fd = creat("/home/steve/Desktop/test", 0777);
		if (fd == -1) {
			perror(NULL);
			exit(-1);
		}
	}
	```
	比如这个代码，编译后的程序如果所有人是root，并且编译后的程序的SUID被设置，
	尽管是steve去运行这个程序，但新建文件的所有人却是root。

	```shell
	# SUID is set owner is root
	-rwsrwsr-x 1 root  steve  17K Jan 30 15:50 main 
	```

	```shell
	# steve运行这个程序，得到的新文件的owner是root
	-rwxrwxr-x  1 root  steve    0 Jan 30 16:36 test
	```

27. 文件所属组
    The group ownership (group ID) of the new file is set either to the effective 
	group ID of the process (System V semantics) or to the group ID of the parent 
	directory (BSD semantics).  On Linux, the behavior depends on whether the 
	set-group-ID mode bit is set on the parent directory: if that bit is set, 
	then BSD semantics apply; otherwise, System V semantics apply.  For some 
	filesystems, the behavior also depends on the bsdgroups and sysvgroups  
	mount  options described in mount(8)).

	上面这段摘自linux的`man 2 creat`

	而在macos的`man 2 open`中写道: When a new file is created, it is given the 
	group of the directory which contains it.

28. 修改文件的所有者或所属组
    使用`int chown(const char *pathname, uid_t owner, gid_t group)`来修改文件的所
	有者或者所属组。当其中的字段不想修改时，传入`-1`即可。
	
29. 修改mtime和atime
    系统调用`int utime(const char *filename, const struct utimbuf *times)`来修改。
	当times这个参数被传入`NULL`时，时间时间会被设置为当前时间。

	```c
	/* Structure describing file times.  */
	struct utimbuf
	{
	    __time_t actime;		/* Access time.  */
	    __time_t modtime;		/* Modification time.  */
	};
	```

30. 系统调用`int rename(const char *oldpath, const char *newpath)`可以用来改变文
    件名，也可以用来移动，其实就是`mv`使用的syscall。
