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
