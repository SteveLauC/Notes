1. 构造`std::string`
   
   ```cpp
   string s1;      // empty string
   string s2 = "hello"; // copy from string literal
   string s3 = string(10, 'c'); // cccccccccc
   string s4 = string(s3); // copy from s3
   ```

2. 直接初始化和拷贝初始化
 
   ```cpp
   string s_directly_initialized("hello"); // 直接初始化
   string s_copied_from_a_tmp_value = "hello"; // 拷贝初始化

   string s_direct(10, 'c'); // 直接初始化
   string s_copoed = string(10, 'c'); //拷贝初始化 
   ```

   拷贝初始化会先创建一个临时值，然后从临时值那里拷贝到新变量那里

   > 好像c++ 17把东西又给改了，真的蚌埠主了

3. 使用`std::getline`来读一整行

   ```cpp
   template< class CharT, class Traits, class Allocator >
   std::basic_istream<CharT,Traits>& getline( std::basic_istream<CharT,Traits>& input,
                                           std::basic_string<CharT,Traits,Allocator>& str );
   ```

   ```cpp
   std::string str = std::string();
   std::getline(std::cin, str);
   std::cout << str << std::endl
   ```

   注意换行符没有被读进来，而且在调用`getline`时它会将原来的buffer先清掉，再append
   到buf里面(和rust的行为不同)。还有一个值的注意的是这个函数重载了4次，其中有2个有
   第3个参数，`delim`，可以手动定义delimiter，默认的是换行符

   和`std::cin`一样，此函数返回stream的引用，由此可以用来判断stream的状态，是否读完

   ```cpp
   while(getline(std::cin, str)) {
       std::cout << str << std::endl;
   }
   ```

4. 判断string是否为空

   ```
   std::basic_string<CharT,Traits,Allocator>::empty
   ```

5. `std::basic_string`和`std::string`
  
   前者是template(范化版)，后者是item为char的前者

   ```cpp
   typedef std::basic_string<char> string
   ```

   前者的存在是为了支持unicode，c的历史遗留问题，char是1byte的

6. 拿到它们的大小
   
   使用`size()`或`length()`，他们是同义的，都返回`std::size_type`类型

   
   `decltype`在这时就有点用了...

   ```cpp
   decltype(std::string().size()) len = std::string("hello").size();

   // 当然也可以使用auto
   auto len = std::string("hello").size();
   ```

   需要注意的是，由于`size`返回的是无符号数，那么在比较的时候就要留意不同类型之
   间的转化。应该在`cmake`里面禁止掉不同类型的运算，不过我不知道怎么做...
   
   ```cpp
   #include <cstdint>
   #include <iostream>
   #include <string>
   
   int main() {
     decltype(std::string().size()) len = std::string("hello").size();
   
     int32_t n = -1;
   
     // len: unsigned long 
     // n: int32_t
     // n will be implicitly converted into unsigned long, which is UINT64_MAX
     if (len >= n) {
       std::cout << "len > n" << std::endl;
     } else {
       std::cout << "len < n" << std::endl;
     }
     return 0;
   }
   ```
   ```shell
   $ g++ main.cpp && ./a.out
   len < n
   ```

7. 比较字符串，和C不同，CPP将运算符对std::string进行了重载

   ```cpp
   if (std::string("h") > std::string("H")) {
     cout << "h > H" << endl;
   }
   ```
   ```cpp
   $ g++ main.cpp && ./a.out
   h > H
   ```

8. 拼接字符串

   在cpp中可以直接使用`+`来拼接字符串。也可以字符字面量和字符串字面量到字符串上，
   只要`+`两侧的操作对象至少有一个是`std::string`

   ```cpp
   auto str = '1' + std::string("hello");
   auto str2 = "hello world" + std::string("hello");
   auto str3 = std::string("hello") + std::string("hello");
   std::string str4 = "hello" + "hello"; // error
   ```

   看下面的语句
   ```
   auto str = std::string("hello") + " " + "world";
   ```
   第2个`+`明明左右都是字符串字面量，怎么会通过编译？

   因为`std::string("hello") + " "`的结果是一个字符串...


   ```cpp
   auto str = "steve" + " " + std::string("hello") + "world";
   ```
   这条语句则无法通过编译，因为cpp编译器从左往右看，看到的是2个字符串字面量。所
   以在写的时候，确保最左边2个当中有一个是`std::string`就好了

   > Rust也有这种限制。在Rust里面，重载`+`的函数签名是 `fn add(mut self,
   > other: &str) -> String`，所以`+`左侧必须是`String`
   >
   > String + &str + &str + &str ...这样的模式是允许的
   > 
   > 相比之下，cpp更加灵活一点。不过在Rust里大家都用`concat!`，而不是`+`


9. 检查字符的属性

   其实就是c中`ctype.h`里面的那些函数，只不过在cpp里面需要引`#include <cctype>`

   由于和C的是一样的，所以引起UB的地方也是一样的。参数必须是EOF或者是unsigned
   char可以表示的值，否则就是UB。

   cpp reference也比较贴心，像Linux manual一样将他们写到了Notes里面

   ```
   Like all other functions from <cctype>, the behavior of std::isupper is 
   undefined if the argument's value is neither representable as unsigned 
   char nor equal to EOF. To use these functions safely with plain chars 
   (or signed chars), the argument should first be converted to unsigned char:
   ```

   > 我惊了，cpp reference这页的最下面有一个ascii表，标注了哪些字符对于上面的函
   > 数返回真，哪些返回假[link](https://en.cppreference.com/w/cpp/string/byte/isupper)
   > 
   > ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-24%2011-30-59.png)

10. 遍历字符串中的字符
  
    ```cpp
    #include <iostream>
    #include <string>
    
    int main() {
      auto str = std::string("BBB");
      for (const char &c : str) {
              std::cout<< c << std::endl;
      }
      return 0;
    }
    ```
    ```shell
    $ g++ main.cpp && ./a.out
    B
    B
    B
    ```

    上面用了常引用，对于`str`是只读的访问。想要改变某个char可以用引用来写。我发
    现Rust在遍历`String`上只暴露了只读的接口`chars`。原因我想应该是utf-8是变长
    的，改变其中一个字形，可能引起字节长度的变化。但是cpp里面这个拿到的char其实
    就是u8，就可以随便改，不过可能把字符串改的面目全非就是了

    在Rust中将字符串全部改为大写，可以用到`flatten`这个函数

    ```rust
    use std::io::stdin;
    
    fn main() {
        let mut buf: String = String::new();
        stdin().read_line(&mut buf).unwrap();
    
        let res: String = buf.chars().map(|c: char| c.to_uppercase()).flatten().collect();
    
        println!("{}", res);
    }
    ```

11. 使用下标来访问字符串的单个字符
    
    ```cpp
    std::string str("helllo");
    assert(str[0] = 'h');
    ```

    需要注意的是如果索引越界，就是UB:(。还有就是要留意下索引的类型需要是`string
    ::size_type`也就是`unsigned long`，如果不是这种类型，千万留意隐式类型转换的
    结果。再就是在对字符串进行索引的时候，提前检查字符串是否为空，空字符串索引
    起来也是UB

    ```cpp
    if (!str.empty()) {
      str[x];
    }
    ```

    使用索引遍历字符串

    ```cpp
    std::string buf("hello");

    for (decltype(buf.size()) i = 0; i < buf.size(); i+=1) {
      std::cout << buf[i];
    }
    ```

12. `std::vector`中的范型不可以是引用，因为引用不是对象

     初始化:  
     ```cpp
     vector<T> v1; // 空的 Vector::new();
     vector<T> v2 = v1|v2(v1);// 拷贝 let v2 = v1.clone();
     vector<T> v3(n, val); // let v3 = vec![val; n];
     vector<T> v4(n); // let v4 = vec![T:default(); n];
     vector<T> v5 = {1, 2} | v5{1, 2}; // let v5 = vec![1, 2]; 注意是{}而不是()
     ```

     留意下下面的初始化语句

     ```cpp
     vector<string> v{10, "hello"}; // 初始化列表的{}也可以用来批量初始化
     ```

     如果编译器发现`10, "hello"`不是合法的初始化列表值，它就会转而用批量初始化
     来做。

     我写的时候还是老老实实用`()`批量初始化，`{}`来做初始化列表吧。从这里就可以
     一瞥cpp的烂，简单的事情也给你100种做法，徒增心智负担

     > 发现在给`类内初始值(in-class initializer)`的时候，使用`vector<T> v(n, val)`
     > 会报错，使用`vector<T> v{n, val}`则正常...好像编译器把它视作一个成员函数的
     > 函数签名了
     > ```cpp
     > class Window_mgr
     > {
     > private:
     > public:
     > std::vector<Screen> screens{3, Screen(24, 80, ' ')};
     > std::vector<Screen> screens(3, Screen(24, 80, ' '));
     > };
     > ```

13. 可写的与只读的迭代器

    > 所有的标准库容器都可以使用迭代器，均有`begin/cbegin/end/cend`函数来产生迭代
    器，返回的类型是某种智能指针，对其deref返回其指向的容器元素的引用。需要注意
    `(c)end`返回的是指向最后一个元素的后一个元素。`cbegin/cend`中的c是const，对其
    deref返回的是常引用

    ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-26%2008-40-58.png)


    `begin/end`创建的迭代器是可写可读还是只读的，由容器是否可写决定。在cpp11之前
    是没有`cbegin/cend`这两个函数的，这两个函数无论容器是否可以写都只能创建只读
    的迭代器，让语义更统一了

    留意下面的代码

    ```cpp
    #include <cstdint>
    #include <iostream>
    #include <vector>
    
    using std::vector;
    
    int main() {
      auto v = vector<int32_t>{10, 11};
      // std::cout << "metadata: cap = " << v.capacity() << std::endl;
      auto p = v.begin();
      auto e = v.end();
    
      v.push_back(123);
      // std::cout << "metadata: cap = " << v.capacity() << std::endl;
    
      while (p != e) {
        std::cout << *p << std::endl;
        p += 1;
      }
      return 0;
    }
    ```

    ```shell
    $ g++s main.cpp && ./a.out
    1669944888
    5
    ```

    发生了什么呢？拿到首尾指针之后，我们又`push`了一个元素，导致其堆上内存进行
    了重新分配，使得原先拿到的首位指针无效了。将上面代码的2行注释取消掉便可以
    很清晰地发现其内存分配。这段代码如果在Rust中写，则根本不会通过编译，因为拿
    到指针是borrow了vec，那么在引用被丢掉之前，根本不允许对vec进行写


14. 在cpp中判断2个变量是否具有相同的类型

    ```cpp
    auto a = 1;
    auto b = 2;

    cout << std::is_same<decltype(a), decltype(b)>::value << endl;
    ```

    其中`std::is_same<T, U>::value`的返回类型是bool

15. 迭代器的类型(begin end返回什么)

    我们可以不用关心其具体的返回类型，就像我们不关心`std::string::size()`的具体
    类型一样，但我们可以通过`Container::iterator/const_iterator`拿到

    ```cpp
    #include <cassert>
    #include <cstdint>
    #include <iostream>
    #include <string>
    #include <vector>
    
    using std::string;
    using std::vector;
    
    int main() {
      auto v = string();
      auto res = std::is_same<decltype(v.cbegin()), string::const_iterator>::value;
      assert(res == 1);
    
      auto vv = vector<int32_t>();
      res = std::is_same<decltype(vv.begin()), vector<int32_t>::iterator>::value;
      assert(res == 1);
    
      return 0;
    }
    ```

16. 批量移动迭代器

    ```cpp
    auto str = string("hello");

    auto p = str.cbegin();
    auto end = str.cend();

    while(p != end) {
         cout << *p << endl;
         p += 2;
    }
    ```

    迭代器支持像上面的那样的`p += n`这样的批量移动的操作，但在批量移动的时候千万
    小心，别刚好错过`end`变成非法的迭代器

    ```
    $ g++ main.cpp && ./a.out
    h
    l
    o
    sdjfljsl
    ...    # 乱七八糟的输出

    [1]    351109 segmentation fault (core dumped)  ./a.out
    ```

    或许把循环的条件改为`p <= end`则不会有问题，但是如果越界了拿到非法的迭代器
    再去和`end`比较谁知道会不会是正常的呢?书上说参与比较的两个迭代器必须是同一
    容器中的元素或者尾元素的下一位置(a.k.a. end())

    
17. 两个迭代器相减的类型
    
    很多容器都定义了`Container::difference_type`来表示两个迭代器相减，这是一个
    有符号的数据类型

18. cpp为了把数组搞得像容器，引入了`std::begin()/std::end()`函数来返回首指针和
    指向尾元素后一元素的指针

    ```cpp
    #include <cassert>
    #include <cstdint>
    #include <iostream>
    
    int main() {
      int32_t v[] = {1, 2, 3};
      int32_t *p = std::begin(v);
      assert(p == v);
      size_t len = sizeof(v) / sizeof(int);
      int32_t *end = std::end(v);
      assert((end - 1) == &v[len - 1]);
    
      return 0;
    }
    ```

    除了可以让数组用起来更加像容器，还有一个作用就是让指针更安全，使用函数可以确
    保创建出来的指针不会指向别的地方

19. 使用数组初始化vector

    ```cpp
    #include <cassert>
    #include <cstdint>
    #include <iostream>
    #include <iterator>
    #include <vector>
    
    using std::vector;
    
    int main() {
      int32_t v[] = {1, 2, 3};
      auto vv = vector<int32_t>(std::begin(v), std::end(v));
    
      for (const int32_t &v : vv) {
        std::cout << v << std::endl;
      }
      return 0;
    }
    ```

    又或者使用数组的一部分来初始化vector

    ```cpp
    auto vv = vector<int32_t>(std::begin(v) + 1, std::end(v) - 1); // 只包含元素2
    ```
