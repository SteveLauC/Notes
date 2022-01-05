##### 7.2
1. 配置文件多是文本格式，因为这东西既需要被机器阅读，也需要被人阅读。


##### 7.2.1
1. 什么是序列化serialization
   序列化是为了方便传输，多指的是网络传输或者存储到磁盘，将一个对象变为一种可以存储的形式的过程。存储形式可以是多种多样的。
 
2. rust中给引入的第三方库起别名，使用as关键字 `use xx as XX`


##### 7.3
1. 在rust的普通字符串中，如果你想让一个字符串跨越多个行，你需要使用转义的`\n`。然后在rust的raw字符串中，并不需要这个，字符串是什么样
   它就是什么样。
   raw string syntax: r#"string literal"#


2. 再看看pattern这个trait
   > The pattern can be a char, a slice of chars, or a function or closure that determines if a character matches
   这个trait可以用来函数或闭包上，是什么样子的闭包或函数呢，可以决定一个char是否满足条件的函数或者闭包，也就是说大概要
   `fn(c: char)->bool`的函数，比如
   ```rust
   pub fn is_numeric(self) -> bool {
       match self {
          '0'..='9' => true,
           c => c > '\x7f' && unicode::N(c),
       }
   }
   ```
   这个用来判断字符是否为数字的函数

3. 如果想把string liternal变为字节的slice，`&[u8]`，可以在引号前加上b
   ```rust
   let bytes_slice: &[u8] = b"hello world";
   // 这个bytes的constants只能是ascii字符的
   ```

4. rust的宏的语法`DelimTokenTree`可以是`()` `[]` 或 `{}`，所以macro可以:

   ```rust
   macro!();
   macro![];
   macro!{};
   
   ```
   选择哪一种并不影响代码的执行，但对于类似函数的宏，如`println`，人们更偏好`()`；而对于`vec!`这种声明块的变量的宏，人们更偏好`[]`。


5. slice的chunks方法
   ```rust
   pub fn chunks(&self, chunk_size: usize) -> Chunks<'_, T> {
       assert_ne!(chunk_size, 0);
	   Chunks::new(self, chunk_size)
   }
   ```
   这个方法是将slice再分块的方法，比如一个slice有4个元素，然后你`slice.chunks(2)`，那么就会得到一个`Chunks`结构体，这家伙可迭代，每
   个迭代的元素都是3大小的slice，这样就将大slice给划分为了小slice。需要注意的是，&str并不能使用这个方法，大概是因为它叫string slice
   而官方要的仅仅是slice吧...


   ```rust
   fn main(){
       let str = "111122223333";
	   for item in str.chars().collect::<Vec<char>>().chunks(4){
	       println!("{:?}", item.into_iter().collect::<String>());
	   }
   }
   // 打印结果
   1111
   2222
   3333
   ```
6. 关于打印rust的16进制数，如果想要字母那部分是大写，formatter用`{:X}`；小写则用`{:x}`；如果想自带`0x`前缀，则用`{:#x}`。
   如果有其他的要求的话，比如占多少位，用什么补齐，则`{:#08x}`。放在#和x之间。


7. `?`在rust中的应用
   其主要是用在函数内部的`Result`类型后面，作用是:
   1. 如果Result是Ok，那么Ok里面的会直接返回，直接剥开。
   2. 如果是Err，那么函数直接返回。


   ```rust
   fn foo()->std::result::Result<(), std::io::Error>{
       let mut f = std::fs::File::open("/path/to/file")?;
	   let mut buf = vec![];

	   f.read_to_end(&mut buf)?;

	   Ok(())
   }
   ```
   来看上面的代码，我最初不懂，open函数和read_to_end函数的返回值不同，怎么可以在同一个函数中使用，open返回`std::result::Result<File,
   std::io::Error`，而read_to_end返回`std::result::Result<usize, std::io::Error>`，而我们的函数返回`std::result::Result<(), std::io
   ::Error>`，这怎么可以？
   看完`?`的作用，我明白了，被加了`?`的函数如果返回的是`Ok(T)`，那么就直接把值给交出去了，只有当其返回Err，才会作为值传为函数，所以
   只要控制这些函数的`Err(T)`里面的`T`相同就可以了，而我们设定的函数的Ok的成功类型，只要我们手动地如倒数第2行所写的，去满足它就可以了
   。


8. 迭代器的`nth`函数，可以返回迭代器的第n个元素，注意它是从0开始计算下标的。还有需要注意的是，它会修改原来的迭代器，消耗掉`0..=n`个元
   素，如果你调用`iter.nth(n)`的话。
   ```rust
   fn nth(&mut self, n: usize) -> Option<Self::Item> {
       self.advance_by(n).ok()?;
	   self.next()
   }
   ```

9. 想要在rust当中读文件只读一行，我们需要`std::io::BufRead::read_line()`函数，此函数`BufRead`trait独占，如果『文件』想要使用它，只
   有将其放到`std::io::BufReader`这个结构体中，或者调用`Read`trait中的take函数变为Take结构体。
   ```rust
   let file = std::fs::File::open("/path/to/file").unwarp();
   let mut buf_reader = std::io::BufReader::new(file);
   let mut buf = String::new();
   while let Ok(bytes_read) = buf_reader.read_line(&mut buf) {
       if bytes_read == 0 {
	       break;
       }
	   // do some operations to the line contents
	   // ...
   }
   ```

10. `std::io::Read::read_exact()`方法填不满buffer的情况
    在官方文档中，如果填不满buffer，此函数会返回error，unexpected error，failed to fill whole buffer.
	文档中说此时buffer中的内容是unspicified，但是我验证了下buffer仍然读到了一些东西，官方应该是不建议去依赖出现error的结果。



##### 7.4.1
1. `std::fs::File::open()`和`std::fs::File::create()`函数的区别
    前者是为了读，后者是为了写(直接覆盖之前的内容)，如果文件不存在，则会直接创建。

2. 当一个文件使用`std::fs::Openoption`以追加的方式打开一个文件的话，再调用`std::io::Write`这个trait里的方法，就是去追加了。



##### 7.4.2
1. `std::path::Path`和`std::path::PathBuf`
    rust的标准库为什么要引入这样的类型，本来路径用字符串表示就蛮够了，这样可以增加类型安全，还可以专门给路径引入一些功能，还可以跨平
	台。PathBuf的底层是OsString，Path的底层是OsStr，也就是说，不保证他们是合法的utf-8值。
	> 但我目前不是很懂它的跨平台在那里
	

2. PathBuf可以使用一个from方法，这个方法不是结构体的，而是`std::convert::From`这个trait里面的。



##### 7.6.1
1. rust的条件编译
   想要在rust中使用条件编译，需要在要被条件编译的代码块前加上`#[cfg(target_xx="")]`。
   > 注意这里的`cfg`不是一个宏
   > 需要注意在这里并没有`!=`这种运算，只有一个类似函数的语法`not()`。
   在编译时，也可以使用`--cfg ATTRIBUTE`来进行设置，`cargo`和`rustc`都可以



##### 7.7.1
1. 在流中移动cursor，就像c语言中`fseek`一样。在rust中专门有一个trait，`std::io::Seek`
   ```rust
   pub trait Seek {
       fn seek(&mut self, pos: SeekFrom) -> Result<u64>;

       fn rewind(&mut self) -> Result<()> { ... }
       fn stream_len(&mut self) -> Result<u64> { ... }
       fn stream_position(&mut self) -> Result<u64> { ... }
   }
   ```
   注意看`seek()`方法中的`pos`参数的类型`SeekFrom`，这是一个枚举体`std::io::SeekFrom`
   ```rust
   pub enum SeekFrom {
       Start(u64),
	   End(i64),
	   Current(i64),
   }
   ```
   * Start代表游标的位值为: 0 + u64的参数
   * End: 流的大小(流的末尾) + i64的参数  注意这里是i64，可以给负数的
   * Current: 游标的当前位置 + i64的参数 

   > 如何返回当前游标的位值，c语言里也是这样做的，调用`seek(SeekFrom(0))`就可以了

2. 介绍一个结构体`std::io::Take`，这个结构体需要使用`std::io::Read`trait中的`fn take(self, limit: u64) -> Take<Self>`方法得到，作用
   是使实现了`std::io::Read`trait的东西最多被读`limit`个字节，从而限制可以读的字节数，使用take产生的Take结构体，在被读完limit这些个
   字节后，再去读它会返回Ok(0)，也就是加装达到了EOF。Take结构体是可以被读的，因为它实现了`std::io::Read`的trait。
   但需要注意的是take函数的第一个参数`self`，其会拿走被调用物体的所有权，如果是File这种move语句的东西，那么它被读一个，然后就不能再
   使用了。
   > 刚看到了一个技巧，如果是File之类的东西，避免所有权被夺走，可以使用引用，因为take函数的self指的是任何实现了read的类型，&Read也实
   > 现了这个东西，比如此时我们可以调用`std::io::Read`里的`by_ref()`函数，才创建一个引用。

   ```rust
   use std::fs::{OpenOptions};
   use std::io::{Read};


   fn main(){
        let mut file = OpenOptions::new().read(true).open("/home/steve/Desktop/abc").unwrap();

	    let mut buf: Vec<u8> = Vec::new();
	    while let Ok(bytes_count) = file.by_ref().take(5).read_to_end(&mut buf) {
	        if bytes_count==0{
	            break;
	        }
	        println!("{:?}", buf);
	        buf.clear();
	    }
   }
   ```

3. `std::vec::Vec`的split_off函数
   ```rust
   pub fn split_off(&mut self, at: usize) -> Vec<T, A>ⓘ where
       A: Clone,
   ```
   功能是将一个Vec切分为两个，原来的Vec保留着[0,at)；新来的Vec拿走了[at, len).
   > 如果说at>len，则会panic
   > 如果at==len，那么新的Vec将为空，没有意义。

4. `std::debug_assert_eq!()`这个宏
   是用来断言两个东西是否相同的，但和`assert_eq`不同的是，它只有在没有开启优化时才有用
   > Unlike assert_eq!, debug_assert_eq! statements are only enabled in non optimized builds by default. 
   试了下，发现`cargo run`会触发，`cargo run --release`不会触发
   然后又分别在`Cargo.toml`中`[profile.dev]`和`[profile.release]`中修改`opt-level`为0/1/2/3
   结果是:
   * 在`cargo run`时，无论[profile.dev] opt-level设为几，都会触发
   * 在`cargo run --release`时，无论[profile.release] opt-level设为几，都不会触发

5. 详细分辨`std::io::Error`的类别，可以调用其`kind()`方法，返回`std::io::ErrorKind`枚举体
   ```rust
   [non_exhaustive]
   pub enum ErrorKind {
       NotFound,
	   PermissionDenied,
	   ConnectionRefused,
	   ConnectionReset,
	   HostUnreachable,
	   NetworkUnreachable,
	   ...
   }
   ```

   > 没有看到别的Error类型有这个kind函数，貌似只有std::io::Error有

6. 在文件中我们可以使用seek函数来调整光标的位置，但在内存里的东西，并不可以，比如Vec<u8>就没有实现`std::io::Seek`这个trait。所以我们
   有了`std::io::Cursor`，这个struct是一个wrapper，可以将任何实现了`AsRef<[u8]>`的类型包一下，然后Cursor实现了`Seek`的trait，所以就
   可以给内存中的东西移动光标了。
   ```rust
   use std::io::{Cursor, Read, SeekFrom, Seek};

   fn main() {
       let v: Vec<u8> = vec![1,2,3,4,5];
       let mut cursor = Cursor::new(v);

       let cur_cursor = cursor.seek(SeekFrom::Current(0)).unwrap();
       if cur_cursor == 0 {
           println!("move the cursor");
           cursor.seek(SeekFrom::Current(4));
       }
       let mut buf: Vec<u8>= Vec::new();
       cursor.read_to_end(&mut buf);
       println!("{:?}", buf);                // [5]
   }
   ```

7. byteorder这个crate，是用来在写的时候决定大小端序的，以哪种顺序写入多字节的数据，同样也可以用来创建二进制文件，就像调用c的write函数
   一样，这个库的用法也很简单
   ```rust
   use std::fs::OpenOptions;

   use byteorder::{LittleEndian};
   use byteorder::{ReadBytesExt, WriteBytesExt};


   fn main() {
       let mut file = OpenOptions::new()
	        .read(true)
	        .write(true)
	        .create(true)
	        .open("/home/steve/Desktop/test").unwrap();


       let n = 1;
	   file.write_i32::<LittleEndian>(n).unwrap();
   }
   ```
   只需要引入你要使用的大小端序，然后引用读和写的两个主要的trait就可以了
   ```rust
   pub trait ReadBytesExt: io::Read
   pub trait WriteBytesExt: io::Write
   ```
   首先这里有一个trait继承，必须先实现标准库里的`Read/Write`
   ```rust
   impl<W: Write + ?Sized> WriteBytesExt for W
   // All types that implement Write get methods defined in WriteBytesExt for free.
   impl<R: Read + ?Sized> ReadBytesExt for R
   // All types that implement Read get methods defined in ReadBytesExt for free.
   ```
   然后它做了这个实现，任何实现了标准库里的`Read`的东西都可以用`ReadBytesExt`
   任何实现了`Write`的东西都可以用`WriteBytesExt`


8. hashmap的insert方法
   Inserts a key-value pair into the map.
   If the map did not have this key present, None is returned.
   If the map did have this key present, the value is updated, and the old value is returned. The key is not updated, though; this 
   matters for types that can be == without being identical. See the module-level documentation for more.
   ```rust
    pub fn insert(&mut self, k: K, v: V) -> Option<V> {
	     self.base.insert(k, v)
	}
   ```

   函数的返回值是Option<V>，V是value的范型，当被插入的key之前不存在与hashmap中，None被返回；如果key之前存在，那么Some(old_value)被返
   回，old_value被更新为new_valuel
   ```rust
   use std::collections::HashMap;

   fn main() {
       let mut hm = HashMap::new();
	   hm.insert(1,1);

	   if let Some(old_val) = hm.insert(1,2) {
	       println!("这个key之前已经存在，old_val: {}现在其value已经被更新", old_val);
	       let new_val = hm.get(&1);
	       println!("new_val: {}", new_val.unwrap());
	   }
   }
   ```

##### 7.7.8

1. HashMap的快捷语法，Hashmap一直以来都没有像Vec那样的快捷的初始化语法，但在1.56后，它引入了。
   ```rust
   let v = vec![...];
   ```
   ```rust
   use std::collections::HashMap;
   fn main(){
       let hm = HashMap::from([("key", "value"), ("key2", "value2")]);
	   println!("{:?}", hm);
   }
   ```
   这是实现了From的trait搞过来的from方法
   ```rust
   impl<K, V, const N: usize> From<[(K, V); N]> for HashMap<K, V, RandomState> where
       K: Eq + Hash, 
   ```

2. hashmap的`[&key]`语法糖，可以看作get.unwrap的简写，如果key不存在的话，直接panic.
   这个`[]`这种索引的方式，是`std::ops::Index`这个trait里的`index()`方法。
   ```rust
   use std::collections::HashMap;
   use std::ops::Index;
   fn main() {
       let hm = HashMap::from([(1, 2)]);
	   println!("{:?}", hm.index(&1)); // 和hm[&1]一样
   }
   ```
 


