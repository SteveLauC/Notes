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
