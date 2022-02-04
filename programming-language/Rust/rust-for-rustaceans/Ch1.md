1. string literal是readonly的，在c中，试图修改它会产生UB

   > he C and C++ standards just say that string literals have static storage 
   duration, any attempt at modifying them gives undefined behavior, and 
   multiple string literals with the same contents may or may not share the 
   same storage.

   ```c
   int main() {
       char * str = "hello";
	   *str = 'H';
	   return 0;
   } 
   [1]    7114 bus error  ./main
   ```

   ```rust
   fn main() {
       let str: &str = "hello";
   }
   ```
   而在rust中，你跟本就没有办法去修改它，因为其类型是`&str`，如果硬改成`&mut str`
   则无法通过编译。

   还有一点和c不同的是，C中的`str`只是一个指针，而rust中的`str`则是一个胖指针，除
   指针那一部分，还有字符串的长度。c的字符串比较特殊，null-terminated string，就没
   必要保存长度了大概？

2. 关于变量
   作者认为随着对rust理解的逐渐深入，会对变量有更好的理解，并在心中形成mental
   model，这种model可以大概分为两类: 
   1. high-level model: 更高的视角，不那么具体，更关心代码的总体结构。对变量而言，就
      是lifetime和borrows。
   2. low-level model: 更加具体，更加底层，注重实现的细节，在处理unsafe code和raw ptr
      时，这个model大有裨益。

3. high-level model
   在这个model中，我们不把变量看作字节的容器，仅仅把它看作是一种被赋值的名字。当一个
   value被给了一个variable，这个value就由这个variable命名了。

   当访问一个变量时，可以想想着，绘制一条线，这条线的起点是上次的访问代码语句，终点是
   当前访问变量的代码。在rust中，一个变量如果被move掉，就不可以再被访问了，在这种画线
   的思考方式中，就意味着没有线可以从move掉的变量画出了。

   在这种模型中，一个变量只有具有合法的值后，它才"存在"，才可以从它画出线。当一个变量
   未初始化或被move掉，它就不"存在"。

   使用这种模型或者叫思考方式，整个程序就由各种各样的线组成了，称这种线为flow(感觉中文
   译作生命线更好哈哈哈)，每一条线都是一个变量的生命历程。编译器会检查各条flow，确保在
   同一时间各条flow是compatible的(其实就是borrow checker的工作)。比如，对同一个变量，
   同一时间，不能存在2条flow均对这个变量有写的权限；或者，不能有一条flow借用了一个变量，
   而没有一个flow拥有这个变量(dangling ptr)。

   ```rust
   fn main() {
       let mux x;
	   x = 42;
	   let y = &x;
	   x = 43;
	   assert_eq!(*y, 42);
   }
   ```
   比如上面这个代码，在第57行x被初始化，那么就可以从这里开始画线，从57行到59行，这条flow
   拥有这个变量。57-58-60，这条flow共享借用了变量。borrow checker会在每条flow的每个顶点
   进行判断，是否出现了incompatible的flow，在行59这个顶点处，borrow checker发现，有两条
   flow，一条拥有变量，一条共享借用了变量，并且borrwo checker知道对于变量x，具有写权限的
   flow出现时，其他flow应该都不存在，所以borrow checker发现这里是incompatible的flow，拒绝
   编译。

   varable shawing: 在rust中，如果一个变量已经存在，而在随后的代码中，又使用`let`关键字对
   这个变量进行重新的binding，这称之为variable shawing。老旧的变量并不会立即调用drop函数，
   但已经不可以再访问老旧变量。在high-level model的思维方式中，如果一个变量被shadow，那么
   两者的flow应该是一前一后的两条。

4. low-level model: 在这个模型中，一个变量就等同于一个块具名的内存块。当变量未被初始化时，
   这块内存是空的，我们不可以访问它。这种思考方式是C/CPP或者说是low level programming的思
   考方式。

   > `将变量看作具名的内存块`，这一说法不是很准确，忽略了寄存器。

5. heap
   在rust中使用堆上的内存，最常见的是使用Box手动显式地将要使用的东西放到堆上，返回一个栈上
   的指针，8B。
   
   ```rust
   fn main(){
       let b: Box<i32> = Box::new(1);
	   println!("{}", std::mem::size_of_val(&b));
	   // 8
   }
   ```

6. `'static`用作trait bound
   As a trait bound, it means the type does not contain any non-static references. Eg. the 
   receiver can hold on to the type for as long as they want and it will never become 
   invalid until they drop it.  

   It's important to understand this means that any *owned* data always passes a 'static 
   lifetime bound, but a reference to that owned data generally does not:'

   ```rust
   use std::fmt::Debug;

   fn print_it( input: impl Debug + 'static ) {
       println!( "'static value passed in is: {:?}", input );
   }

   fn main() {
	   // i is owned and contains no references, thus it's 'static:
	   let i = 5;
	   print_it(i);

	   // oops, &i only has the lifetime defined by the scope of
	  // main(), so it's not 'static:
	   print_it(&i);
   }
   ```

7. `'static`做trait bound的一个典型用法就是`std::thread::spawn`
   ```rust
   pub fn spawn<F, T>(f: F) -> JoinHandle<T> 
   where
       F: FnOnce() -> T,
       F: Send + 'static,
       T: Send + 'static, 
	```
	因为新的thread可能outlive当前的线程，所以这个闭包必须自给自足(owned)，或者
	说借用的都是在整个程序的生命周期都合法的东西。

8. rust中的`const`函数是可以给`const/static`的变量赋值的函数

9. 在rust中的static标记的东西并不一定是在static memory里的东西，例如owned的东西
   自己自足。

10. 试想一下在rust中，`Box`类型是Copy的会怎样
    ```rust
	fn main(){
		let b: Box<i32> = Box::new(1);
		let cpy_b: Box<i32> = b;
	}
	```

	`b`和`cpy_b`是两个具有相同值的指针，他们都对堆上这块存储着`1`的内存负责，当
	main函数要执行完，这两个指针都会尝试去释放堆上的内存。也就是出现那种double
	free的问题
	写cpp的时候，如果是一个class含指针，实现浅拷贝，最后的析构函数如果对指针直接
	delete/free可能会出现这个问题。


11. rust中drop的顺序
	规则很简单，在变量(包含函数参数)是倒序析构，嵌套的类型是按照源代码的顺序析构

    > 为什么嵌套的类型的析构顺序是正序，因为safe的rust不允许self-reference

	> 变量的倒序是因为在栈上，FILO的特点。书上没有说原因是这个，而是后面的变量可
	能会引用前面的变量，如果这时前面的析构了，后面的引用就无效了。

	测试代码:
	```rust
    use std::ops::Drop;

    struct Name{
        inner: String,
    }

    struct Person{
        name: Name,
    }

    impl Drop for Name{
        fn drop(&mut self) {
            println!("dropping Name: {}", self.inner);
        }
    }

    impl Drop for Person{
        fn drop(&mut self) {
            println!("dropping Person")
        }
    }

    fn main(){
        test_variables();
        test_nested();
    }

    fn test_variables(){
        let n1: Name = Name {
            inner: "first".into(),
        };
        let n2: Name = Name {
            inner: "second".into(),
        };
    }

    fn test_nested(){
        let p: Person = Person {
            name: Name{
                inner: "steve".into(),
            },
    test_nested();
        };
    }
    ```

12. references are pointers that come with an additional contract for how they 
    can be used, such as whether the refercnce provides exclusive access to the
    referenced value, or whether the referenced value may also have other references
    point to it.

13. 当有`shared reference`时，rust编译器会保证在`reference`背后的值不会发生改变。
    以下代码的断言应该永远成立:
    ```rust
    fn cache(input: &i32, sum: &mut i32) {
        *sum = *input + *input;
        assert_eq!(*sum, *input * 2);
    }
    ```
     
    `shared ref`没有权限去更改ref后面的值，并且，当`shared ref`存在时，mut的原变量
    也没有更新值的权利。`shared ref`就像是把一个东西借给别人看一看，当你借出去后，
    不能动小手脚，借给人家时什么样人家在借用期间就得是什么样。
    ```rust
    fn main()
        let mut x: i32 = 1;
        let shared_ref: &i32 = &x;
        x = 5;
        println!("{}", shared_ref);
    }
    // 上段代码编译不过。
    ```
14. `mutable ref`是独占的，所以rustc在某些代码中可以给出特殊的优化:
    ```rust
    fn noalias(input: &i32, output: &mut i32) {
        if *input == 1 {
            *output = 2;
        }
        if *input != 1 {
            *output = 3;
        }
    }
    ```
    
    比如这段代码，由于`output`是`mutable ref`，是独占的，所以rustc可以确定input和
    output不是一个东西，第一个if的赋值不会影响第二个if的判断，不需要去再执行第二
    个分支两个if可以合并为一个.

    ```rust
    fn noalias(input: &i32, output: &mut i32) {
        if *input == 1 {
            *output = 2;
        }else{
            *output = 3;
        }
    }
    ```

    而如果`input`和`output`可以指向同一个值的话，其指向的值又是1的话，那么最后其值
    会变成3；不会指向同一个值而input是1的话，output会是2.

15. `mut ref`的可变形只能是改变最近的那一层指针，也就是只能改变它指向的东西。如果这
    个`mut ref`是很多级指针，更高层的可变形，要看更高层的ref是不是mut的了。

    ```rust
    let x = 42;
    let y: &i32 = &x;
    let z: &mut &i32 = &mut y;
    ```
    这段代码，z可以修改`y`，但不能间接地修改`x`，因为中间人`y`是不可变的借用。

16. 在rust里，`mut ref`和`onwer`的区别就在于:
    1. `owner`需要负责析构。
    2. 当你将一个`mut ref`后面的值move走以后，需要给一个新的，因为`owner`要析构
    ，不能没有值给它去析构。
    
    > 在safe的rust中貌似不能通过ref move掉背后的值，error[E0507]: cannot move 
    out of `*r` which is behind a mutable reference，`r`是一个`mut ref`。不可以
    直接move，只能间接换

    ```rust
    // Replaces dest with the default value of T, returning the previous dest 
    // value.
    std::mem::take 
    pub fn take<T>(dest: &mut T) -> T 
    where
        T: Default, 

    // Swaps the values at two mutable locations, without deinitializing either 
    // one.
    std::mem::swap
    pub fn swap<T>(x: &mut T, y: &mut T)
    ```

17. 当被borrow时就不能重新赋值了，无论是怎样的borrow。 
    ```rust
    fn main(){
        let mut x: i32 = 42;
        let r: &mut i32 = &mut x;
        x = 8;
        println!("{}", r);
    }
    // mut ref 出现时，同样不能赋值
    error[E0506]: cannot assign to `x` because it is borrowed
    ```

18. `std::mem::take`是`std::mem::replace(&mut value, Default::dafalutl())`的一种
    语法糖
 
19. 看代码

    ```rust
    fn main(){
        let str: String = String::from("old string");
        let p: &mut String = &mut str;

        *p = String::from("new string");
        // `old string` is dropped immediately
    }
    ```
    当老旧的值被替换掉，老旧的值立即被drop

20. interior mutability
    some types provide interior mutabilitym, meaning they allow you to mutate a
    value through a shared reference.

    在rust中的`interior mutability`的类型可以按照实现方式分为两类:
    1. 真的通过`shared ref`给你`mutable ref`，比如`Mutex`或者`RefCell`
    2. 一个萝卜一个坑，不给你`shared ref`但是给你替换它内部数据的能力，直接换掉。
    比如`atomic`或者是`Cell`。

21. 下面这段代码挺不可思议

    ```rust
    fn main(){
        let mut x = Box::new(4);
        let r = &x;
        *x = 8;
        println!("{}", r);
    }
    ```

    这段代码无法通过编译，会报:

    ```rust
    error[E0506]: cannot assign to `*x` because it is borrowed
     --> src/main.rs:4:5
       |
     3 |     let r = &x;
       |             -- borrow of `*x` occurs here
     4 |     *x = 8;
       |     ^^^^^^ assignment to borrowed `*x` occurs here
     5 |     println!("{}", r);
             |                    - borrow later used here

     For more information about this error, try `rustc --explain E0506`.
     ```
     按照逻辑，被借用的是`x`，这是一个栈上的指针，里面的值是堆上的地址，我借用的是
     `x`而修改的是堆上的值，这两码事啊

     怎么说，借用的是`x`，但是`r`的操作还是对堆上的值进行操作的。

     但rustc并不能看到这种深度，你借用了x，那么和x有关的东西都不能动。

     还有就是对`x`进行`deref`操作时，其代码会变为`DerefMut::deref_mut(&mut x)`，所以
     `x`既有可变借用又有不可变借用，自然是不被允许的。

22. 当一个变量被重新赋值时，他的ref就已经不合法了，如果此时我们还在使用这个ref的
    话，代码就不能通过编译，除非我们对`ref`重新赋值，让其重得生命周期。

    ```rust
    1 fn main() {
    2     let mut x: Box<i32> = Box::new(2);
    3     let mut r: &Box<i32> = &x;
    4     x = Box::new(1);
    5    
    6     println!("{}", r);
    7 }
    ```

    以上的代码无法通过编译，会抱`cannot assign to x while it's borrowed`

    用rust for rustacean书中的这种high-level model来看的话，我们的`ref`的flow开始
    于行3，然后一直到了行6，在这其中，x被重新赋值了，是不被允许的。另一个视角，进
    行生命周期标注，从行3开始有生命周期`'a`，当x被重新赋值后，`r`的生命周期`'a`就
    该结束了，然后`borrow checker`发现在行6还被使用，就出现了问题。

    如果我们在行5加上`r = &x`，将生命周期再续上，给他一个新的生命周期，那么代码就
    合法了。那么此时的flow就是从行3到行4(由于x重新赋值致使第一个生命周期终结)，然
    在行5重新开始另一段生命周期，到行6。

    > 从lifetime为了压制悬垂指针的角度来看，当line4执行完后，r就已经是一个悬垂指针
    了，原来的`x`被析构，r已经指向不复存在的东西了。

   
