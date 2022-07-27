1. 类型转换
  
   * static_cast<T>(): 可以静态地将类型进行转换
   
   * dynamic_cast<T>(): 支持运行时时别

   * const_cast<T>(): 用来将const属性从类型中去除或加上

   * reinterpret_cast<T>(): 将对象的那些字节重新解释为别的类型，像rust中的
   `std::mem::transmute()`。

   比如指针类型的转换，就需要使用`reinterpret_cast`

   ```cpp
   #include <cassert>
   #include <cstdint>
   #include <cstdio>
   #include <iostream>
   #include <iterator>
   #include <vector>
   
   // using std::vector;
   
   int main() {
     int32_t v[2][2] = {1, 2, 3, 4};
   
     char *c = reinterpret_cast<char *>(std::begin(v));
     return 0;
   }
   ```
