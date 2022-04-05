1. `*`这个运算符号所对应的trait是`std::ops::Deref`或者`std::ops::DerefMut`。
    
   ```rust
   pub trait Deref {
       type Target: ?Sized;
       fn deref(&self) -> &Self::Target;
           
   
   }
`````

   `*x`的操作其实是`*Deref::deref(&x)`的语法糖

   但这个操作会把Box这个东西给move掉，示例代码:

   ```rust
   0 fn main() {
   1     let x: Box<String> = Box::new(String::new());
   2     let _y: String = *x;
   3     println!("{}", x);
   4 
   }
`````

   ```shell
   $ cargo run -q
   error[E0382]: borrow of moved value: ```x`
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
`````

   ```shell
   $ cargo run -q
   [(1, true), (2, false)]
   `````


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
`````

   所以当我们把`&str`的Vec转为`String`的Vec可以这样写:  
   

   ```rust
   fn main() {
       let v: Vec<&str> = vec!["hello"]; // 会move掉v
       let owned: Vec<String> = v.into_iter().map(str::to_owned).collect();

       println!("{:?}", owned);
   
   }
`````

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
`````
   > 2022-3-26 [question_link](https://stackoverflow.com/questions/71615447/how-to-split-line-in-rust)

7. 写一个函数，既可以接受`String`类型的参数，又可以要`&str`的参数，可以使用范型+trait bound

   ```rust
   fn foo<T: Into<String>(_: T){

   
   }
`````

   > 甚至这个trait bound还可以传`char`的参数。

   > 2022-3-26 [question_link](https://stackoverflow.com/questions/71613464/is-it-possible-to-create-a-function-that-accepts-optionstring-or-optionstr)

8. 将rust中`Ipv4Addr`转换为`u32`: 
   
   ```rust
   // 由于Ipv4Addr实现了```From<u32>`的trait，所以我们可以使用它反过来的trait，`Into`

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
`````

    注意它调用的`map`不是对迭代器map，而是对`next()`产生的`Option<char>`map，将
    `Option<char>`变为`Option<char, &str>`.

    真的优雅这个实现。

    > 2022-3-27 [question_link](https://stackoverflow.com/questions/71628761/how-to-split-a-string-into-the-first-character-and-the-rest)

11. rust的`const generics`

    generics除了可以用作范型参数，还可以表示常量，最显著的一个用法，就是你想给任
    何长度的数组实现一个方法，但数组的长度是算在数组的类型里面的，以前做这个就只
    能给所有长度的数组都实现一遍，但现在不需要了，因为可以把数组的大小用`const
    generics`来表示。

    ```rust
    use std::fmt::Display;

    fn main() {
        let arr: [String; 1] = ["hello".to_owned()];
        print_any_len_arr(&arr);
    
    }

fn print_any_len_arr<T: Display, const N: usize>(arr: &[T; N]) {
    for item in arr.into_iter() {
            println!("{}", item);
        
    }
    
}
`````   

    忘了在哪个版本的rust中数组的`into_iter()`函数稳定了，这个的稳定就归功于`const generics`

    > 2022-3-28 [question_link](https://stackoverflow.com/questions/71640770/is-this-the-most-efficient-way-to-append-an-array-to-a-slice) 

12. 数组的方法，`copy_from_slice()`

    ```rust
    pub fn copy_from_slice(&mut self, src: &[T])
    where T: Copy, 
    `````
    
    > 如果你要用的`T`不copy，使用`clone_from_slice()`

    Copies all elements from src into self, using a memcpy. The length of src 
    must be the same as self.   

    ```rust
    fn main() {
        let mut arr: [u8;3] = [0;3];
        let brr: [u8;3] = [1;3];
        arr.copy_from_slice(&brr);
        println!("{:?}", arr); // [1, 1, 1]
        arr[0..1].copy_from_slice(&[99]);
        println!("{:?}", arr); // [99, 1, 1]
    
    }
`````

    可以对数组整体赋值，也可以slice一个数组再赋值，slice的大小是在编译时不清楚
    的，可以通过编译大概是参数包含了长度信息，又要求`self`和`src`的打小相同才可
    以的吧。

    > 2022-3-28 [question_link](https://stackoverflow.com/questions/71640770/is-this-the-most-efficient-way-to-append-an-array-to-a-slice) 


13. rust中的`union`类型，是为了兼容c的abi才设计出来的。rust中常用的`enum`就是基于
    tagged union搞出来的，使用rust中的union需要注意
    
    1. 其字段的类型不可以是要被drop的类型
    2. 访问union实例的字段是unsafe的

    比如我们之前写过的用`union`来判断机器的大小端序的代码，可以这样在rust中写:

    ```rust
    union E {
        s: u8,
        i: u32,
    
    }

    
fn main() {
        let u: E = E { i: (0x12345678)  };
        unsafe{println!("0x{:x}", u.s)};  // 0x78
        println!("{}", std::mem::size_of_val(&u)); // 4
    
}
`````

    还有就是`union`可以使用pattern matching，但是和`enum`不同的是，它可以match任
    何一个字段，就像上面的代码一样，即使初始化时使用的是`i`，但仍可以拿到`s`。

    > 2022-3-29 [question_link](https://stackoverflow.com/questions/71653011/rust-is-an-enum-a-subset-of-structs)


14. `&str`和`&Stirng`是可以比较的，因为

     ```rust
     impl<'a, 'b> PartialEq<&'a str> for String
     ```

     > 2022-3-29

15. 突然意识到，在rust中，`ReRef`和`&`这两个操作并不总是相反的，对称的。

    ```rust
    let v: Vec<i32> = vec![1, 2, 3];
    let slice: &[i32] = &*v;
    `````

    比如上面这段代码，解引用再引用就变成了slice。

    > 2022-3-29


16. rust的method lookup 
 
    简单概括就是，对于`T`类型的实例`x`，`x.foo()`这个函数调用会去寻找`foo`这个函数
    。寻找的过程是:  
    
    ```
    当x可以被解引用时，逐层地解引用(*x/**x/***x...)：
    1. 每一次解引用得到的类型设为U，看看`impl U/impl trait for U`中有没`foo`这个
    函数，如果有则调用
    2. 对U进行一次引用，得到`&U/&mut U`，然后查看`impl &U/impl &mut U/impl trait 
    for &U/impl trait for &mut U`中有没有`foo`这个函数，如果有则调用

    > 如果在impl和trait中都存在`foo`这个函数，impl中的会被先调用。
    ```

    > 2022-3-29 [question_link](https://stackoverflow.com/questions/28519997/what-are-rusts-exact-auto-dereferencing-rules)

17. 有一个RFC想要enum的variabts成为first-class citizen, 但是这个RFC被推迟了

    > [rfc](https://github.com/rust-lang/rfcs/pull/1450)

    > 2022-3-30 [question_link](https://stackoverflow.com/questions/71669082/is-it-possible-to-have-a-struct-where-the-field-is-an-item-from-an-enum-rust)


18. rust中的bit-wise copy
   
   ```rust
   pub unsafe fn read<T>(src: *const T) -> T
   `````

   > 2022-3-31 [question_link](https://stackoverflow.com/questions/71681279/why-does-value-allocated-in-stack-didnt-result-in-double-free-pointer)

19. 有一个trait用来帮你的类型变成`String`
   
    ```rust
    pub trait ToString {
        fn to_string(&self) -> String;
    
    }

    > A trait for converting a value to a String.

    This trait is automatically implemented for any type which implements the 
    Display trait. As such, ToString shouldn’t be implemented directly: Display 
    should be implemented instead, and you get the ToString implementation for free.

    这个trait不应该被实现给你的类型，如果想要使用这个类型，则应该使用```std::fmt::
    Display`
    ```

    > 2022-3-31 [question_link](https://stackoverflow.com/questions/71678232/best-way-to-create-a-hashmap-from-a-slice-of-string-slices)

20. `Itertools`这个crate貌似在对迭代器进行排序上非常好用
   
    > 2022-3-31 [question_link](https://stackoverflow.com/questions/71440867/how-to-sort-json-in-rust)

21. 要实现一个方法，这个方法要给迭代器用，函数原型可以是 
   
    ```rust
    fn foo<T: Iterator>(item: T)
    ````` 

    但是

    ```rust
    fn foo<T: IntoIter>(item: T)
    `````

    更好，比如传Vec，可以传Vec本身，也可以传Vec.into_iter()，通用性更好，而且实
    现了`Iterator`的类型就实现了`IntoIter`，这是一个blanket implementation


23. 将`Option<Result<T, E>>`转换为`Option<T>`，可以这样做

    ```rust
    fn main() {
        let nothing: Option<Result<i32, String>> = None;
        assert_eq!(None, foo(nothing));
        let okk: Option<Result<i32, String>> = Some(Ok(1));
        assert_eq!(Some(1), foo(okk));
        let err: Option<Result<i32, String>> = Some(Err("error info".into()));
        assert_eq!(None, foo(err));
    
    }

fn foo(t: Option<Result<i32, String>> ) -> Option<i32> {
        t.and_then(|x| x.ok())
    
} 
`````

    但其实我看到的问题，要求是在`Some<Err<E>>`的情况下返回Err，所以他想要的是将
    `Option<Result<T, E>>`变为`Result<Option<T>, E>`

    可以使用`pub fn transpose(self) -> Result<Option<T>, E>`，这个函数是有两个的，
    也就是两种类型可以互换

    > 2022-4-2 [question_link](https://stackoverflow.com/questions/71709494/convert-optionresultt-e-to-optiont)


24. rust中的`mut`不是关于值的，而是关于`binding`的
  
    ```rust
    let v: String = "hello".to_owned();
    `````
    当像上面这样申请一个变量时(创建一个binding时)，是在说不可以通过`v`来改变这个
    字符串，但当这个字符串被给了别的binding，就要看新的`binding`是不是可以被改变
    了。

    ```rust
    fn foo(mut str: String) {
        // 新的binding可以进行改变
        str = "world".to_owned();
    
    }

    foo(v);
    `````

    > 2022-4-4 [question_link](https://stackoverflow.com/questions/71730828/why-does-an-immutable-struct-become-mutable-when-moved-into-a-vector)


25. 有时候rust的报错信息也是有误导性的，比如它有时说`method exist for xxx, but 
    the trait bound is not satisfed`，这种时候可能是对这种类型，这个方法压根不
    存在
    
    > 2022-4-4 [question_link](https://stackoverflow.com/questions/71730011/the-method-fold-exists-for-reference-t-but-its-trait-bounds-were-not-sat)

26. 在栈上的数据类型，`move`是会移动内存的，而在堆上的，目前来看只会移动栈上的指
    针，而堆上的内存则不会被移动。

    > There is a special case where the lifetime tracking is overzealous: when 
    you have something placed on the heap. This occurs when you use a Box<T>, 
    for example. In this case, the structure that is moved contains a pointer 
    into the heap. The pointed-at value will remain stable, but the address 
    of the pointer itself will move. In practice, this doesn't matter, as you 
    always follow the pointer.

    > 2022-4-4


27. match ergonomic这个rfc
  
    [link_to_detailed_doc](https://github.com/rust-lang/rfcs/blob/master/text/2005-match-ergonomics.md)

    这个`rfc`的行为是在对`reference`进行match并且`pattern`是`non-reference 
    pattern`时，rustc会自动将被match的`reference`给`dereference`，然后更新
    `bind mode`。

    最后所新生成的类型，如果最后一步的处理的pattern是`non-ref pattern`，那
    么所新拿到的类型是收到`bind mode`所影响的。

    > 2022-4-4 [question_link](https://stackoverflow.com/a/71733238/14092446)
    [question_link](https://stackoverflow.com/questions/71731788/weird-type-when-pattern-matching-references)


28. rust的饮式类型转换，`&U->&T(如果U实现了Deref<T>)`，还有就是`*U = *deref(&U) = T`
    ````
    ````
````
    ````
    ````
    ```
    `
   ````
    ```
    ```
    ````
     ```'```
````
````
    ````
````
````
```
   `
````
````
````
````
   ````
````
            ```
   `
````
````
