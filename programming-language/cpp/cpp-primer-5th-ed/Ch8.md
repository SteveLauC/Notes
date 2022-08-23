1. `std::getline`函数

   ```cpp
   template< class CharT, class Traits, class Allocator >
   std::basic_istream<CharT,Traits>& getline( std::basic_istream<CharT,Traits>& input,
                                              std::basic_string<CharT,Traits,Allocator>& str,
                                              CharT delim );

   template< class CharT, class Traits, class Allocator >
   std::basic_istream<CharT,Traits>& getline( std::basic_istream<CharT,Traits>&& input,
                                              std::basic_string<CharT,Traits,Allocator>& str,
                                              CharT delim );

   template< class CharT, class Traits, class Allocator >
   std::basic_istream<CharT,Traits>& getline( std::basic_istream<CharT,Traits>& input,
                                              std::basic_string<CharT,Traits,Allocator>& str );
   
   template< class CharT, class Traits, class Allocator >
   std::basic_istream<CharT,Traits>& getline( std::basic_istream<CharT,Traits>&& input,
                                              std::basic_string<CharT,Traits,Allocator>& str );
   ```

   你会发现它的返回类型是`std::basic_istream&`，所以可以连环调用。

   它是如何判断`EOF`的，有一个专门的类来说明`istream`的状态

   > 如果发生EOF，eofbit和failbit都会被置1

   ```cpp
   std::ios_base::iostate
                
   Specifies stream state flags. It is a BitmaskType, the following constants are defined:
   
   Constant        Explanation
   goodbit        no error
   badbit        irrecoverable stream error
   failbit        input/output operation failed (formatting or extraction error)
   eofbit        associated input sequence has reached end-of-file
   ```

   如果遇到了`EOF`，`iostate`的`eofbit`会被设置，可以让调用者对其进行检查

   ```cpp
   #include <iostream>
   #include <string>
   
   using std::string;
   
   int main()
   {
       string buf;
       while (std::getline(std::cin, buf)) 
       // 这里应该会检查iostate的状态
       // 等价于!std::getline(std::cin, buf).fail()
       {
           std::cout << buf << std::endl;
       }
   }
   ```

   > cpp的getline一个好处就是它自己会事先将buf清理掉，然后delimiter也不会被放到
   > 缓冲区中，相比Rust `BufRead::read_line(&mut String)`还是蛮方便的。

2. cpp读写文件

   ```cpp
   /// Class for @c char input file streams.
   typedef basic_ifstream<char>                 ifstream;

   /// Class for @c char output file streams.
   typedef basic_ofstream<char>                 ofstream;

   /// Class for @c char mixed input and output file streams.
   typedef basic_fstream<char>                 fstream;
   ```

   > 可以使用上面3种stream来做读写，目前还没有搞懂，先跳过。

   比较有意思的是，`fstream`是继承自`iostream`的，所以所有`iostream`支持的操作，
   `fstream`都可以做。比如使用`getline`从文件中读一行

   ```cpp
   #include <fstream>
   #include <iostream>
   
   int main()
   {
       std::string buf;
       std::ifstream f("back.cpp");
       std::getline(f, buf);
       std::cout << buf << std::endl;
       return 0;
   }
   ```
   ```shell
   $ g++ main.cpp && ./a.out
   //// a type to represent person
   ```

   或者这样向文件写

   ```cpp
   #include <fstream>
   #include <iostream>

   int main()
   {
       std::string buf("hello world");
       std::fstream f("back.cpp");
       f << buf << std::endl;
       return 0;
   }
   ```
   ```shell
   $ g++ main.cpp && ./a.out
   $ cat back.cpp
   hello world
   ```

   > 有点那种抽象的味道了


3. 流是无法拷贝的
   
   > Ch7.md: 3

   通常都是传递可变引用，因为IO会改变流的状态

4. IO状态

   就是第1点说的那个`iostate`

   cpp定义了一些函数，来方便你检查流的状态:
   ```cpp
   fstream f;

   f.eof() //是否到达eof
   f.fail() // 检查failbit或者badbit位
   f.bad() // 检查badbit
   bool good() const { return this->rdstate() == 0; } // rdstate是mask 因为good
   f.clear() // 将所有条件复位 置0
   f.clear(flags) // 复位指定位 比如只复位eof clear()
   void setstate( iostate state ) // 直接将状态变为flag 置1
   f.rdstate() // 返回Self  iostate rdstate() const;
   ```

   ```cpp
   /**
    *  @brief  Fast error checking.
    *  @return  True if no error flags are set.
    *
    *  A wrapper around rdstate.
    */
   bool good() const { return this->rdstate() == 0; } // rdstate是mask
   // 因为goodbit这个constant是0
   ```


   如果我们用上面的东西重写我们最开始的程序(循环从stdin读直到eof):

   ```cpp
   #include <iostream>
   
   int main()
   {
       std::string buf;
       std::ios_base::iostate stdin_state = std::cin.rdstate();
   
       // 写法1
       //    while ((stdin_state & std::ios_base::eofbit) == 0)
       //    {
       //        std::getline(std::cin, buf);
       //        stdin_state = std::cin.rdstate();
       //        std::cout << buf << std::endl;
       //    }
   
       // 写法2
       while (!std::cin.eof())
       {
           std::getline(std::cin, buf);
           std::cout << buf << std::endl;
       }
       return 0;
   }
   ```

   使用`clear`清除某些特定的位，比如清除`eofbit`。这个clear如果没有参数的话，那
   它就直接将iostate置0；如果传递参数的话，那么直接`iostate = argument`，所以使
   用它来清除某些特定的位比较反直觉:

   ```rust
   // 写成Rust的话大概是这样子
   fn clear(cur_state: std::ios_base::iostate, state: Option<std::ios_base::iostate>) {
       if state.is_none() {
           cur_state = 0;
       } else {
           cur_state = state;
       }
   }
   ```
   
   ```cpp
   // 不是这样直接清除
   std::cin.clear(std::ios_base::eofbit);

   // 而是这样子...而是这样子，这就很反直觉
   std::cin.clear(std::cin.rdstate() & ~std::ios_base::eofbit);
   ```

   ```cpp
   #include <iostream>
   
   int main()
   {
       std::cin.setstate(std::ios_base::eofbit);
   
       if (std::cin.eof())
       {
           std::cout << "EOF is set\n";
       }
       else
       {
           std::cout << "EOF is not set\n";
       }
   
       std::cin.clear(std::cin.rdstate() & ~std::ios_base::eofbit);
   
       if (std::cin.eof())
       {
           std::cout << "EOF is set\n";
       }
       else
       {
           std::cout << "EOF is not set\n";
       }
       return 0;
   }
   ```


   在Rust中读写返回`Ok(n)`表示此次读写是成功的，如果是`Ok(0)`则表示遇到了EOF。
   所以正常的读写应该这么些


5. 刷新缓冲区
   
   由于stdout在指向终端的时候是`line-buffered`，所以可以使用`std::endl`来刷新缓
   冲区。

   除此之外，还有2个显式地不打印还行符号来刷新的

   ```cpp
   std::flush;
   std::ends;
   ```

   ```rust
   // std::io::Write

   fn flush(&mut self) -> Result<()>;
   ```

   自动刷新

   ```cpp
   // Defined in header <ios>                

   std::ios_base& unitbuf( std::ios_base& str );
   std::ios_base& nounitbuf( std::ios_base& str );
   ```

   ```cpp
   // enable automatic flushing for stdout
   std::cout << std::unitbuf;

   std::cout << "hello";

   // disable it
   std::cout << std::nounitbuf;
   ```

6. 关联输入和输出
  
   当一个输入流被关联到输出流时，任何试图从输入流读取数据的操作都会先刷新关联的
   输出流。cpp将`cin`和`cout`关联到一起了，所以从`cin`读东西会flush到`cout`

   ```cpp
   #include <iostream>
   
   int main() {
       std::cout << "Type something: ";
   
       std::string input;
       std::cin >> input; // flush stdout
   
       std::cout << input << std::endl;
   }
   ```

   ```shell
   $ g++s main.cpp && ./a.out
   Type something: hello
   hello
   ```

   > 这和Rust就不一样了
   > ```rust
   > use std::io::stdin;
   >
   > fn main() {
   >     print!("Type someting: ");
   >
   >     let mut buf: String = String::new();
   >     stdin().read_line(&mut buf).unwrap();
   >     buf.truncate(buf.len()-1);
   >
   >     println!("{}", buf);
   > }
   > ```
   >
   > ```shell
   > $ cargo r -q
   > h
   > Type something: h
   > ```

7. 读写文件

   fstream继承于`iostream`，但除了继承于`iostream`的行为，它还定义了自己的行为

   ```cpp
   fstream fs; // 未绑定到文件的

   fstream fs("main.cpp"); // 绑定到`main.cpp`，默认mode是读写

   fstream fs("main.cpp", mode); // 以`mode`打开main.cpp

   is_open(); // checks if the stream has an associated file
 
   open(); // opens a file and associates it with the stream
 
   close(); // close the associated file
   ```

   `fstream fs('<name>')`以及`open(<name>)`中的`<name>`在c++11后既可以是c风格字符
   又可以是`std::string`，这是用函数重载实现的

   ```cpp
   // (since C++11)
   void open( const std::string &filename, ios_base::openmode mode = ios_base::in|ios_base::out );
   ```

   甚至还有这种重载

   ```cpp
   // (since C++17)
   void open( const std::filesystem::path &filename, ios_base::openmode mode = ios_base::in|ios_base::out );
   ```

8. 当一个`fstream`被销毁时，会自动地调用`close` (RAII)

9. split一个string
    
    ```cpp
    #include <iostream>
    #include <string>
    #include <vector>
    
    using std::string;
    using std::vector;
    
    vector<string> split(const std::string& source, const std::string& delim)
    {
        vector<string> res;
        string::size_type start_idx = 0;
        string::size_type found = source.find(delim, start_idx);
    
        while (found != string::npos)
        {
            res.push_back(source.substr(start_idx, found - start_idx));
            start_idx = found + delim.size();
            found = source.find(delim, start_idx);
        }
        if (start_idx != source.size())
        {
            res.push_back(source.substr(start_idx, source.size() - start_idx));
        }
    
        return res;
    }
     
    int main()
    {
        std::string buf("hello world");
        for (const string& item : split(buf, " "))
        {
            std::cout << item << std::endl;
        }
    }
    ```
    ```shell
    $ g++ main.cpp && ./a.out
    hello
    world
    ```

10. `string_view` (since c++17)
    
    > 和Rust中的`&str`很像，只不过没有`lifetime`的概念在里面

    对原字符串(c风格字符串/std::string/char array)只读的胖指针(首地址+size)

    ```cpp
    // 从char array生成string_view
    constexpr basic_string_view( const CharT* s, size_type count );

    // 从c字符串生成string_view
    constexpr basic_string_view( const CharT* s );

    // 从迭代器生成string_view
    template< class It, class End >
    constexpr basic_string_view( It first, End last );
    ```

    ```cpp
    // string to string_view

    // 目前没看懂这个是怎么转化的
    // 非explicit的单参数的构造函数 (见Ch7.md 24)
    constexpr basic_string_view( const CharT* s );

    string s("hello");
    string_view v = s;
    ```

    ```
    // string_view to string

    // explicit constructor 所以不可以隐式地类型转换，只能用constructor建造或者用static_cast<string>()
    // 不过诡异的是explicit不是应该给单参数的构造函数用的吗？？？
    // 怎么这个2个参数也可以用，因为它另一个参数有默认值...
    // 
    template< class StringViewLike >
    explicit basic_string( const StringViewLike& t, const Allocator& alloc = Allocator() );
    ```


    ```cpp
    string owned_str1("hello");

    string_view view_str = owned_str1; // implicit conversion
    string_view view_str2(owned_str1); // or explicitly call that constructor


    string owned_str2(view_str); // call the converting constructor
    string owned_str3 = static_cast<string>(view_str); // using static_cast
    ```

    > 在Rust里面，`String`倒是不可以隐式转换为`&str`，但是`&String`可以

11. `at()`
    
    
    `string`和`string_view`都重载了`operator[]`，但此操作在运行时并不会做bound 
    checking，所以在数组越界时会发生UB

    而`at()`函数则会进行运行时检查，和Rust中的`Index/IndexMut`类似。

    `at()`返回的是可变或只读的引用

    ```rust
    // Index/IndexMut返回的类型是`Output`

    pub trait Index<Idx> 
    where
        Idx: ?Sized, 
    {
        type Output: ?Sized;
    
        fn index(&self, index: Idx) -> &Self::Output;
    }
    ```

    ```cpp
    string owned_str1("hello");
    for (std::string::size_type i = 0; i < owned_str1.size(); i+=1) {
        owned_str1.at(i) = 'O';
    }

    std::cout << owned_str1 << std::endl;
    ```

    ```shell
    $ ./a.out
    OOOOO
    ```

12. 文件的打开模式

    ```
    in 只读
    out 只写
    app 等价于O_APPEND 每次写之前offset都移到末尾
    ate 打开文件后定位到文件末尾
    truc O_TRUNCATE
    binary 二进制I/O
    ```

    > Rust里面`OpenOptions`中的`append()`方法也是写前append，等价于`O_APPEND`
    > ```rust
    > use std::fs::{File, OpenOptions};
    >
    > fn main() {
    >     let f: File = OpenOptions::new().append(true).open("test").unwrap();
    > }
    > ```
    > 
    > 可以使用`strace`来查看一下
    >
    > ```shell
    > $ cargo b -q
    > $ strace ./target/debug/t
    > openat(AT_FDCWD, "test", O_WRONLY|O_APPEND|O_CLOEXEC) = -1 ENOENT (No such file or directory)
    > ```

    不同的文件stream是有默认的文件打开方式的，构造函数有默认参数

    ```cpp
    // fstream
    explicit basic_fstream( const char* filename, std::ios_base::openmode mode = ios_base::in|ios_base::out );

    // ifstream
    explicit basic_ifstream( const char* filename, std::ios_base::openmode mode = ios_base::in );

    // ofstream
    explicit basic_ofstream( const char* filename, std::ios_base::openmode mode = ios_base::out );
    ```

    > 这个`std::ios_base::openmode`是class`std::ios_base`的member type，其中的
    > 另一个我们熟知的member type是`iostate`。

13. IO类的继承关系
   
    ![diagram](https://github.com/SteveLauC/pic/blob/main/io-class.svg)

    > `std::ios_base`居然是最大的基类 

14. 默认情况下，没有指定`truc`的openmode，使用`out`打开文件还是会截断

    ```cpp
    #include <fstream>
    #include <iostream>
    
    using std::ofstream;
    
    int main()
    {
        ofstream fd("test");
        if (fd.is_open())
        {
            fd.write("12345", 5);
        }
    }
    ```

    ```shell
    $ g++s main.cpp
    $ echo "helloworld" >> test
    $ ./a.out
    $ cat test
    12345 # 而不是12345world
    ```


    为什么会这样呢，因为C的`fopen(filepath, "w")`也是写的方式打开，也会默认truncate
    掉文件，cpp为了和它保持一致。

    [link](https://stackoverflow.com/a/57070159/14092446)

    POSIX open(2)和Rust的`OpenOptions::new().write(true)`均不会这样。Rust中的
    `std::fs::write`会truncate掉。

    ```cpp
    // 这3种打开方式都会truncate掉文件
    ofstream of1("test");
    ofstream of2("test", ofstream::out);
    ofstream of3("test", ofstream::out | ofstream::trunc);
    ```

    如果想使用`ofstream`打开文件并且**不**truncate掉，只能在打开时同时指定上
    `ios_base::out|ios_base::app` (不过这样每次写都是追加)。或者使用`fstream`
    ，因为mode中既有in又有out。

    ```cpp
    // 验证fstream默认不会truncate

    fstream f("test");
    f.write("12345", 5);
    ```
    ```shell
    $ echo "helloworld" > test
    $ g++s main.cpp && ./a.out
    $ cat test
    12345world
    ```

    > ```cpp
    > ofstream("test", fstream::app); 
    > ```
    > 上面的代码手动指定了mode，但却没有指定`out`，如果这样写的话，则`out`是隐
    > 式指定的。目前不清楚这语法是怎么来的，但我写的话，肯定不要这样写。

15. `sizeof(reference)`

    标准指定了`sizeof(reference)`会返回被引用的类型的大小

    ```cpp
    sizeof(T&) returns sizeof(T)
    ```

16. fstream的`open`函数

    一个流可以在构造的时候指定绑定的文件，也可以在`open`是指定。

    ```cpp
    void open( const char *filename, ios_base::openmode mode = ios_base::in|ios_base::out );
    void open( const std::filesystem::path::value_type *filename, ios_base::openmode mode = ios_base::in|ios_base::out );
    void open( const std::string &filename, ios_base::openmode mode = ios_base::in|ios_base::out );
    void open( const std::filesystem::path &filename, ios_base::openmode mode = ios_base::in|ios_base::out );
    ```

    它还有一个`close`函数，cpp的RAII使得我们并不需要手动地关闭文件。所以这个函数
    并不是让我们用来关闭文件的，而是更换文件。当想要更换与流相绑定的文件时，我们
    需要先`close`再`open`。

17. string流

    > string流指的是包含一个String作为缓冲区的流

    ```cpp
    stringstream   // 流中的string buffer，既可读又可写
    istringstream  // 可以从流中的string buffer读
    ostringstream  // 可以向流中的string buffer写
    ```

    ![inheritance](https://github.com/SteveLauC/pic/blob/main/std-basic_stringstream-inheritance.svg)

    stringstream特有的一些操作

    1. 创建时同时初始化内置buffer

       ```cpp
       string buf("Hello world");
       // 创建ss时，初始化其内置stringbuf的内容为buf的内容
       stringstream ss(buf);

       // 构造函数 3
       explicit basic_stringstream( const std::basic_string<CharT,Traits,Allocator>& str, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out );
       ```

    2. 返回内置buffer的拷贝 string

       ```cpp
       stringstream ss;
       ss.str();

       std::basic_string<CharT,Traits,Allocator> str() const; (until C++20)
       std::basic_string<CharT,Traits,Allocator> str() const&; (since C++20)
       template<class SAlloc>
       std::basic_string<CharT,Traits,SAlloc> str( const SAlloc& a ) const;
       ```
    3. 使用其他string替换内置buffer
       
       ```cpp
       string new_buf("World")
       stringstream ss;
       ss.str(new_buf); // 

       void str( const std::basic_string<CharT,Traits,Allocator>& s ); (4)	
       template<class SAlloc>                                          (5)
       void str( const std::basic_string<CharT,Traits, SAlloc>& s );
       ```

18. 使用ostringstream将整个文件读到一个string中去

    ```cpp
    #include <fstream>
    #include <iostream>
    #include <sstream>
    using std::cout;
    using std::endl;
    using std::fstream;
    using std::ostringstream;
    
    int main()
    {
        ostringstream buf;
        fstream f("test");
        buf << f.rdbuf();
        cout << buf.str() << endl;
    }
    ```

    ```shell
    $ echo "Hello" > test
    $ g++s main.cpp && ./a.out
    Hello
    ```

    ```rust
    // 等同的rust操作
    use std::fs::{File, OpenOptions};
    use std::io::Read;
    
    fn main() {
        let mut f: File = OpenOptions::new().read(true).open("test").unwrap();
    
        let mut buf: String = String::new();
        f.read_to_string(&mut buf).unwrap();
        println!("{}", buf);
    }
    ```

    ```c
    // 相同的C操作
    #include <assert.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <stdint.h>
    
    int main(void)
    {
	    int32_t fd = open("test", O_RDONLY);
	    assert(fd != -1);
    
	    char buf[BUFSIZ];
	    ssize_t total_num_read = 0;
	    ssize_t num_read =
		    read(fd, buf + total_num_read, BUFSIZ - total_num_read);
    
	    while (num_read != 0) {
		    total_num_read += num_read;
		    num_read =
			    read(fd, buf + total_num_read, BUFSIZ - total_num_read);
	    }
    
	    buf[total_num_read] = '\0';
	    printf("%s\n", buf);
    
	    return EXIT_SUCCESS;
    }
    ```
