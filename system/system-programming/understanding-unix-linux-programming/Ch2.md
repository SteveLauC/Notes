1. 编写who命令时，使用`utmp`这个结构体，需要从`/var/run/utmp`这个文件中读取此结构体，需要注意的是，此文件是二进制文件，需要反序列化。
   在c中，直接使用`open`拿到文件描述符，然后使用read读此文件到buffer中即可。
   ```c
   // 此段代码没有进行错误处理
   int fd = open("/var/run/utmp", O_RDONLY);
   struct utmp buf;
   read(fd, &buf, sizeof(struct utmp));
   close(fd);
   ```

   而在rust中，如果不想使用libc提供的系统调用，可以这样写:
   ```rust
   fn main() {
        let mut utmp_file: File = File::open("/var/run/utmp").unwrap();
        let struct_size: usize = mem::size_of::<utmpx>();                                       
	    unsafe {               
        	let mut current_record: utmpx = mem::zeroed();// 先把内存准备好
        	let buffer: &mut [u8] =                       // 因为read_exact需要的是u8的slice所以我们准备slice
            slice::from_raw_parts_mut(&mut current_record as *mut utmpx as *mut u8, struct_size);
			while utmp_file.read_exact(buffer).is_ok() {
            	show_info(&current_record);               // 将utmp结构体的信息打印出来
			}   
		}   
   }

   ```
 
2. 想要将`[c_char; x]`这种c风格的转变为rust的字符串的话，使用:
   ```rust
	use std::ffi::CStr;                                                                                                                                                                 
	use libc::c_char;

	fn main() {
	    let c_str: [c_char;6] = [104, 101, 108, 108, 111, 0];
		unsafe{
		    let str: &str = CStr::from_ptr(c_str.as_ptr()).to_str().unwrap(); // 注意这里我们使用`CStr`而不是`CString`因为我们不想拿走其所有权
		    println!("{}", str);
		}
	}
   ```
  
3. `/var/run/utmp`文件中有各种各样的内容，不止是用户登录信息，所以我们在编写who命令时需要对拿到的结构体进行过滤
   ```c
   struct utmp
   {
       short int ut_type;		/* Type of login.  */  // i16 
	   // other fields are omitted here
   }
   ``` 

   而在`utmp.h`这个文件中有一个宏，`USER_PROCESS(7)`标识用户的登录。

4. 记录下`timeval`这个结构体
   ```rust
   #[repr(C)]
   pub struct timeval {
       pub tv_sec: time_t,
       pub tv_usec: suseconds_t,
   }
   ```
   其中的`time_t`和`suseconds_t`类型都是`i64`，在c中，就是`long int`.
   而到了`utmp`结构体中记录登录时间的字段`ut_tv`时，使用的是`__timeval`
   ```rust
   pub struct __timeval {
       pub tv_sec: i32,
       pub tv_usec: i32,
   }
   ```
   其中的两个字段居然变成了`i32`，由long退变为了int，难道是觉得一定会在这个小时间范围内登录吗

5. 在c中将字符串右trim的方式
   ```c
   char *trim(char *s) {
       char *ptr;
       if (!s)
           return NULL;   // handle NULL string
       if (!*s || strlen(s)==0)
           return s;      // handle empty string
       for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
	       ptr[1] = '\0';
       return s;
   }
   ```
6. 在c中将从`UNIX_EPOCH`开始的秒数转变为人类可读的格式，使用`time.h`中的ctime函数
   ```c
   char *ctime(const time_t *timep);
   ```
   这个函数需要time_t的指针，也就是i64的指针，然后直接返回字符串。需要注意的是，他返回的字符串右侧自带换行，如果想要trim掉，使用上面的trim函数

7. 在rust中做`6`中的这件事可以使用`chrono`这个crate:
   ```rust
   fn show_time(seconds: i32) {
       let date: DateTime<Local> =
       DateTime::from(UNIX_EPOCH + Duration::from_secs(u64::try_from(seconds).unwrap()));
       let date_str: String = date.format("%b %e %H:%M:%S %Y").to_string();
       print!("{:<12}", date_str);
   }
   ```
8. 在rust中(1.58)使用`std::fs::File::create()`来创建文件，其权限默认是664.

9. posix里的`creat(2)`函数其实是`open(2)`的语法糖:
   是open("path", OWRONLY|O_CREAT|O_TRUNC, mode)的封装。

10. 如何在rust中判断给定的一个路径是文件还是文件夹:
    ````rust
	use std::fs;

	fs::metadata(p).unwrap().is_dir()
	fs::metadata(p).unwrap().is_file()
	````
	以上这两个函数都是返回bool的。

11. 有时在模拟c的连续内存buffer时，可以使用这个内存存放的类型的数组，栈上还更快一些
    在进行IO，比如使用`std::io::Read/Write`这两个trait中的函数，需要用到slice，可以
	使用`use std::slice::from_raw_parts;`这个函数来创建slice，这个函数还用兄弟版本
	可以，创建mut的slice。
	```rust
	pub unsafe fn from_raw_parts<'a, T>(data: *const T, len: usize) -> &'a [T]
	pub unsafe fn from_raw_parts_mut<'a, T>(data: *mut T, len: usize) -> &'a mut [T]
	```
	一般用的slice都是字节(u8)的slice，在指针那个形参进行cast就可以。
	> 写了这么多IO的代码了，如果想读一个东西，模式就是先要准备好内存，然后准备slice
	  slice更像是内存的代理。

12. 内核缓冲技术，内核和外设进行IO操作，为了提高效率，内核在内核态也使用了内核缓冲区。
    记得我大致看过一个文章，一个用户写的内容，是流经多级缓冲区，才到外设上的。
    有用户程序的缓冲，内核缓冲，然后才到外设，貌似是这样的，不太记得了啊哈哈。

13. 在GNU的`man 2 lseek`中的Notes这部分有这样一段话
    If the O_APPEND file status flag is set on the open file description, 
	then a write(2) always moves the file offset to the end of the file, 
	regardless of the use of lseek().
	在rust中写代码验证一下: 
    ```rust
	use std::fs::{File, OpenOptions};                                                           
	use std::io::Seek;
    use std::io::SeekFrom;
    use std::io::Write;

    fn main() {
        let mut f: File = OpenOptions::new()
            .append(true)
            .open("/Users/steve/Documents/workspace/Proj/rs_proj/t/src/main.rs")
            .unwrap();
        let init_off: u64 = f.seek(SeekFrom::Current(0)).unwrap();
        println!("{:?}", init_off);  // 0 刚打开文件offset是0
        f.write(b"hello");
        let after_off: u64 = f.seek(SeekFrom::Current(0)).unwrap();
        println!("{:?}", after_off); // 642 因为有append的标记，所以每次write都会seek到末尾
                                     // 然后再写，来到新的末尾
        f.seek(SeekFrom::Start(0));
        f.write(b"hello");
        let after_off: u64 = f.seek(SeekFrom::Current(0)).unwrap();
        println!("{:?}", after_off); // 647
	}
	```

14. c语言的错误处理，是靠errno这个全局变量的，`number of last number`。当syscall
    或者一些库出错时，会修改这个变量的值，如果想要使用这个变量，需要`#include 
	<errno.h>`
	在Linux上，有一个命令行工具，就叫`errno`(sudo apt install errno)，然后使用
	`errno -l`就可以查看不同的值所代表的不同意思。

	```c
	#include <stdio.h>
	#include <errno.h>
	#include <fcntl.h>

	int main(){
	    int fd;
	    fd = open("/home/steve/Desktop/sc", O_WRONLY);

		if (fd == -1){
	   	    printf("can not open file: \n");

			if (errno == ENOENT) {
	            printf("There is no such file\n");
			}

			if (errno == EINTR) {
			    printf("Interrupted\n");
			}
					        
    		if (errno == EACCES) {
	            printf("do not have permission\n");
			}
		}
	}
	```

	perror函数，这个函数就是一个封装好的函数，`void perror(const char *s)`
	如果s不是NULL，或者*s不是'\0'那么，s首先被打印，然后是一个冒号，然后一
	个空格，然后是根据errno值的描述信息。所以可以大概猜一下里面的实现。
	
	```c
	void perror(const char *s) {
		if (s || *s != '\0') {
			printf("%s: ", s);
		}
		switch (errno) {
			case EPERM:  
				printf("Operation not permitted\n");
				break;	
			case ...  :
				... 
		}
	}
	```

