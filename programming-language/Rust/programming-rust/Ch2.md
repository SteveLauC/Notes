1. rustc用来编译代码，rustdoc用来编译文档，但是我们通过让cargo来调用他们。

   ```shell
   $ cargo b
   $ cargo doc
   ```

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

8. pub fn trim(&self) -> &str
   这个函数返回参数的substring，和C中自己控制指针移动相似，是reslice。

9. 将字符串变小写，如果仅仅是ascii的字符串，可以使用`pub fn make_ascii_lowercase
   (&mut self)`，这个可以不需要额外的空间分配，是在原内存处直接修改，将小写字母变为
   大写字幕。如果是UNICODE的小写变大写，由于:

   > ‘Lowercase’ is defined according to the terms of the Unicode Derived Core 
   Property Lowercase. Since some characters can expand into multiple characters 
   when changing the case, this function returns a String instead of modifying 
   the parameter in-place.

   所以需要使用额外的空间，其函数签名是这样的`pub fn to_lowercase(&self) -> String`
   在堆上进行了额外的内存分配。

10. 字符串的寻找

    str提供了`find`和`rfind`两种方法，可以返回子串在母串的最左和最右的第一个字节
    的索引值.

    ```rust
    pub fn rfind<'a, P>(&'a self, pat: P) -> Option<usize>
    where
        P: Pattern<'a>,
        <P as Pattern<'a>>::Searcher: ReverseSearcher<'a>,
    ```

    ```rust
    pub fn find<'a, P>(&'a self, pat: P) -> Option<usize>
    where
        P: Pattern<'a>,
    ```

    or you can use `position/rposition` exposed by `Iterator`:
    
    ```rust
    fn main() {
        let str = "hello";
        let left_find_res = str.chars().position(|item| item == 'l');
        let right_find_res = str.chars().rposition(|item| item == 'l');
    
        println!("{:?} {:?}", left_find_res, right_find_res);
    }
    ```

    ```shell
    $ cargo b
    error[E0277]: the trait bound `Chars<'_>: ExactSizeIterator` is not satisfied
        --> src/main.rs:4:38
         |
    4    |     let right_find_res = str.chars().rposition(|item| item == 'l');
         |                                      ^^^^^^^^^ the trait `ExactSizeIterator` is not implemented for `Chars<'_>`
         |
         = help: the following other types implement trait `ExactSizeIterator`:
                   &mut I
                   Args
                   ArgsOs
                   ArrayChunks<'_, T, N>
                   ArrayChunksMut<'_, T, N>
                   ArrayWindows<'_, T, N>
                   Box<I, A>
                   Chunks<'_, T>
                 and 109 others
    note: required by a bound in `rposition`
        --> /home/steve/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/src/rust/library/core/src/iter/traits/iterator.rs:2879:23
         |
    2879 |         Self: Sized + ExactSizeIterator + DoubleEndedIterator,
         |                       ^^^^^^^^^^^^^^^^^ required by this bound in `rposition`
    
    For more information about this error, try `rustc --explain E0277`.
    error: could not compile `rust` due to previous error
    ```

    `position` can be used but `rposition` needs the iterator to be 
    `ExactSizeIterator` and `DoubleEndedIterator`

    ```rust
    fn rposition<P>(&mut self, predicate: P) -> Option<usize>
    where
        P: FnMut(Self::Item) -> bool,
        Self: ExactSizeIterator + DoubleEndedIterator, 
    ```


11. 当我们在`cargo.toml`中指明版本是`1`时，cargo会自动帮我们选出在`2`之前的最新版本
    因为`2`的主版本号变动了，所以crate作者可以引入不兼容的API。

    semantic versioning

12. 在终端实现彩色的输出，有一个`text-colorizer`的crate可以帮忙做到。

13. `std::debug_assert!()` just like `assert`, but will only be evaluated in 
    development build.
