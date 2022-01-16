##### 9.1
1. 世界上的两种时间：
   * TAI: 让每一秒的时长都是固定的，这对于计算机来说是准确的，但对于现实生活来说不是，因为潮汐阿，地球自传等各种因素，时间是动态的。
   * UTC: 每年都会随真实的物理情况对时间进行调整，这对人很友好，但对机器来说不友好。
   > 还有一种时间是GMT，以天文的现象规定时间，没有UTC准确。

2. 计算机当中一般有2种时钟：
   * one is a battery powered device, called the real-time clock.
   * the other one is known as system time.
   > raspberry pi中并没有第一种时钟。

3. crate level的lint要放在文件的头部
   ```rust
   line0: #[alow(unused_variables)]
   ```
4. 计算机中如何表示时间？
   一般是记录UTC时间，然后还有一个整数记录用户的时区，在UTC的基础上加上时区数的偏移量，就是用户的本地时间。

5. NTP(network time protocal)是一个应用层的协议，用来在计算机间同步时间。 

6. todo!()和unimplemented!()都会返回`never type`，也就是`!`.
   那么这两个宏有什么区别呢？todo!()意如其名，表明这个功能在将来会被做掉，而unimplemented!()则没有这个含义 
   它两个在发生panic时打印出来的信息也不一样，todo是`not yet implemented`，而unimplemented是`not implemented`
   从这里也能够看出来todo有将来要做现在没有做的意思

7. ZST(zero sized type)指的是在运行时不占用内存的类型，比如没有字段的struct就是一个ZST

8. 使用chrono库时如何返回现在的时间并将其转换为UNIX timestamp
   ```rust
   #![allow(unused_variables)]

   use chrono::Local;

   fn main(){
       let now: DateTime<_> = Local::now();  // get current time
	   let timestamp: i64 = now.timestamp(); // convert current time to UNIX timestamp
   ```

10. 时间的格式
    * UNIX timestamp: 即从1970年1月1日0点0时0分的秒数
	* rfc2822: 这个格式是电子邮件使用的时间格式
	* rfc3339: 这个格式是IP协议使用的时间格式，请注意ISO8601这个标准，这两个东西是很类似的，但两种标准的格式又不完全相同。

11. rust的多行字符串，使用`/`对换行进行转义
    ```rust
	fn main() {
	    let str:&str = "abc\
		    def";
	    println!("{}", str);
	}
	// print "abcdef" to stdout
	// NOTE: there is no space before `def`, if you wanna this, add spaces beween `abc` and `\`
	```

12. 在使用`clap`库创建cli app时，如果你的参数给了默认值，那么在调用`value_of()`时就不用害怕会panic，因为一定是有值的。

13. UNIX中的syscall都是被libc提供的，libc是c的标准库，有多种实现，比如glibc就是GNU的实现。
    为什么syscall是由libc提供的，因为UNIX考虑兼容性不使用汇编后就改用c写了，天生一对。

14. 在libc中，类型的命令是蛇形命名的，类型的别名通常在最后会有一个`_t`。在rust中，由于`struct`和`trait`都是驼峰命名，所以当你想用
    libc的命令方式时需要使用lint `#![allow(non_camel_case_types)]`来开启这个功能

15. 在vim中有时按到了`ctrl+s`，vim会卡死，然而并不是，这是shell用来冻住输出的快捷键，`ctrl+q`即可消除这个
