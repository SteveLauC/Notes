1. const形参和实参
  
   顶层const `const int a = 0`，此时的`const`就是顶层的

   一个函数，如果形参是有顶层const的，那么在传实参的过程中，既可以传const 变量
   也可以传正常的非const变量，也就是说，顶层const的传参时会被忽略掉

   ```cpp
   void foo(const int);
   int main() {
     int a = 0;
     const int b = 0;
     foo(a);
     foo(b);
     return 0;
   }

   void foo(const int i) {
     std::cout << i << std::endl;
   }
   ```

   和Rust中的`&mut T`可以隐式转换为`&T`有点像

   指针类型的顶层const指的是限制此指针不能指向别的地址的const，而不是限制指针内
   容不能更改的

   ```cpp
   #include <cstdio>
   #include <iostream>
   
   void foo(const int *p);
   void foo(int *p);
   
   int main() {
     return 0;
   }
   
   void foo(const int *p) { return; } // 此const就不是顶层const
   void foo(int *p) { return; }
   ```

   > [What are top-level const qualifiers?](https://stackoverflow.com/questions/7914444/what-are-top-level-const-qualifiers)


2. 函数重载可以在编译时就被决定，只要函数不是virtual的(不需要vtable那些东西)

   只要形参列表不一样，就可以重载

   ```cpp
   #include <cstdio>
   #include <iostream>
   
   int32_t max(int32_t a, int32_t b);
   int64_t max(int64_t a, int64_t b);
   
   int main() {
     printf("%d", max(1, 2));
     printf("%ld", max(static_cast<int64_t>(2), static_cast<int64_t>(3)));
     return 0;
   }
   
   int32_t max(int32_t a, int32_t b) { return a; }
   
   int64_t max(int64_t a, int64_t b) { return a; }
   ```


   由于传参时顶层const会被忽略掉，`const TYPE t`和`TYPE t`在编译器眼里是一样的，
   所以在重载时不同的这样的类型是够不成重载条件的

   ```cpp
   #include <cstdio>
   #include <iostream>
   
   int32_t max(int32_t a, int32_t b);
   int32_t max(const int32_t a, const int32_t b);
   
   int main() {
     printf("%d", max(1, 2));
     return 0;
   }
   
   int32_t max(int32_t a, int32_t b) { return a; }
   
   int32_t max(const int32_t a, const int32_t b) { return a; }
   ```

   ```shell
   $ g++ main.cpp
   main.cpp: At global scope:
   main.cpp:14:9: error: redefinition of ‘int32_t max(int32_t, int32_t)’
      14 | int32_t max(const int32_t a, const int32_t b) { return a; }
         |         ^~~
   main.cpp:12:9: note: ‘int32_t max(int32_t, int32_t)’ previously defined here
      12 | int32_t max(int32_t a, int32_t b) { return a; }
         |         ^~~
   main.cpp: In function ‘int32_t max(int32_t, int32_t)’:
   main.cpp:14:44: warning: unused parameter ‘b’ [-Wunused-parameter]
      14 | int32_t max(const int32_t a, const int32_t b) { return a; }
         |                              ~~~~~~~~~~~~~~^
   ```

   ```cpp
   #include <iostream>
   
   void foo(int *const p);
   void foo(int *p);
   
   int main() { return 0; }
   
   void foo(int *const p) { return; }
   void foo(int *p) { return; }
   ```

   ```shell
   $ g++ main.cpp
   main.cpp: At global scope:
   main.cpp:10:6: error: redefinition of ‘void foo(int*)’
      10 | void foo(int *p) { return; }
         |      ^~~
   main.cpp:9:6: note: ‘void foo(int*)’ previously defined here
       9 | void foo(int *const p) { return; }
         |      ^~~
   main.cpp: In function ‘void foo(int*)’:
   main.cpp:10:15: warning: unused parameter ‘p’ [-Wunused-parameter]
      10 | void foo(int *p) { return; }
         |          ~~~~~^
   ```

3. 默认参数

   函数可以有默认的参数，`void foo(int size = 1)`，比如这个参数`size`就有默认参
   数1。

   需要注意的是，在函数中，如果一个参数具有默认参数，那么它右边的参数都要有默认
   参数

   ```cpp
   #include <cstdint>
   #include <iostream>
   #include <string>
   
   using std::cout;
   using std::endl;
   using std::string;
   
   void hello(string name = string("nobody"), uint32_t repeat);
   
   int main() {
     hello("steve", 2);
     return 0;
   }
   
   void hello(string name = string("nobody"), uint32_t repeat) {
     for (uint32_t i = 0; i < repeat; i += 1) {
       cout << "hello" << name << endl;
     }
   }
   ```
   ```shell
   main.cpp:16:53: error: default argument missing for parameter 2 of ‘void hello(std::string, uint32_t)’
      16 | void hello(string name = string("nobody"), uint32_t repeat) {
         |                                            ~~~~~~~~~^~~~~~
   main.cpp:16:19: note: ...following parameter 1 which has a default argument
      16 | void hello(string name = string("nobody"), uint32_t repeat) {
         |            ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~
   ```

   在调用的时候，省略的实参必须从右边开始省略:

   ```
   void hello(string name = string("nobody"), uint32_t repeat = 1) {
     for (uint32_t i = 0; i < repeat; i += 1) {
       cout << "hello" << name << endl;
     }
   }

   hello(3);
   ```

   比如在上面的代码中，我们想要省略`name`只提供`repeat`，这是不可以的
   ```
   main.cpp:12:9: error: could not convert ‘2’ from ‘int’ to ‘std::string’ {aka ‘std::__cxx11::basic_string<char>’}
      12 |   hello(2);
         |         ^
         |         |
         |         int
   ```
   不过可不可以还要看编译器可以不可以将你后面的参数给隐式类型转换成前面参数的类
   型，如果可以的话，那编译还是会通过的。
   
   > 隐式类型转换还是很危险的，不过完全没有隐式类型转换又不那么人体工学，如何权
   > 衡还是挺难的

4. 带有默认实参的函数的声明

   默认参数的指定既可以写在函数声明中，又可以写在实现中，但只可以选择一处。

   如果写在实现里面貌似`可见性`会有一些问题，别的编译单元会看不见这个默认参数。
   但我现在对cpp的编译单元不懂。SO上的人认为为了让大家(caller)都可以看见，应该
   写在声明中

   [Where to put default parameter value in C++?](https://stackoverflow.com/questions/4989483/where-to-put-default-parameter-value-in-c)

   这个[答案](https://stackoverflow.com/a/2842942/14092446)说明了可见性的问题

   ```cpp
   // hello.h

   #ifndef HELLO_H
   #define HELLO_H
   
   #include <cstdint>
   #include <iostream>
   #include <string>
   
   using std::string;
   
   void hello(string name, uint32_t repeat);
   
   #endif
   ```

   ```cpp
   // hello.cpp

   #include "hello.h"
   
   using std::cout;
   using std::endl;
   
   void hello(string name = string("steve"), uint32_t repeat = 1) {
     for (uint32_t i = 0; i < repeat; i += 1) {
       cout << name << endl;
     }
   }
   ```
   ```cpp
   // main.cpp

   #include "hello.h"

   int main() { hello(); }
   ```

   ```shell
   main.cpp: In function ‘int main()’:
   main.cpp:24:19: error: too few arguments to function ‘void hello(std::string, uint32_t)’
      24 | int main() { hello(); }
         |              ~~~~~^~
   In file included from main.cpp:22:
   hello.h:10:6: note: declared here
      10 | void hello(string name, uint32_t repeat); // 编译器看到的函数原型是这样的
         |      ^~~~~
   ```

   > 关于编译单元，应该和c是一样的，一个headr+若干的cpp实现

5. 局部变量不能作为默认参数

   按照将默认参数给在声明，声明定义在header的习惯来说，没有办法在header确定一个
   在运行时创建的局部变量吧...

6. 内联函数

   函数的调用会有一些开销，栈帧的分配，环境的保存巴拉巴拉。对于一些小函数，可以
   使用内联函数让其在编译器将函数展开从而将函数变为普通的语句避免掉函数调用的开
   销 

   ```cpp
   #include <cstdint>
   #include <iostream>
   
   using std::cout;
   using std::endl;
   
   inline int32_t max(int32_t a, int32_t b);
   
   int main() {
    cout << max(1, 2) << endl;
   }
   
   inline int32_t max(int32_t a, int32_t b) { return (a < b) ? b : a; }
   ```
   比如上面的`max`函数，很小，写成语句可读性不好，但写成代码会有调用开销。就可以
   用内链函数

   在Rust里面`std::cmp::max`也是被指定为内链函数的

   ```Rust
   #[inline]
   #[must_use]
   #[stable(feature = "rust1", since = "1.0.0")]
   #[cfg_attr(not(test), rustc_diagnostic_item = "cmp_max")]
   pub fn max<T: Ord>(v1: T, v2: T) -> T {
       v1.max(v2)
   }
   ```

   > [inlining from Rust performance book](https://nnethercote.github.io/perf-book/inlining.html)

   > 被标记为`inline`只是向编译器提出请求，但是会不会被inline掉还要看编译器的想
   > 法

7. conexpr函数
 
   > 类似于Rust中的`const`函数

   cpp中的`constexpr`函数会隐式地被转换为内链函数


   cpp允许`constexpr`函数的参数不是常量，当参数不是常量时，函数的返回值也不是常
   量

   ```cpp
   #include <cstdint>
   #include <iostream>
   
   const int32_t foo(int32_t a, int32_t b);
   
   int32_t main() {
     int32_t i = 1;
     int32_t j = 2;
     int a = foo(i, j);
     std::cout << a << std::endl;
   }
   
   const int foo(int32_t a, int32_t b) { return a + b; }
   ```
   ```shell
   $ g++s main.cpp && ./a.out
   3
   ```

   Rust也可以

   ```rust
   fn main() {
       let a = 1;
       let b = 2;
       let r: u8 = add(a, b);
       println!("{}", r);
   }
   
   const fn add(a: u8, b: u8) -> u8 {
       a + b
   }
   ```
   ```shell
   $ cargo r
   3
   ```

8. `assert`与`NDEBUG`

   `assert`依赖于`NDEBUG`，当`NDEBUG`被定义的时候，`assert`所有的`assert`语句都
   会失效

9. `__func__`是编译器定义的一个字符串，代表此语句出现的函数名
  
   还有一些其他的常用宏，比如

   ```c
   #include <stdio.h>
   
   int main(void)
   {
    printf("%s\n", __func__);
    printf("%d\n", __LINE__);
    printf("%s\n", __FILE__);
    printf("%d\n", __COUNTER__);
    return 0;
   }
   ```

   Rust也有这些的编译宏

   ```rust
   fn main() {
       println!("{}", line!());
       println!("{}", column!());
       println!("{}", file!());
   }
   ```
