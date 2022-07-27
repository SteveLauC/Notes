1. const形参和实参
  
   顶层const `const int a = 0`，此时的`const`就是顶层的

   一个函数，如果形参是有顶层const的，那么在传实参的过程中，既可以传const 变量
   也可以传正常的非const变量，也就是说，顶层const的传参时会被忽略掉

   ```cpp
   void foo(const int);
   int main() {
     int a = 0;
     const int b = 0;
     foo(a);
     foo(b);
     return 0;
   }

   void foo(const int i) {
     std::cout << i << std::endl;
   }
   ```

   指针类型的顶层const指的是限制此指针不能指向别的地址的const，而不是限制指针内
   容不能更改的

   ```cpp
   #include <cstdio>
   #include <iostream>
   
   void foo(const int *p);
   void foo(int *p);
   
   int main() {
     return 0;
   }
   
   void foo(const int *p) { return; } // 此const就不是顶层const
   void foo(int *p) { return; }
   ```

   > [What are top-level const qualifiers?](https://stackoverflow.com/questions/7914444/what-are-top-level-const-qualifiers)


2. 函数重载可以在编译时就被决定，只要函数不是virtual的(不需要vtable那些东西)

   只要形参列表不一样，就可以重载

   ```cpp
   #include <cstdio>
   #include <iostream>
   
   int32_t max(int32_t a, int32_t b);
   int64_t max(int64_t a, int64_t b);
   
   int main() {
     printf("%d", max(1, 2));
     printf("%ld", max(static_cast<int64_t>(2), static_cast<int64_t>(3)));
     return 0;
   }
   
   int32_t max(int32_t a, int32_t b) { return a; }
   
   int64_t max(int64_t a, int64_t b) { return a; }
   ```


   由于传参时顶层const会被忽略掉，`const TYPE t`和`TYPE t`在编译器眼里是一样的，
   所以在重载时不同的这样的类型是够不成重载条件的

   ```cpp
   #include <cstdio>
   #include <iostream>
   
   int32_t max(int32_t a, int32_t b);
   int32_t max(const int32_t a, const int32_t b);
   
   int main() {
     printf("%d", max(1, 2));
     return 0;
   }
   
   int32_t max(int32_t a, int32_t b) { return a; }
   
   int32_t max(const int32_t a, const int32_t b) { return a; }
   ```

   ```shell
   $ g++ main.cpp
   main.cpp: At global scope:
   main.cpp:14:9: error: redefinition of ‘int32_t max(int32_t, int32_t)’
      14 | int32_t max(const int32_t a, const int32_t b) { return a; }
         |         ^~~
   main.cpp:12:9: note: ‘int32_t max(int32_t, int32_t)’ previously defined here
      12 | int32_t max(int32_t a, int32_t b) { return a; }
         |         ^~~
   main.cpp: In function ‘int32_t max(int32_t, int32_t)’:
   main.cpp:14:44: warning: unused parameter ‘b’ [-Wunused-parameter]
      14 | int32_t max(const int32_t a, const int32_t b) { return a; }
         |                              ~~~~~~~~~~~~~~^
   ```

   ```cpp
   #include <iostream>
   
   void foo(int *const p);
   void foo(int *p);
   
   int main() { return 0; }
   
   void foo(int *const p) { return; }
   void foo(int *p) { return; }
   ```

   ```shell
   $ g++ main.cpp
   main.cpp: At global scope:
   main.cpp:10:6: error: redefinition of ‘void foo(int*)’
      10 | void foo(int *p) { return; }
         |      ^~~
   main.cpp:9:6: note: ‘void foo(int*)’ previously defined here
       9 | void foo(int *const p) { return; }
         |      ^~~
   main.cpp: In function ‘void foo(int*)’:
   main.cpp:10:15: warning: unused parameter ‘p’ [-Wunused-parameter]
      10 | void foo(int *p) { return; }
         |          ~~~~~^
   ```
