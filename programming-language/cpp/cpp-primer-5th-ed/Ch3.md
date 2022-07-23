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
