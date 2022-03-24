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
   cargo run -q
   ➜  t cargo run -q
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

   > 2022-3-33 [question_link](https://stackoverflow.com/questions/71580696/whats-the-difference-between-operator-and-deref-method)
