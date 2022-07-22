1. iostream包含istream以及ostream

   ```
   cin  => stdin
   cout => stdout
   cerr => stderr
   clog => also stderr
   ```

2. 关于返回值
 
   ```
   std::cin >> v1 >> v2;
   // 等价于
   (std::cin >> v1) >> v2;
   ```

   由于`std::cin >> v1`的返回值是`std::cin`(返回左侧对象)，所以我们可以像最上面
   一样，将其写在一行上。

3. 注释界定符`/**/`不可以嵌套，在c中也是这样的，在rust里面可以
	
   ```cpp
   /*
    *
    *  abc /**/ hello Unknown type name hello
   */ 
   ```

4. read until EOF
 
   ```cpp
   #include <iostream>
   
   int main() {
     int v = 0;
     while (std::cin >> v) {
      std::cout << v << std::endl;
     }
     return 0;
   }
   ```
