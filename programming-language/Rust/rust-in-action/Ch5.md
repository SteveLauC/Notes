##### 5.1
1. 查看浮点数的比特模式，可以使用`to_bits()`这个方法，f32调用会产生u32，f64调用产生u64，同时也可以用unsafe直接挖开
   ```rust
   fn main() {
       let f = 8.9_f64;
	   let bits = f.to_bits();
	   let bits_2: u64 = unsafe{
	       std::mem::transmute(f)
	   };

	   println!("{}", bits);
	   println!("{}", bits_2);
   }
   ```


2. transmute函数是用来将一个值的bit重新解释成另外一种类型的，要求源和目的的大小相同即可.



3. std::fmt::Binary 的trait
    b formatting. The Binary trait should format its output as a number in binary
	当在打印时使用`{:b}`的formater时，用到的就是这个trait。

	> 如果在打印时，使用了{:#b}，在:和b之间加了一个#，那么在打印时会在二进制数字前加上"0b"


   似地，{:?}调用的是std::fmt::Debug


##### 5.2
1. rust有两种编译模式，dev mode和release mode，当我们使用`cargo build`时，我们使用的是dev mode，而当我们使用`cargo build --release`
   时，我们使用的是release mode。
   我们可能对这两个mode不是很熟悉，但编译时的信息却经常看到他们: 
   ```rust
   $ cargo build
   Finished dev [unoptimized + debuginfo] target(s) in 0.0s
   $ cargo build --release
   Finished release [optimized] target(s) in 0.0s
   ```
   注意看方括号中写的有关优化的东西，dev并没有有优化，而release是优化过的。

2. 关于rust编译的优化选项
   rust的编译优化有`0..=3`4种不同的规格，dev mode是没有优化的，优化等级是0，而release优化到满，是3。
   在使用rustc进行编译时，可以选择`-O`参数，此参数会将优化等级调到2。

   > dev和release的优化等于是可以被更改的，可以在`cargo.toml`中设置
   > ```rust
   > [profile.dev]
   > opt-level = 0
   >
   > [profile.release]
   > opt-level = 3
   > ```
 

3. 关闭rust的overflow的check
   1. 源代码中要开启这个`#[allow(airthmetic_overflow)`或者`#![allow(arithmetic_overflow)]`
   2. 在使用rustc编译时 `rustc -O target.rs`或者`rustc -C overflow_checks=off target.rs`
   3. 在使用cargo编译时，在cargo.toml中:
      ```rust
	  [profile.dev] # 这是dev mode release mode就是profile.release
	  overflow-checks = false
	  ```
4. endianness问题
   除了有byte的endianness问题，还有bit的endianess，即一个byte的bit是如何被摆放的。但是除非到具体的某一个bit操作，我们并不需要关心它
   因为计算机最小单位就是byte，内部实现是透明的。




###### 5.6
1. 给你一个byte，根据它生成一个0-1的f32浮点数
   知道了IEEE754f32的格式，我们就可以利用这个格式来做这个，一个思路是是sign bit是0，exponent的编码值是-1，mantissa中利用上这个byte
   就可。因为exponent为-1，所以最后的值一定是1.xx * 2^(-1)，也就落在0-1之间了。



##### 5.7.1
1. 使用match时如果被match的对象是个元组，然后有几个字段我们想要在case忽略它，可以使用`_`来忽略。

2. `std::todo`宏
   可以在未实现的功能时使用，但是被标记todo宏的功能就不能使用






