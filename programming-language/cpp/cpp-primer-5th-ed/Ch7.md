1. `定义`在类内部的函数是隐式的inline函数

2. const成员函数(const member function)

   cpp的成员函数中的隐式的`this`指针是`T* const p`, 如果成员函数再是`const`的话，
   那么`this`指针就会变为`const T *const p`，指`this`指针无法对类实例进行修改。

   ```cpp
   class Person {
       string name;

       void print_name() const { this->name = "steve"; }
   };
   ```

   ```shell
   $ g++ main.cpp
   main.cpp: In member function ‘void Person::print_name() const’:
   main.cpp:35:58: error: passing ‘const string’ {aka ‘const std::__cxx11::basic_string<char>’} as ‘this’ argument discards qualifiers [-fpermissive]
      35 |     void print_name() const { this->name = string("steve"); }
         |                                                          ^
   In file included from /usr/include/c++/11/string:55,
                    from main.cpp:3:
   /usr/include/c++/11/bits/basic_string.h:717:7: note:   in call to ‘std::__cxx11::basic_string<_CharT, _Traits, _Alloc>& std::__cxx11::basic_string<_CharT, _Traits, _Alloc>::operator=(std::__cxx11::basic_string<_CharT, _Traits, _Alloc>&&) [with _CharT = char; _Traits = std::char_traits<char>; _Alloc = std::allocator<char>]’
     717 |       operator=(basic_string&& __str)
         |       ^~~~~~~~
   ```

   和Rust中的`fn foo(&self)`类似。

3. `std::istream`和`std::ostream`都是不可以copy的

   > 2) The copy constructor is protected, and is deleted. Output streams are 
   > not copyable.
   > [basic_ostream](https://en.cppreference.com/w/cpp/io/basic_ostream/basic_ostream)

   所以我们想要实现`cout << 1 << 2`这种连续的调用的话，重载函数的参数和返回值都
   必须是`istream&/osteam&`的引用

   比如，我们定义一个自己的类，给它实现`<<`的重载(类似于Rust中的`std::fmt::Display`trait)

   ```cpp
   #include <cstdint>
   #include <iostream>
   #include <ostream>
   
   using std::ostream;
   
   class Time
   {
       uint32_t hour;
       uint32_t min;
   
   public:
       friend ostream& operator<<(std::ostream& os, const Time& instance)
       {
           os << "Time is: (hour: " << instance.hour << ", min: " << instance.min
              << ")" << std::endl;
   
           return os;
       }
   };
   
   int main()
   {
       auto t = Time();
       std::cout << t << ".." << std::endl;
   }
   ```

   比较有意思的是，这个函数是一个友元函数，并不是成员函数。需要是友元是因为字段
   是`private`的，所以需要`friend`来访问他们。

   不过正常情况下应该是将函数定义的外面，然后把函数签名放到类里面做友元

   ```cpp
   class Time
   {
       uint32_t hour;
       uint32_t min;
   
   public:
       friend ostream& operator<<(std::ostream& os, const Time& instance);
   };
   
   ostream& operator<<(std::ostream& os, const Time& instance)
   {
       os << "Time is: (hour: " << instance.hour << ", min: " << instance.min
          << ")" << std::endl;
   
       return os;
   }
   ```

   由于读和写会改变流的内容，所以他们都是可变的引用，而非`const`引用

4. 默认构造函数default constructor

   如果我们没有**显示**地提供构造函数，编译器就会为我们隐式地生成一个默认构造函数。
   编译器创建的构造函数又称为`合成的默认构造函数(synthesized default constructor)`


   合成的默认构造函数将按以下规则来初始化类:
   1. 如果存在类的初始值，用它来初始化成员
   2. 否则，默认初始化

   > 有点像Rust中的std::default::Default
   >
   > ```rust
   > pub trait Default {
   >     fn default() -> Self;
   > }
   > ```
   > 如果是类的初始值就像Rust中我们手动实现`Default`，默认初始化则是我们令编译器
   > 来实现`#[derive(Default)]`

   > 类的初始值是这样的
   > ```cpp
   > class Person {
   >     string name = "steve"; // 类的初始值
   > };
   > ```

5. 在默认构造函数参数后面写上`=default`
   
   > since c++11

   默认情况下，只有在程序员什么构造函数都不提供的情况下，编译器才会为我们生成默
   认构造函数。而这个关键字是强行让编译器为我们生成默认构造函数，无论我们有没有
   手动生成其他的构造函数。

   如果我们既需要默认构造函数，又需要其他的构造函数，`=default`会很有用。比如下
   面的代码，我们既需要默认构造函数，又需要有参构造函数。

   > 不过仔细想想，在Rust里面`Default`还是用得比较少的

   ```cpp
   #include <cstdint>
   #include <iostream>
   #include <ostream>
   #include <string>
   
   using std::string;
   
   class Person
   {
       string name;
       uint32_t age;
   
   public:
       Person() = default;
   
       Person(string name, uint32_t age) : name(name), age(age){};
   
       friend std::ostream& operator<<(std::ostream&, const Person&);
   };
   
   std::ostream& operator<<(std::ostream& os, const Person& item)
   {
       os << "Person: (name: " << item.name << ", age: " << item.age << ")\n";
       return os;
   }
   
   int main()
   {
       auto p = Person();
   
       auto pp = Person("steve", 18);
   
       std::cout << p << pp;
   }
   ```

6. 有参构造函数，但只提供部分字段

   如果只提供部分字段，其他未提供的字段将会被隐式默认初始化

   ```cpp
   #include <cstdint>
   #include <iostream>
   #include <ostream>
   #include <string>
   
   using std::string;
   
   class Person
   {
       string name;
       uint32_t age;
   
   public:
       Person() = default;
   
       Person(string name) : name(name){}; // 只提供name, age会被默认初始化
   
       friend std::ostream& operator<<(std::ostream&, const Person&);
   };
   
   std::ostream& operator<<(std::ostream& os, const Person& item)
   {
       os << "Person: (name: " << item.name << ", age: " << item.age << ")\n";
       return os;
   }
   
   int main()
   {
       auto p = Person();
   
       auto pp = Person("steve");
   
       std::cout << p << pp;
   }
   ``` 

   在Rust中的话，需要显式地写出来

   ```rust
   #[derive(Default, Debug)]
   struct Person {
       name: String,
       age: u32,
   }
   
   impl Person {
       fn new(name: String) -> Self {
           Self {
               name,
               age: u32::default(),
           }
       }
   }
   
   fn main() {
       let p: Person = Person::default();
       let pp: Person = Person::new("steve".into());
   
       println!("{:?}\n{:?}", p, pp);
   }
   ```

7. `=default`和`=delete`
   
   编译器会默认提供:
   1. 默认构造函数
   2. 拷贝构造函数
   3. `=`的重载函数
   4. 析构函数
   
   如果我们没有手动地去处理他们的话。

   构造函数规则:
   1. 如果程序员手动提供了有参构造函数，那么编译器则不再提供默认构造函数。
   2. 程序员手动提供了拷贝构造函数，编译器不再提供默认构造函数

   有的时候，我们不想应用这种构造函数规则，比如手动提供了有参构造，又想要编译器
   提供默认构造函数，就可以使用`=default`显式地写出来

   而`=delete`则是明确地告诉编译器，不想要它给我们提供的默认实现。

8. copy constructor和copy assignment的区别

   * copy constructor是用来给之前**没有**被初始化的变量使用的
   * copy assignment是给之前**已经**被初始化的变量使用的

   > 如何理解记忆，copy constructor是一个constructor，是用来construct一个没有被
   > construct的东西的

   ```cpp
   A a;
   A b;
   b = a; // copy assignment
   ```

   ```cpp
   A a;
   A b = a; // copy constructor
   ```

   注意，`A a;`这样的写法，`a`是被初始化的了，调用的是默认构造函数。不和c一样，
   这是cpp比较强调的一个地方。当然也不和Rust一样，Rust的初始化是很显式的。

   可以测试一下:

   ```cpp
   class Person
   {
   public:
       Person() = delete;
   };
   int main()
   {
       auto p; // Constructor is called implicitly
   
       return 0;
   }
   ```

   ```shell
   $ g++s main.cpp
   main.cpp: In function ‘int main()’:
   main.cpp:79:5: error: declaration of ‘auto p’ has no initializer
      79 |     auto p;
         |     ^~~~
   ```

9. `class`和`struct`的唯一区别就是默认的访问权限，`class`默认为`private`，而
   `struct`默认为`public`

10. 友元: 一般来说最好在类的定义开始或者结束前的位置集中声明友元

11. cpp允许为特定的类来创建某种类型在类中的别名，cpp称其为`member type`，有点像
    Rust中trait里面的类型一样？但用途很不一样哈
   
    ```cpp
    class Screen
    {
    public:
        using pos = std::string::size_type;
    
    private:
        pos cursor = 0;
        pos height = 0;
        pos width = 0;
        std::string contents;
    };
    ```

    比如我们在上面的例子里面，将`std::string::size_type`在`Screen`中重命名为
    `pos`。其实`std::string::size_type`也是在`string`中的一种重命名

    Member type也是有访问权限的，比如这个public的，在`Screen`外就可以使用`Screen::pos`
    来拿到

12. mutable的成员
    
    当一个字段被标记为`mutable`的时候，即使是const的变量，或者是`const member
    function`，也可以修改此mutable字段

    ```cpp
    class Person
    {
    public:
        mutable uint32_t age = 0;
    
        Person() = default;
        inline void change_age(uint32_t new_age) const;
    };
    
    // const function can change mutable field
    inline void Person::change_age(uint32_t new_age) const { this->age = new_age; }
    
    int main()
    {
        auto p = Person();
        std::cout << p.age << std::endl;
        p.change_age(18);
        std::cout << p.age << std::endl;
    
        // mutable field of a constant can also be modified.
        const auto con_p = Person();
        std::cout << con_p.age << std::endl;
        con_p.age = 18;
        std::cout << con_p.age << std::endl;
        return 0;
    }
    ```

    ```shell
    $ g++ main.cpp
    ./a.out
    0
    18
    0
    18
    ```
