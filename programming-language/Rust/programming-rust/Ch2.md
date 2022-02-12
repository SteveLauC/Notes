1. rustc用来编译代码，rustdoc用来编译文档，但是我们通过让cargo来调用他们。

2. unlike c and cpp, in which assertions can be skipped, Rust always checks
   assertions regardless of the how the programm was compiled. There is also
   a `debug_assert!` macro, whose assertions are skipped when the programm is
   compiled for speed(release mode)

3. 迭代器有一个用于跳过前几个元素的方法

   ```rust
   fn skip(self, n: usize) -> Skip<Self>
   ```

   注意看他是直接拿走前面的迭代器的所有权，然后返回一个新的迭代器。
   这个在处理命令行参数时感觉会挺好用的，直接去掉二进制的名字，比`next`优雅一点

4. `std::str::FromStr`这个trait是用来从字符串解析类型的，其内有函数`from_str()`
    
   ```rust
   fn from_str(s: &str) -> Result<Self, Self::Err>
   ```

5. 对于这种trait中的方法，可以使用方法的路径来调用它，也可以用实现了他的类型的路径
   来调用
   
   ```
   use std::str::FromStr;
   fn main() {
       let n: i32 = FromStr::from_str("1").unwrap();
       let n2: i32 = i32::from_str("1").unwrap();     // 用实现了他的类型的路径来调用
   }
   ```

   这样就仿佛，那个trait的方法就是类型自己impl的方法了，但是缺点就是让人误以为这不是个
   单独的trait。

6. `std::io::Lines`这个struct也是用来一行一行地处理内容的，但是是`std::io`里面的，用来
   处理文件而不是字符串。

   可以调用`std::io::BufRead`里面的`lines()`函数来构造这个结构体。

7. 基本上每一个collection类型的module中都有一个`IntoIter`结构体，是具有所有权的迭代器。
   当这个collection调用`into_iter(self)`时，就会返回这个结构体。

   所以当写自己的库的时候，某个类型可以变为迭代器，最好就单独创建这样一个类型，但感觉没
   办法定义自己的类型，只能是std中类型的wrapper。

   创建迭代器还有的两个方法，`iter`和`iter_mut`，有的类型就是用的slice中的`iter`方法和
   `slice`中的`Iter`结构体(迭代器)，而有的类型就是自己的module中的`iter`和`Iter`(类型于
   `IntoIter`).

