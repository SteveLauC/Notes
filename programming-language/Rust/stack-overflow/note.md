1. `*`这个运算符号所对应的trait是`std::ops::Deref`或者`std::ops::DerefMut`。
    
   ```rust
   pub trait Deref {
       type Target: ?Sized;
       fn deref(&self) -> &Self::Target;
           
   }
   ```

   `*x`的操作其实是`*Deref::deref(&x)`的语法糖

   但这个操作会把Box这个东西给move掉，示例代码:

   ```rust
   0 fn main() {
   1     let x: Box<String> = Box::new(String::new());
   2     let _y: String = *x;
   3     println!("{}", x);
   4 }
   ```

   ```shell
   $ cargo run -q
   error[E0382]: borrow of moved value: `x`
    --> src/main.rs:4:20
      |
    2 |     let _y: String = *x
      |                      -- value moved here
    3 |     println!("{}", x);
      |                    ^ value borrowed here after move
      |
            = note: move occurs because `*x` has type `String`, which does not implement the `Copy` trait
            = note: this error originates in the macro `$crate::format_args_nl` (in Nightly builds, run with -Z macro-backtrace for more info)

            For more information about this error, try `rustc --explain E0382`.
            error: could not compile `t` due to previous error
   ```

   这个是历史的产物，可以看下这个[回答](https://stackoverflow.com/questions/42264041/how-do-i-get-an-owned-value-out-of-a-box)
   其实很奇怪，应该给这个语义单独加一个trait，不然`deref(&self)`怎么会move掉参数呢？

   > 2022-3-24 [question_link](https://stackoverflow.com/questions/71580696/whats-the-difference-between-operator-and-deref-method)

2. 函数的参数，`&[T]`与`&[T;N]`的区别。

   传slice更加的泛化，支持更多的实参类型`&Vec<T>``&[T;N]`之类的，任何可以创建
   `&[T]`这种slice的都可以传。数组的类型是包含其长度的，所以传数组的引用的话，
   实参就只能传入此种类型及长度的数组了。但传数组引用的好处在于可以让编译器在
   编译时拿到更多的信息，做更多的优化。

   > 2022-3-24 [question_link](https://stackoverflow.com/questions/71586633/what-is-the-difference-between-taking-a-slice-and-taking-a-reference-to-an-array/71586719#71586719)


3. 可以使用`std::iter::zip`来将两个迭代器合为一个，甚至没有要求两个迭代器中的元素
   数量相等。

   ```rust
   use std::iter::zip;

   fn main() {
       let x: Vec<i32> = vec![1,2,3];
       let y: Vec<bool> = vec![true, false];

       let res: Vec<(i32, bool)> = zip(x, y).collect();
       println!("{:?}", res);
   }
   ```

   ```shell
   $ cargo run -q
   [(1, true), (2, false)]
   ```


   > 2022-3-24 [question_link](https://stackoverflow.com/questions/71594073/filter-a-vector-using-a-boolean-array/71594147#71594147)

4. redirect stdout to vim
  
   `ls | vim -`

   redirect stderr to vim, redirect stderr to stdout first, and then use pipe to give it to vim.

   `ls 2>&1 | vim -`

   > 2022-3-24 [question_link](https://stackoverflow.com/questions/2342826/how-can-i-pipe-stderr-and-not-stdout)

5. rust的参数和返回值都是要求在编译时确定大小的，也就是要满足这个`std::marker::
   Sized`的隐性要求，但假如你的参数不想这样子，可以使用`?Sized`来去除这个限制。

   > All type parameters have an implicit bound of Sized. The special syntax 
   ?Sized can be used to remove this bound if it’s not appropriate.

6. 原来`map`函数需要的闭包参数可以直接传一个函数指针的
  
   ```rust
   pub fn format(basic_line: &str) -> Vec<String> {
       basic_line
           .trim()
           .split_whitespace()
           .map(str::to_lowercase) // 注意这的用法
           .collect()
   } 
   ```

   所以当我们把`&str`的Vec转为`String`的Vec可以这样写:  
   

   ```rust
   fn main() {
       let v: Vec<&str> = vec!["hello"]; // 会move掉v
       let owned: Vec<String> = v.into_iter().map(str::to_owned).collect();

       println!("{:?}", owned);
   }
   ```

   想不move掉v的话:

   ```rust
   fn main() {
       let v: Vec<&str> = vec!["hello"];
       let owned: Vec<String> = v.iter().map(|&item|{ // 使用`&来匹配掉一个&`
               item.to_owned()
                   
       }).collect();

       println!("{:?}", owned);
       println!("{:?}", v);
   }
   ```
   > 2022-3-26 [question_link](https://stackoverflow.com/questions/71615447/how-to-split-line-in-rust)

7. 写一个函数，既可以接受`String`类型的参数，又可以要`&str`的参数，可以使用范型+trait bound

   ```rust
   fn foo<T: Into<String>(_: T){

   }
   ```

   > 甚至这个trait bound还可以传`char`的参数。

   > 2022-3-26 [question_link](https://stackoverflow.com/questions/71613464/is-it-possible-to-create-a-function-that-accepts-optionstring-or-optionstr)

8. 将rust中`Ipv4Addr`转换为`u32`: 
   
   ```rust
   // 由于Ipv4Addr实现了`From<u32>`的trait，所以我们可以使用它反过来的trait，`Into`

   use std::net::Ipv4Addr;
   fn main() {
       let add: Ipv4Addr = Ipv4Addr::new(127, 0, 0, 1);
       let num: u32 = add.into();
   }
   ```

   > 2022-3-27 [question_link](https://stackoverflow.com/questions/71632582/how-to-convert-an-ip-to-a-i32-and-back-in-rust)

9. rust中的大数，使用数组实现的。是`num`这个crate旗下的crate，`[num-bigint](https://crates.io/crates/num-bigint)`

   > 2022-3-27 [question_link](https://stackoverflow.com/questions/71630159/how-can-i-convert-u32-datatype-to-bigint-in-rust)

10. 一个`&str`，如何将它拆分为第一个`char`和余下的`&str`
   
   ```rust
   fn split_first_char(s: &str) -> Option<(char, &str)> {
       let mut chars = s.chars();
       chars.next().map(|c| (c, chars.as_str()))
   }
   ```

   注意它调用的`map`不是对迭代器map，而是对`next()`产生的`Option<char>`map，将
   `Option<char>`变为`Option<char, &str>`.

   真的优雅这个实现。

   > 2022-3-27 [question_link](https://stackoverflow.com/questions/71628761/how-to-split-a-string-into-the-first-character-and-the-rest)
