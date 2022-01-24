##### 3.1
1. 当我们的代码中有变量定义却未被使用时，rustc会给出warning，我们可以用`#![allow(unused_variable)]`来去除warning。这个attribute很有
   意思，前面是一个shebang，然后接[]，里面写具体的属性。
   注意这个是一个shebang，也就是有!的，那么这个atrribute就是arate level的，也就是直接平息整个crate的没用到的变量

2. 给变量定义别名

   ```rust
   type alias = orig_type
   ```
   和go语言不同的是，在编译器看来，这两种类型是相同的，然而在go中给变量起别名，编译器会将其看为不同的类型。

3. 对于定义却没有使用的函数，我们也有一个属性可以平息编译器的warning
   `#[allow(dead_code)]`，但这个属性必须紧放在那个函数定义的上面，也就是两者之间除了空白行不能有别的。
   > dead_code这个lint可以用来平息任何关于unused的warning

4. `std::fmt::Debug`
   实现了这个trait的东西，可以使用`{:?}`来打印。

5. 两种特殊的类型
   1. unit type: means no value
   2. never type: usually used as a return type of a function, means this function will never ever return something,
      such as panic! or a infinite loop.


##### 3.2
1. 我们前面说道alias一个新的类型，但在编译器眼中这两种类型并没有区别，那么如何构建一个新类型，使其和原有类型在编译器眼里不一样呢
   使用具名元组，比如我们想给`String`别名成一个hostname，我们并不是只`type hostname = String`，而是定义一个结构体
   ```rust
   struct Hostname(String)
   ```
   这样，我们就可以使两者在编译器眼中不同，但用起来差不多了。在rust in action中称其为`newtype`。


2. Vec的append方法

   ```rust
   pub fn append(&mut self, other: &mut Self) {
        unsafe {
            self.append_elements(other.as_slice() as _);
            other.set_len(0);
        }
   }
   ```

   这个函数会将自己曾长，然后将被插入方的长度设为0，所以other是一个可变的引用。

3. `pub fn from_utf8_lossy(v: &[u8]) -> Cow<'_, str>` 函数
    
	这个函数的功能是从utf-8的slice中创建字符串，String是Vec<u8>，但是u8并不一定是合法的utf-8的某个字节，所以这里的函数名后缀有一个
	lossy，也就是损失的，但参数的slice中有非法的utf-8字节时，其会被替换为�。
	然后来看一下此函数的返回类型
	```rust
	pub enum Cow<'a, B> where
	B: 'a + ToOwned + ?Sized,  {
        Borrowed(&'a B),
	    Owned(<B as ToOwned>::Owned),
	}
	```
	上面提到当使用`from_utf8_lossy`时，如果有非法的utf8字节，其会被替换，也就是需要对其进行修改，那么此时返回的是具有所有权的String
	类型，如果没有非法的utf8字节，其返回的是&str。
	这个智能指针有如下方法：
	* `pub fn into_owned(self) -> <B as ToOwned>::Owned`: 可以将内部具有所有权的数据导出来
	* `pub fn to_mut(&mut self) -> &mut <B as ToOwned>::Owned`: 可以把内部具有所有权的数据的&mut指针导出来。
	* 其他的两个方法是nightly的，想要使用这种nightly的方法，必须开feature，然后使用nithglty的toolchain才可以。
	

4. `pub unsafe fn from_utf8_unchecked(bytes: Vec<u8, Global>) -> String` 
	这个方法是上面的没有检查非法utf8的版本，所以最后肯定是返回具有所有权的String类型。



##### 3.3.1
1. fields are private by default but can be accessed within the module that defines the struct.

2. `pub fn reserve(&mut self, additional: usize)`
	这个方法是用来填充self这个vec的，它会使self的vec的cap >= self.cap + additional
	> 这个函数可能会多多扩充些容量来避免之后过多的填充
	如果在调用这个方法时，self的cap已经足够了，此函数什么都不会做。

3. rust中的`self`关键字
	这个关键字有2种使用场景：
	1. 表示当前的模块module
	2. 用来method里表示调用这个method的struct实例，并且
	 * `self`: `self: Self` 交出所有权
	 * `&self`: `self: &Self`
	 * `&mut self`: `self: &mut Self`
	 > 前者是语法糖，self有其特殊含义，后者中的self只是一个普通的形参名。
	 不论在method中self以上述何种形式出现，都可以用`instance.method()`来调用，当然也可以用`Self::mthod(instance的某种形式)`
	 来调用，不过这样的话，instance的形式就要和3种中的某一种对应了。


##### 3.4.1
1. 全局 变量
   全局的变量在rust中这样声明
   ```rust
   static mut GLOBAL:type = value;
   ```

2. const和let都可以创建常量，那么有什么区别呢？
   const创建的是编译时的量，而let则是在运行时创建变量.

3. 一个在io中经常用到的专用的Result类型: `std::io::Result`
   `pub type Result<T> = Result<T, Error>;` 这是一个新的类型，后面的Error指的是`std::io::Error`这个结构体，后面的Result指的是
   `std::result::Result`，由于新定义的类型省略了Error字段，所以可以将其视为一种语法糖吧。
   A specialized Result type for I/O operations.
   This type is broadly used across std::io for any operation which may produce an error.
   This typedef is generally used to avoid writing out io::Error directly and is otherwise a direct mapping to Result.
   While usual Rust style is to import types directly, aliases of Result often are not, to make it easier to distinguish 
   between them. Result is generally assumed to be std::result::Result, and so users of this alias will generally use 
   io::Result instead of shadowing the prelude’s import of std::result::Result.
   > 正如官方所说，由于std::result::Result被prelude所引入，所以在代码中写Result就指的是std::result::Result，所以在使用io版的
   > Result时我们会明示地写`io::Result`(当use std::io;时)


##### 3.4.2
1. collect方法
   ```rust
   fn collect<B: FromIterator<Self::Item>>(self) -> B
   where
   Self: Sized,
   {
       FromIterator::from_iter(self)
   }
   ```
   这个方法用的很多，可以从一个实现了`std::iter::Iterator`trait的东西造出一个collection来，然后你看它的返回值，是一个实现了
   `std::iter::FromIterator`trait的范型，有一堆东西实现了这个trait，所以很多时候编译器不能推断出你到底要collect成一个什么，
   tuibofish或者明确给出左值类型就是必须的了。
   这个方法的一个频繁的使用场景就是从一个collection变为另一个collection。

 2. &str的`splitn`方法
    ```rust
	#[stable(feature = "rust1", since = "1.0.0")]
    #[inline]
    pub fn splitn<'a, P: Pattern<'a>>(&'a self, n: usize, pat: P) -> SplitN<'a, P> {
        SplitN(SplitNInternal { iter: self.split(pat).0, count: n })
    }
	```
	和split方法一样进行分割返回可叠代的结构体，但不一样的是，这个迭代器里面的可叠代项的数量是有上限的，也就是参数`n`给出的。
    官方给出了一些示例代码: 
    ```rust
	let v: Vec<&str> = "Mary had a little lambda".splitn(3, ' ').collect();
	assert_eq!(v, ["Mary", "had", "a little lambda"]);

	let v: Vec<&str> = "lionXXtigerXleopard".splitn(3, "X").collect();
	assert_eq!(v, ["lion", "", "tigerXleopard"]);

	let v: Vec<&str> = "abcXdef".splitn(1, 'X').collect();
	assert_eq!(v, ["abcXdef"]);

	let v: Vec<&str> = "".splitn(1, 'X').collect();
	assert_eq!(v, [""]);
	```
##### 3.6.2
1. println! print! write! writeln! format! 这些宏都依赖于debug和display这两个trait

2. format! 是什么：
   这个宏是用于字符串拼接的，它是一个语法糖，用来代替下面的代码:
   ```rust
   let s = std::fmt::format(format_args!("Hello, {}!", "world"));
   assert_eq!(s, "Hello, world!");
   ```
   有了宏之后: 
   ```rust
   let s = format!("Hello, {}!", "world"); // 注意一下它的用法
   assert_eq!(s, "Hello, world!");
   ```

3. rust的result类型
	`std::result::Result`这个enum是最重要的类型
	`std::io`这个module里在它的基础上利用它重新定义了`std::io::Result`
	`std::fmt`也在它基础上重新定义了`std::fmt::Result`
	两个重定义均是将Error变为了自己模块内的Error，然后io中未对Okk(T)的T作出改动，fmt直接将T变为了()也就是unit type.


4. 如何为你的类型实现`std::fmt::Display`trait
   > 由于std::fmt::Display的输出是面向人的，所以它不可以derive
   ```rust
   pub trait Display {
       fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), Error>; // 注意这个Result是`std::fmt::Result`
   }
   ```

   仅需要把这个fmt函数的签名写上，然后在内部调用`write!("", )`即可。 

   ```rust
   use std::fmt;

   struct Point {
       x: i32,
       y: i32,
   }

   impl fmt::Display for Point {
       fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
	       write!(f, "({}, {})", self.x, self.y) // 在write!中定义你想要的输出格式
       }	
   }

   let origin = Point { x: 0, y: 0 };

   assert_eq!(format!("The origin is: {}", origin), "The origin is: (0, 0)");
   ```


##### 3.7.1
1. enum如果是pub的，那么它的字段也会是pub，而struct不是，structpub，字段依然private，struct提供了更加精细的控制

##### 3.8.1
1. rustdoc这个命令行工具是给你的单独代码文件生成html的inline doc用的，它会在当前目录下生成doc文件夹，doc文件夹下有一个和你的代码或者
   是项目同名的文件夹，cd进去，就可以看到:
   ```shell
   ➜  ch3_file_doced l
   Permissions Links Size User  Group Date Modified Name
   .rw-rw-r--      1 3.0k steve steve 13 Dec 11:59  all.html
   .rw-rw-r--      1 4.2k steve steve 13 Dec 11:59  index.html
   .rw-rw-r--      1  115 steve steve 13 Dec 11:59  sidebar-items.js
   .rw-rw-r--      1  24k steve steve 13 Dec 11:59  struct.File.html
   ```


2. rustup doc这个命令才是从命令行打开std文档

3. rustdoc针对单个文件，类似于rustc，而当针对一个很大的项目的，可以使用cargo doc来助你一下子生成整个项目的文档。
   当使用cargo doc来生成inline doc时，文档会在`/root_of_proj/target/doc/parj_name/`中。
   > 注意使用`cargo doc`时，它不仅会为你的代码生成文档，还会给你的依赖生成文档，在一些依赖众多的项目中，这会很费事费时，如果
   > 不想给依赖生成文档，可以使用`cargo doc --no-deps`，给一个`--no-deps`的参数。













