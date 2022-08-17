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
   
   Constant	Explanation
   goodbit	no error
   badbit	irrecoverable stream error
   failbit	input/output operation failed (formatting or extraction error)
   eofbit	associated input sequence has reached end-of-file
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
   typedef basic_ifstream<char> 		ifstream;

   /// Class for @c char output file streams.
   typedef basic_ofstream<char> 		ofstream;

   /// Class for @c char mixed input and output file streams.
   typedef basic_fstream<char> 		fstream;
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

   或者这样向文件写(会truncate掉文件)

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


2. 流是无法拷贝的
   
   > Ch7.md: 3

   通常都是传递可变引用，因为IO会改变流的状态

3. IO状态

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

9. 当一个`fstream`被销毁时，会自动地调用`close` (RAII)
