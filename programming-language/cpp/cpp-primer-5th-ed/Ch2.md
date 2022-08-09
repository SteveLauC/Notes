##### 2.3.1 引用

1. 引用必须初始化，且在绑定后不可以变更绑定对象
   
   ```cpp
   #include <cstdint>
   #include <iostream>
   
   int main() {
     int32_t i = 0;
   
     int32_t &ref;   // ■ Declaration of reference variable 'ref' requires an initializer
     return 0;
   }
   ```

   避免了这样的情况出现

   ```cpp
   #include <string>
   #include <iostream>
   
   using std::string;
   
   int main() {
    const string &f;
    {
            string str("hlelo");        
            f = str;
    }
    std::cout << f << std::endl;
    return 0; 
   }
   ```

2. 由于引用不是一个对象，只是别名，所以不可以定义引用的引用

   ```cpp
   #include <cstdint>
   #include <iostream>
   
   int main() {
     int32_t i = 0;
     int32_t &ref = i;
     int32_t &ref_to_ref = ref; // another reference to i
     return 0;
   }
   ``` 

3. 引用的类型必须和被引用的类型保持一致
   
   ```cpp
   const int32_t i = 0;
   int32_t &ref = i;
   ```

   ```shell
   main.cpp:6:16: error: binding reference of type ‘int32_t&’ {aka ‘int&’} to ‘const int32_t’ {aka ‘const int’} discards qualifiers
   ```

   但是有2个例外:

   1. 在初始化常量引用时，只要可以初始化成功就可以         

      ```cpp
      int32_t i = 42;
      double f = 3.14;
      const int32_t &r1 = i; // ok
      const int32_t &r2 = 5; // ok
      const int32_t &r3 = r1 * 2; // ok
      const int32_t &r4 = f; // ok
      ```

      其实是编译器生成了一个临时的变量，然后绑定上去了

      ```
      double f = 3.14;
      const int32_t &r4 = f;

      // 等价于

      const int tmp = (int)3.14;
      const int32_t &r4 = tmp;
      ```

##### 2.4.4 constexpr常量表达式

1. 常量表达式指的是在编译期间运算出结果且值不改变的表达式，类似于rust中的`const`
   变量

   我们可以使用`constexpr`来告诉编译器或许可以在编译期间确定这个值

   ```cpp
   constexpr int32_t t = 9;
   ```

   > 和rust的const有点像，但不知道是否相同

   
   值得注意的是，constexpr修饰指针和const修饰指针语义不一样

   ```cpp
   const int32_t *p = nullptr; // the contents pointed by p is immutable
   constexpr int *p = nullptr; // p itself is immutable
   ```


##### 2.5.1 type alias

1. 可以使用`typedef`和`using`来给类型起别名
   
   ```cpp
   typedef int i32;
   using i32 = int;
   ```

##### 2.5.3 decltype 关键字
有时只想让编译器帮助我们推断类型，而不想使用被推断表达式的值，可以用`decltype()`
来拿到类型

```cpp
#include <cstdint>
#include <iostream>

int32_t foo() { return 1; }

int main() {
  decltype(foo()) i = 9;
  return 0;
}
```

> 切记`decltype(i)`和`decltype((i))`拿到的东西完全不一样，后者拿到的一定是引用
