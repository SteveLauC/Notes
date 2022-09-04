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

15. emplace操作

    C++11引入了3个新成员

    ```
    emplace  -  insert
    emplace_front - push_front
    emplace_back - push_back
    ```

    和他们之前对应的函数不同的是，这些函数是**构造**而不是
    插入现成的元素。其参数是相应元素的构造函数的参数。

    ```cpp
    #include <string>
    #include <vector>
    #include <iostream>
    
    using std::string;
    using std::vector;
    using std::cout;
    using std::endl;
    
    struct Person {
        string name;
        int32_t age;
    
        Person(string name, int32_t age) : name(name), age(age) {
            cout << "I am constructed:" << endl;
            cout << this->name;
            cout << this->age;
            cout << endl;
        }
    };
    
    
    int main() {
        vector<Person> t;
        t.emplace_back("steve", 18);
        return 0;
    }
    ```

    ```shell
    I am constructed:
    steve18
    ```

    奇怪的是，下面的代码通不过编译，明明vector有传两个迭
    代器的构造函数

    ```
    #include <array>
    #include <string>
    #include <vector>
    #include <iostream>
    
    using std::array;
    using std::string;
    using std::vector;
    using std::cout;
    using std::endl;
    
    int main() {
        array<int32_t, 1> a = {1};
        vector<int32_t> t;
        
        t.emplace_back(a.begin(), a.end());
        return 0;
    }
    ```

    ```
    In template: excess elements in scalar initializer
    ```

16. List的迭代器没有`+=operator`运算符

    [这里](https://stackoverflow.com/a/43637819/14092446)有一个表格，很好

    上面问题ac的答案说list的iterator是`BirdirectionalIterator`，所
    以不支持`+=`的操作，然而好像并没有说为什么不支持这样
    的操作

17. 访问容器的元素

    顺序容器(包含array)都有一个`front()`成员函数，除了`forward_list`
    之外的容器都有一个`back()`函数。

    这两个函数会返回最前和最后元素的可变或不可变引用

    NOTE: 对一个空容器调用`front()/end()`是UB。

    正确的用法是在调用`front()/end()`前先检查容器是否非空

    ```cpp
    if (!c.empty()) {
        c.front();
        c.end();
    }
    ```

    还有一个访问的操作是使用`operator[]`， 如果越界就是UB。
    应该使用`.at(idx)`，这样越界会抛出`our_of_range`的异常。

    ```cpp
    int main() {
        vector<int32_t > a = {1};
        try {
            cout << a.at(10);
        // 这里直接使用的`std::exception`的子类
        }catch (const std::out_of_range& invalid_index) {
            cout << invalid_index.what() << endl;
        }
        return 0;
    }
    ```

    ```shell
    $ ./a.out
    vector::_M_range_check: __n (which is 10) >= this->size() (which is 1)
    ```

    上面的这4种操作返回的都是容器中元素的引用。如果容器
    是const的，则返回const引用；若不是，则返回普通引用。
    
    > 有点死板阿，如果引用作右值的话不就只读就可以了吗？
    > `front()`和`end()`要是把权限分开的话感觉比较好

18. 删除容器中的元素

    > array因为是固定大小，所以没有这些API
    >
    > forward_list有特殊的`std::erase, std::erase_if (filter)`
    > vector与string只支持pop_back()操作，因为pop前面的不高效

    ```cpp
    void pop_back(); // 删除尾部的元素 若容器为空,UB
    void pop_front(); // 删除首部的元素 若容器为空UB
    iterator erase( iterator pos ); // 删除`pos`的元素，返回其后的迭代器 pos是end()UB
    iterator erase( iterator first, iterator last ); // 删除[first, last)的元素，如果`first==last`什么也不做，返回其后的迭代器
    void clear(); // 清除容器内的所有元素
    ```

    > 关于erase的返回值，在写之前可以去[cppreference](https://en.cppreference.com/w/cpp/container/vector/erase)
    > Return value
    > 
    > Iterator following the last removed element.
    >
    > * If pos refers to the last element, then the end() iterator is returned.
    > * If last==end() prior to removal, then the updated end() iterator is returned.
    > * If [first, last) is an empty range, then last is returned

    > Rust中的`pop_back`是这样设计的:
    >
    > ```rust
    > pub fn pop(&mut self) -> Option<T>
    > ```
    > 用返回值是否为`None`来标识是否删除成功

    > C++和Rust的dequeue都支持`pop_front`和`pop_back`

    由于`pop`返回`void`，所以如果你要使用这个值的话，必须在`pop`前使用它

19. 容器的各种写操作会不会invalidate迭代器与指针或引用
    
    [answer](https://stackoverflow.com/a/54004916/14092446)

    比较好的是，这些写操作的返回值会返回新的合法的迭代器

    ```cpp
    vector<int32_t> v = {1, 2, 3};

    for (auto i = v.cbegin(); i < v.cend();) {
        if (*i % 2 == 0) {
            i = v.erase(i);
        } else {
            i += 1;
        }
    }
    ```

    比如cppreference给出的这个，去除容器内的偶数，在erase后,
    在此处和后面的迭代器都会失效，所以需要更新下`i`。还有
    就是`cend()`是每次循环都会被更新的，所以不需要我们去
    手动更新。(不要保存尾后迭代器)

20. forward_list的独有的迭代器
    
    ```cpp
    std::forward_list<T,Allocator>::before_begin
    std::forward_list<T,Allocator>::cbefore_begin
    ```

    Returns an iterator to the element before the first element of the container.
    This element acts as a placeholder, **attempting to access it results in 
    undefined behavior**. The only usage cases are in functions insert_after(),
    emplace_after(), erase_after(), splice_after() and the increment operator:
    incrementing the before-begin iterator gives exactly the same iterator as 
    obtained from begin()/cbegin().

21. forward_list独有的modifier

    ```cpp
    insert_after (C++11) inserts elements after an element
    emplace_after (C++11) constructs elements in-place after an element
    erase_after (C++11) erases an element after an element
    push_front (C++11) inserts an element to the beginning
    emplace_front (C++11) constructs an element in-place at the beginning (public member function)
    pop_front (C++11) removes the first element
    ```

    和一般的容器不一样，由于单链表的特殊属性，添加一个
    元素需要改变此元素的前置元素，所以我们的API的名字都
    是`xx_after`

    `20`说的那两个特别的迭代器就是为了这样的API而设计的

    比如删除`forward_list`中的奇数元素，在学数据结构的时候,
    在用C的单链表来写这题时，也是用两个指针

    ```cpp
    forward_list<int32_t> l = {1, 2, 3, 4, 5};

    auto prev = l.cbefore_begin();
    auto begin = l.cbegin();

    while (begin != l.cend()) {
        if (*begin % 2 == 1) {
	    // 注意这里
            begin = l.erase_after(prev);
        } else {
            prev = begin;
            std::advance(begin, 1);
        }
    }

    for (auto const &item: l) {
        cout << item << " ";
    }
    cout << endl;
    ```

    ```shell
    $ g++ main.cpp && ./a.out
    2 4
    ```

22. 改变容器的大小
    
    ```cpp
    void resize( size_type count ); (1)
    void resize( size_type count, const value_type& value ); (2)
    ```

    和rust中`std::vec::Vec::resize`是一样的

    ```
    pub fn resize(&mut self, new_len: usize, value: T)
    ```

    如果容器的当前大小大于`count`，则直接shrink；如果小于
    `count`，如果调用的是`1`，则填充默认值，如果是`2`，则填充
    `value`.

    ```cpp
    int main() {
        vector<int32_t > v;
        v.resize(3);
    
        for(const auto& item: v) {
            cout << item << " ";
        }
        cout << endl;
    }
    ```
    ```shell
    $ g++s main.cpp && ./a.out
    0 0 0 
    ```

    > 这个我感觉Rust的API有点傻，在要shrink的情况下仍需要提供`new_value`

23. 管理vector/string容量的成员函数

    ```cpp
    void shrink_to_fit(); // 请求让cap和size一样大，但不一定成功
    size_type capacity() const; // 返回容量
    void reserve( size_type new_cap ); // 让cap大于等于`new_cap`
    ``` 

    > Rust的`pub fn reserve(&mut self, additional: usize)`和C++中的语义
    > 是相同的，都是至少分配多少空间

24. string的substr操作
    
    ```cpp
    basic_string substr( size_type pos = 0, size_type count = npos ) const;
    ```
