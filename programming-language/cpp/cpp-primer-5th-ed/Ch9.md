#### Ch9: 顺序容器

1. 顺序容器类型以及其在Rust中的对照
  
   * vector: std::vec::Vec
   * deque: (not a continuous memory container) std::collections::VecDeque 
     > Rust中的VecDeque也是不连续的，甚至它提供了一个方法依VecDeque的值返回连续
     > 的slice
     > ```rust
     > pub fn make_contiguous(&mut self) -> &mut [T]
     > ```
     > Rearranges the internal storage of this deque so it is one contiguous
     > slice, which is then returned.
     
     > deque在内存中不连续，但却支持快速随机访问，这是怎么实现的阿?

   * list: std::collections::LinkedList
   * forward_list: 单链表 Rust std 没有单链表
   * [array](https://en.cppreference.com/w/cpp/container/array): [T;n]
     
     注意这个不是C array:

     ```cpp
     template< class T, std::size_t N> struct array; (since C++11)
     ```

     > 这个array有`begin/end/cbegin/cend`方法，虽然返回的也是`T *`.

   * string: std::string::String

   > 除array外，其他类型均具有高效的堆内存管理

2. 容器的典型member type:
   
   ```
   iterator: 迭代器类型
   const_iterator: 只读的迭代器
   size_type: 一种无符号整数，可以保证足够保存最大容器的数量
   difference_type: 有符号整数，保存两个元素之间的距离
   value_type: 容器内元素类型 T
   reference: value_type& 左值类型
   const_reference: const value_type&
   ```

3. 容器典型构造函数

   ```
   C c; 默认构造
   C c1(c2) 拷贝构造
   // array不支持
   C c(b, e) 将迭代器[b, e]内的元素拷贝到新的容器c中
   C c{a, b, c} 列表初始化
   ```

4. 容器的赋值与swap

   ```
   c1 = c2; 将c1用c2覆盖
   c = {a, b, c} 将c1中的元素替换为列表中元素
   c1.swap(c2) 将c1与c2进行替换 
   std::swap(c1, c2)  将c1与c2进行替换 
   ```

5. 容器的大小操作

   ```
   c.size() c中元素的数目 (forward_list不支持此操作)
   c.max_size() 容器中最多可以放置的元素数量
   c.empty() c.size() == 0
   ```

6. 容器的修饰操作

   ```
   c.insert 插入
   c.clear 清除
   c.emplace 使用args来构造pos前面的元素
   c.erase 清除具pos处的元素
   ```

7. 运算符

   ```
   == !=
   < > <= >=  map/unordered_map不支持
   ```

8. 迭代器
   
   ```
   c.begin c.end // const or non-const depends the container itself
   c.cbegin c.cend // const iterator
   c.rbegin c.rend // reversed iterator
   ```
   
   `std::forward_list`不支持`--`运算，因为是单链表。

9. 列表初始化
   
   ```cpp
   container c = {item1, item2, item3 ...};
   ```
   
   对于除array外的容器，使用列表初始化会默认指定容器的大
   小。对于array，如果初始化列表给出的初始化数据过少，则
   其余的元素默认初始化。如果给的太多，则会报错。

   ```cpp
   int main() {
       array<int32_t, 4> a = {1, 3};
       for (const auto & item: a) {
           cout << item << endl;
       }
       return 0;
   }
   ```
   ```shell
   $ g++ main.cpp && ./a.out
   1
   3
   0
   0
   ```

10. 接受容器大小及(可选的)元素初始值的构造函数

    > array不支持，array貌似只支持初始化列表

    元素初始值是可选的，如果没有提供，则会调用`T`的默认构造函数

    > string调用此构造函数必须提供初始化值
    >
    > ```cpp
    > // string构造时必须要给初始值
    > // until c++20
    > basic_string( size_type count, CharT ch, const Allocator& alloc = Allocator() );
    > // since c++20
    > constexpr basic_string( size_type count, CharT ch, const Allocator& alloc = Allocator() );
    > ```
    > 而vector就是支持的
    >
    > ```cpp
    > // until c++11 以前是以默认参数的形式提供的
    > explicit vector( size_type count, const T& value = T(), const Allocator& alloc = Allocator());
    > // (since C++11) (until C++14) 以后则是以只有一个参数的形式来提供
    > // explicit vector( size_type count );
    > // (since C++14) (until C++20)
    > // explicit vector( size_type count, const Allocator& alloc = Allocator() );
    > ```

    ```cpp
    Container c(size, init_val);
    ``` 

11. array支持赋值与拷贝操作
    
    内置数组不支持赋值与拷贝，但是array可以

    ```cpp
    #include <array>
    #include <iostream>
    
    using std::array;
    using std::size_t;
    using std::cout;
    using std::endl;
    
    template<class T, size_t N>
    void print_array(array<T, N>);
    
    int main() {
        array<int32_t, 10> a;
        array<int32_t, 10> b = {1, 2, 3};
        a = b; // copy
        print_array(a);
        a = {4, 5, 6}; // assignment
        print_array(a);
        return 0;
    }
    
    template<class T, size_t N>
    void print_array(const array<T, N> a) {
        for (const T &item: a) {
            cout << item << " ";
        }
    
        cout << endl;
    }
    ```
    ```shell
    $ g++ main.cpp && ./a.out
    1 2 3 0 0 0 0 0 0 0
    4 5 6 0 0 0 0 0 0 0
    ```

12. 顺序容器(array除外)的assign函数

    赋值运算符号要求左右两边具有相同的类型相同的长度。
    但assign则并不是这样，它支持左右两边不同但相容的类型，
    或者从容器的自序列赋值

    assign操作一般都提供以下几种操作
    
    ```cpp
    void assign( size_type count, const T& value );
    template< class InputIt >
    void assign( InputIt first, InputIt last );
    void assign( std::initializer_list<T> ilist );
    ```

    比如从`vector<float>`转为`vector<int32_t>`

    ```cpp
    int main() {
        vector<int32_t> a;
        vector<float> b = {1.1, 2.2};
        a.assign(b.begin(), b.end());
    
        for (const auto &item: a) {
            cout << item << endl;
        }
    
        return 0;
    }
    ```

    或者从`vector<char*>`到`vector<string>`

    ```cpp
    int main() {
        vector<string> a;
        vector<const char *> b = {"hello"};
        a.assign(b.begin(), b.end());
    
        for (const auto &item: a) {
            cout << item << endl;
        }
    
        return 0;
    }
    ```

13. swap操作是否会invalidate iterator
    

    顺序容器(除了array/string)的`swap`操作并不会在对堆上的数据进行
    移动，只是动栈上的胖指针。所以指向容器的迭代器，指针
    引用都不会失效。

    string不知道为什么会invalidate(clion 给了warning)，array是因为swap会真的移动栈上
    的东西。

    但是end iterator并不保证不会失效，详见[link](https://stackoverflow.com/a/4125186/14092446)

14. 容器的insert操作

    cpp中容器的insert都是使用的迭代器来确定位置的，不懂为
    什么要这么设计。可能只是为了让人们多使用迭代器吧。

    其返回值也是iterator，指向新插入的第一个元素。

    利用这一特性，我们可以在同一个位置反复地插入数据

    ```cpp
    int main() {
        // insert {3, 4, 5} before 6
        vector<int32_t> v = {1, 2, 6};
        int32_t i = 5;
        vector<int32_t>::const_iterator p;
        for (auto begin = v.cbegin(); begin != v.cend(); begin += 1) {
            if (*begin == 6) {
                p = begin;
                break;
            }
        }
        while (i != 3) {
            p = v.insert(p, i);
            i -= 1;
        }
    
        for (const auto &item: v) {
            cout << item << " ";
        }
        cout << endl;
        return 0;
    }
    ```
    ```shell
    $ g++s main.cpp && ./a.out
    1 2 3 4 5 6 
    ```
