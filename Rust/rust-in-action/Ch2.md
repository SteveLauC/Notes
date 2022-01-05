##### 2.1

1. 如果使用cargo run或者cargo build时想要仔细看清楚cargo如何使用rustc来驱动，可以使用 `cargo run -v ` 或者 `cargo run -vv` 来将详细信息打印出来。当然如果你想看清编译的详细过程，需要清cache，也就是target下已经编译好的文件。 `vv` 比 `v` 打印得更加详细
。

##### 2.3.1

1. 浮点数有很多的方法，比如：

   * 找到和它最接近的整数: round()
   * 返回整数部分： trunc()
   * 返回小数部分：fract()

2. pow方法 是指数运算

   ```rust
   fn main() {
       let base: i32 = 2;
       println!("{}", base.pow(5_u32));
   }
   ```

   所有的整数类型均有此方法，返回值和 `Self` 相同，参数是 `u32` 。也就是说i32的实例调用这个方法返回i32.

3. powf方法，这个是上面方法的float-point number版。

##### 2.3.2

1. rust可以创建不同进制的字面量。

   ```rust
   fn main() {
       let octal_literal = 0o7;
       println!("{:o}", octal_literal);
   }
   ```

   字面量的合法与否，lang server应该是可以分析出来的，貌似找到了一个bug，`0o8` 在rust-analyzer中可以报错，在idea里就不可以。

   也可以对数在打印时设置打印的进制

   ```rust
   fn main() {
       let x = 10;
       println!("{}", x);  // 10
       println!("{:b}", x);// 1010
       println!("{:o}", x);// 12
       println!("{:x}", x);// a
       println!("{:X}", x);// A
   }
   
   // 注意16进制打印有两种格式，字母部分以大写还是小写的格式打印的区别
   ```


##### 2.3.3

1. `std::cmp::PartialOrd` 和 `std::cmp::PartialEq`

   这两个trait是用来实现值的比较的，Ord代表着可以判断是否大于、小于、大于等于、小于等于，Eq代表着判断是否相等。其中 `PartialEq` 是 `PartialOrd` 的基础，因为在ord里要判断大于等于和小于等于，所以想要实现 `std::cmp::PartialOrd` 先实现 `std::cmp::PartialEq` 。

   ```rust
   // std::cmp::Ordering
   pub enum Ordering {
       Less,
       Equal,
       Greater,
   }
   ```

   ```Rust
   pub trait PartialEq<Rhs = Self> 
   where
       Rhs: ?Sized, 
   {
       // 必须实现的方法 想等
       fn eq(&self, other: &Rhs) -> bool;
   	
       // 调用eq实现 判断不相等
       fn ne(&self, other: &Rhs) -> bool { ... }
   }
   ```

   ```Rust
   pub trait PartialOrd<Rhs = Self>: PartialEq<Rhs> 
   where
       Rhs: ?Sized, 
   {
       // 必须实现的方法
       fn partial_cmp(&self, other: &Rhs) -> Option<Ordering>;
   
       // 以下4个方法均是对partial_cmp的调用
       // less than
       fn lt(&self, other: &Rhs) -> bool { ... }
       // less than or equal
       fn le(&self, other: &Rhs) -> bool { ... }
       // greater than
       fn gt(&self, other: &Rhs) -> bool { ... }
       // greater than or equal
       fn ge(&self, other: &Rhs) -> bool { ... }
   }
   ```

   还有两个trait，`std::cmp::Ord` 和 `std::cmp::Eq` ，这两个是在PartialXX基础上实现的。

   `PartialXX` 和 `XX` 的区别，官方文档说是 Partial是： Trait for values that can be compared for a sort-order.，而XX是：	Trait for types that form a [total order](https://en.wikipedia.org/wiki/Total_order). 关于这个貌似是数学里面的概念，我还没有找到通俗易懂的东西，但仅从目前来看，如果一个东西仅实现了PartialXX，那么两个这个东西可以进行比较，但一个这个东西的数组，确不能直接调用迭代器的max找出最大值。

   ```rust
   use std::cmp::{Ordering, PartialEq};
   use std::cmp::PartialOrd;
   
   #[derive(Default, Debug)]
   struct Person{
       age: u32,
   }
   
   
   impl PartialEq for Person{
       fn eq(&self, other: &Self) -> bool {
           self.age==other.age
       }
   }
   
   impl PartialOrd for Person{
       fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
           self.age.partial_cmp(&other.age)
       }
   }
   
    It re-exports the
   
   fn main() {
       let mut v = Vec::new();
       for _ in 0..10{
           v.push(Person::default());
       }
       println!("{:?}", v.into_iter().max());
   }
   ```

2. rust不允许不同类型的变量/常数进行操作，此时可以使用 `as` 来进行强制类型转换，但从更长的数据类型转到更短的数据类型有时是很危险的，有时会发生溢出，我们可以使用 `std::convert::TryInto` 这个trait中的 `try_into` 来进行转换。

   ```rust
   pub trait TryInto<T> {
       type Error;
       fn try_into(self) -> Result<T, Self::Error>;
   }
   ```

   ```rust
   use std::convert::TryInto;
   
   fn main() {
       let a: i32 = 10;
       let b: u16 = 100;
   
       // 一般情况下我们需要对转换的目标类型进行手动指定
       // 但这里由于 a 和 b_ 进行了比较 所以b_的类型被推倒出来了
       let b_ = b.try_into().unwrap();
   
       if a < b_ {
           println!("10 < 100");
       }
   }
   ```

3. rust trait中的方法，只有当这个trait被use来引入local scope时才可以使用。但有的trait的使用频率太高，手动use太累，所以rust搞出了 `std::prelude` 这个东西

   > prelude n. 序曲，前奏曲; 开场戏，序幕; 前兆，预兆
   
   > Rust comes with a variety of things in its standard library. However, if you had to manually import every single thing that you used, it would be very verbose. But importing a lot of things that a program never uses isn’t good either. A balance needs to be struck.
   >
   > The *prelude* is the list of things that Rust automatically imports into every Rust program. It’s kept as small as possible, and is focused on things, particularly traits, which are used in almost every single Rust program.
   
   除了 `std::prelude` 外，还有其他的一些prelude模块，比如rust标准库中还有 `std::io::prelude` ，rand库中有 `rand::prelude` ，这些模块虽然叫 `prelude` ，但他们都需要**手动**地引入，而 `std::prelude` 是**自动**引入的。
   
4. f32和f64只实现了`std::cmp::PartiaEq` 而没有实现 `std::cmp::Eq` ，因为 This trait allows for partial equality, for types that do not have a full equivalence relation. For example, in floating point numbers `NaN != NaN`, so floating point types implement `PartialEq` but not [`Eq`](https://doc.rust-lang.org/std/cmp/trait.Eq.html).

5. f32和f64的 `to_bits()` 方法

   这个方法是方便程序员查看浮点数的内存布局的，返回结果是无符号的 和浮点数等宽的数，比如f32调用返回u32，f64调用返回u64。内部是cpu架构的顺序的二进制数，将次数以二进制打印出来就可以得到内存布局。

   ```rust
   // 自己实现的方法
   fn mem_contents(f: f64){
       let mut p = &f as * const f64 as *const u8;
       let mut v: Vec<u8> = Vec::with_capacity(8);
       for _ in 0..8{
           unsafe {
               v.push(*p);
               p=p.offset(1);
           }
       }
       // 要以小端序的数序打印出来
       for item in v.iter().rev(){
           print!("{:08b}", item);
       }
   }
   
   
   fn main() {
       let f: f64 = 0.1;
       let f_in_bits: u64 = f.to_bits();
       // 64位以0填充 就是内存布局了
       println!("{:064b}", f_in_bits);
       mem_contents(f);
   }
   
   // 结果
   0011111110111001100110011001100110011001100110011001100110011010
   0011111110111001100110011001100110011001100110011001100110011010
   ```

6. 关于浮点数比较大小，浮点数在计算机内是按二进制形式存在的，以二进制表示十进制的后果就是精度不够，那么在通常数学上成立的东西在计算机里就不一定成立，比如比较两个浮点数相等，`0.1+0.2==0.3` 这个在数学上成立的东西在计算机里就不一定成立

   ```rust
   fn main() {
       let result: f32 = 0.1;
       let desired: f32 = 0.3 - 0.2;
   
       let abs_diff = (result - desired).abs();
       println!("{}", abs_diff);
   }
   
   // 结果
   0.000000007450581
   ```

   所以在计算机里定义浮点数之间的相等，作出了一点妥协，如果两个浮点数的差值足够小，我们就认为这两个数相等，那么怎么定义足够小，我们认为：如果两个浮点数的差值小于一个**小数**，那么这两个浮点数就足够小，这个小数我们叫它为 `EPSILON` ，是希腊字母中的第5个字母，在rust中定义了 `f32::EPSILON` 和 `f64::EPSILON` 两个小数。

   ```rust
   pub const EPSILON: f32 = 1.19209290e-07_f32;
   pub const EPSILON: f64 = 2.2204460492503131e-16_f64;
   ```

   ```rust
   fn main() {
       let result: f32 = 0.1;
       let desired: f32 = 0.3 - 0.2;
   
       if (desired-result).abs() < f32::EPSILON {
           println!("eq");
       }else{
           println!("ne");
       }
   }
   // 结果
   eq
   ```
   
7. NaN
   
   nan存在于浮点数中，比如负的浮点数调用sqrt方法返回的就是nan。
   
   ```rust
   fn main() {
       let f: f64 = -4.0;
       println!("{}", f.sqrt());
   }
   // 结果
   NaN
   ```
   
   NaN永远不等于NaN
   
   ```rust
   fn main() {
       let f: f64 = (-4.0_f64).sqrt();
   
       if f==f{
           println!("eq");
       }else{
           println!("ne");
       }
   }
   ```
   
   除了NaN还有infinite，比如除数是0.0的情况
   
   ```rust
   fn main() {
       let f = 4.0/0.0;
       println!("{}", f);
   }
   // 结果
   inf
   ```
   

##### 2.3.4

1. rust的标准库相对来说是比较少东西的，很多的数的类型它都没有提供，因此我们需要使用rust的一个crate：num

   ```rust
   use num;
   
   fn main() {
       let a = num::complex::Complex{re: 2.1, im: -1.2};
       let b = num::complex::Complex::new(11.1,22.2);
       let result = a+b;
       println!("{} {}", result.re, result.im);
   }
   ```

##### 2.4.1

1. 在rust中遍历一个collection时不推荐使用像c语言那样的使用index的写法

   ```rust
   fn main() {
       let collections = [1, 2, 3];
       for i in 0..collections.len() {
           println!("{}", collections[i]);
       }
   }
   ```

   原因如下：

   1. 性能：使用索引值，会增加rust的运行时开销，rust会最bound checking。

      而使用迭代器，rust会在编译时做好检查。

   2. 安全性：使用索引时，同时，有其他的东西可以对colection进行修改，而使用 `into_iter` `iter` `iter_mut` 则可以确保在遍历的过程中，collection不会被修改。

##### 2.4.3

1. 在rust中如何让一个线程sleep

   ```rust
   use std::time::{Duration, Instant};
   use std::thread;
   
   fn main() {
       // 这里开始计时
       let now = Instant::now();
   	// Duration::new的第一个参数是秒，第二个参数是纳秒
       thread::sleep(Duration::new(5, 0));
   	// 结束计时
       let span = now.elapsed();
       println!("{:?}", span);
   }
   ```

2. 使用while循环来测试电脑的性能

   ```rust
   use std::time::{Duration, Instant};                
   
   fn main() {
       let mut count = 0;
   	   let time_limit = Duration::new(1,0);            
	   let start = Instant::now();                     
   	   // 给count如果+1后 还没有过1s就继续加一
   	   while (Instant::now() - start) < time_limit {
	       count += 1;
   	   }
   	   println!("{}", count);
   }
   ```


#### 2.4.6

1. 在rust中，大部分语句都是 `expression` ，是可以返回值的。少部分是 `statements`，没有返回值。rust中的 `statements` 只要有3类：

   1. 被 `;` 所结束的 `expression` 
   2. 赋值语句 `let a = 1` ，这样杜绝了c语言中的那种一行多个赋值，也杜绝了在逻辑表达式中将 `==` 写成 `=` 从而造成错误。
   3. 声明函数或者结构体枚举体。

   > 第一种被称为 `expression statement` ，而剩下的2种叫做 `declaration statement` 。



##### 2.4.7

1. rust的match

   ```rust
   fn main() {
       let number = 13;
   
       println!("Tell me about {}", number);
       match number {
           // Match a single value
           1 => println!("One!"),
           // Match several values
           2 | 3 | 5 | 7 | 11 | 13=> println!("This is a prime"),
           // TODO ^ Try adding 13 to the list of prime values
           // Match an inclusive range
           13..=19 => println!("A teen"),
           // Handle the rest of cases
           _ => println!("Ain't special"),
           // TODO ^ Try commenting out this catch-all arm
       }
   }
   ```

   这是官方给的例子，rust中的match支持匹配多个值，使用 `|` 分割，并且支持 `inclusive range` `start..=end` ，但不支持 `exclusive range start..end` 。

   不同的rust分支间可以重复，编译器并没有足够的能力分辨是否重复，比如第2条match包含13，第3条match也包含13，它只会进入第2条语句。

##### 2.9

1. 在rust中如果想将一个很长的字符串写到多行，需要使用 `\` 来将换行给转义掉

   ```rust
   fn main() {
       let str = "a\
       b\
           c\
           d";
       println!("{}", str);
   }
   
   // 结果
   abcd
   ```

   仔细看上面的打印结果，你会发现b前并没有空格，c/d前也没有空格，想要在空格，只能加在上一个 `\` 之前。

2. rust的raw string

   raw string的raw在，它没有任何的转义功能，字符串里面是什么，就是什么。

   ```rust
   fn main() {
       let str = r#"line1
       line2 with a tab space"#;
       println!("{}", str);
   }
   // 结果
   line1
       line2 with a tab space
   ```

3.  `str` 的 `contains` 方法

   ```rust
   pub fn contains<'a, P: Pattern<'a>>(&'a self, pat: P) -> bool {
           pat.is_contained_in(self)
   }
   ```

   Self是一个 `&str` ，而pat是一个实现了 `std::str::pattern::Pattern` 的范型，实现这个trait的类型，就可以用来在str中进行搜索。

   rust已经为：

   1. &str
   2. char
   3. &String
   4. &[char]
   5. &&str
   6. F: [FnMut](../../ops/trait.FnMut.html)([char](../../primitive.char.html)) -> [bool](../../primitive.bool.html)

   实现了这个trait.

4. [u8]是字节的slice，通常在处理info stream时遇到。

5. std::ffi::OSString和String的区别就是在OSString中，rust不保证其：

   1. 一定是utf-8编码的
   2. 一定没有0字节

6.  `enumerate` 方法

   ```rust
   #[inline]
   #[stable(feature = "rust1", since = "1.0.0")]
   fn enumerate(self) -> Enumerate<Self>
   where Self: Sized,
   {
   	Enumerate::new(self)
   }
   ```

   这个方法它不是属于某一个类型的，而是在 `std::iter::Iterator` 这个trait里面的，它消耗一个迭代器(实现了这个trait的类型)，然后返回一个 `std::iter::Enumerate` 结构体，这个结构体也是一个迭代器。

7. 在rust中，函数参数和返回值都需要是 `Sized` ，也就是说，需要在编译时知道其具体的大小，因为要处理栈帧。当给你一个具体的类型，自然是可以说它是sized或者unsized的，但到了范型，则就不好说了。

   ```rust
   fn foo<T>(x: T)->T{
       x
   }
   ```

   在上面这个代码中，我们并没有指定T是sized，但也可以编译成功，原因在于rust中的范型默认是Sized的，也就是实现了 `std::marker::Sized` 这个trait。起初并没有给T这个默认东西，后来人们加trait bound烦了，就加上了。

8.  `grep` 命令的 `-C NUM ` 参数，除了可以将包含待匹配串的那一行打印出来，还可以打印 `NUM` *2行被匹配行的上下文信息，也就是说，将被匹配行上下的NUM行打印出来

   ```shell
   ➜  ~ l|grep -C 3 DataGripProjects
   .rw-rw-r--      1  51k steve steve 25 Nov 08:27  .zcompdump-steve-MacBookPro-5.8
   .rw-------      1 233k steve steve  9 Dec 15:03  .zsh_history
   .rw-rw-r--      1 5.1k steve steve  9 Dec 12:19  .zshrc
   drwxrwxr-x      3    - steve steve  7 Sep 10:33  DataGripProjects/
   drwxr-xr-x      2    - steve steve  9 Dec 10:42  Desktop/
   drwxr-xr-x     10    - steve steve  5 Dec 19:35  Documents/
   drwxr-xr-x      6    - steve steve  8 Dec 20:48  Downloads/
   ```



##### 2.10.3

1. 防止溢出的减法

   ```rust
   pub const fn saturating_sub(self, rhs: i16) -> i16
   ```

   这个函数的它可以确保你不会出现运行时overflow的panic，当你的运行达到了这种类型的边界值后，还想越过边界值，那么它会直接将其卡住在边界值。

   ```rust
   fn main() {
       let zero: u32 = 0;
       assert_eq!(0_u32, zero.saturating_sub(1));
   }
   
   // 不会panic
   ```

   类似的方法给不同的类型的数均有实现，而且加减乘除均有实现。只不过有的是nightly
   
2. 类似上面的方法，还有一个 `overflowing_sub` 方法，这个方法返回两个值，一个是数，另一个是布尔值，有点像go的风格的错误处理，这个布尔值为真，表明发生溢出，为假，表明没有发生溢出。

   ```rust
   fn main() {
       let zero: u32 = 0;
       let (res, err) = zero.overflowing_sub(1);
       if !err {
           println!("{}", res);
       }
   }
   // 结果
   什么也不打印
   ``


##### 2.11.1
1. `regex` 库，给rust提供正则表达式的支持
    use regex::Regex;

```rust
fn main(){
    let re = Regex::new(" p").unwrap();

    let quote = "Every face, every shop, bedroom window, public-house, and
dark square is a picture feverishly turned--in search of what?
It is the same with books. What do we seek through millions of pages?";

    for line in quote.lines() {
        let contains_substring = re.find(line);
        match contains_substring {
            Some(_)=>println!("{}", line),
            None => ()
        }
    }

}
```
   在上面的代码中，我们首先创建一个正则表达时，使用`Regex::new("")`来创建，然后判断一个字符串中是否有这个正则
   表达式的匹配存在，使用 `re.find(string)` 来查找，结果返回的是一个 `Option` ，如果其为Some，则存在，否则不存在。


##### 2.11.2
1. 为第三方的crate生成本地的doc文档，通常情况下第三方crate的文档都是需要在线获取的，但假如我们将这个crate下载下来，我们
   就可以在本地为其生成文档，仅需使用 `cargo doc` 命令即可，然后就可以在 `target/doc` 目录下找到这个文档了。

   
##### 2.13
1. 关于 `std::io::BufReader` : 这个是在io的时候给我们提供缓冲，也就是buffered io

   创建这个结构体的方法叫做
   ```rust
   #[stable(feature = "rust1", since = "1.0.0")]
   pub fn new(inner: R) -> BufReader<R> {
       BufReader::with_capacity(DEFAULT_BUF_SIZE, inner)
   }
   ```
   这个方法接受一个范型参数 `R` ，其有一个 `trait bound` 要求必须实现 `std::io::Read` 



2. `buffered io` 读文件的大致模式

   首先要拿到`File`，然后将其放入bufferreader里面，然后调用bufferreader的方法来读，比如调用它的read_line
   ```rust
   let f = File::open("/home/steve/Desktop/file").unwrap();
   let mut reader = BufReader::new(f);
   let mut line = String::new();
	
   loop {
       // read_line函数返回Option<usize> usize为所读的字节数
       let len: usize = reader.read_line(&mut line).unwrap();
       if len==0 {
           // 说明文件已经读完，已经到达了eof
           break;
       }
       println!("{} {}", line.trim(), len);
       line.clear();
   }
   ```

   但是手动的取这个每一行是比较费力的，所以rust提供了语法糖，`BufReader` 实现了 `BufRead` 的trait，这个trait里面有一个`lines`方法，可以返回
   `Lines`结构体，这个结构体可以迭代
   > 这个结构体我们之前遇到过，str有lines方法将字符串按照换行符号分割，返回这个结构体
   
   ```rust
   let f = File::open("/home/steve/Desktop/file").unwrap();
   let mut reader = BufReader::new(f);

   for line in reader.lines(){
       println!("{}", line.unwrap());
   }
   ```
   这个语法糖还有一个好处就是它会自动帮你对每一行的最后的换行符进行trim

3. clap的使用技巧

   如果你的命令行应用需要多个参数，而且并没有使用`short/long`给定名字，那么在传入参数时，参数的顺序应该是和定义时的顺序一致的，这个顺序还
   可以通过app的help参数查看


###### 2.14
1. 文件和stdin的共同抽象
   
   `T: BufRead+Sized`就可以标识这两个东西，然后调用`BufRead`trait提供的方法就可以读了

   ```rust
   fn process_lines<T: BufRead + Sized>(reader: T, re: Regex) {
       for line in reader.lines(){
           let line = line.unwrap();
           if let Some(_) = re.find(line.as_str()){
               println!("{}", line);
           }
       }
   }
   ```
  > 需要注意stdin并没有实现`BufRead`的trait，将它`.lock()`拿到`StdinLock`就可以了。
  



