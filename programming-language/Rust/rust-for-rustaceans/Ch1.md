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
		};
	}
	```

