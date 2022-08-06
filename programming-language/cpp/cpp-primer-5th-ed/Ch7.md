1. 定义在类内部的函数是隐式的inline函数

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
