1. 错误处理

   * throw expression
     
     > cpp有一系列可以用的[expression class](https://en.cppreference.com/w/cpp/error/exception)

   * try catch


   ```cpp
   #include <cstdint>
   #include <exception>
   #include <iostream>
   
   int32_t max(int32_t a, int32_t b);
   
   int main() {
   
     int32_t a = -1;
     int32_t b = 9;
   
     try {
       int32_t res = max(a, b);
     } catch (const std::exception &t) { // you should always catch expressions as const
       std::cout << "an error occured: " << t.what() << std::endl;
     }
     return 0;
   }
   
   int32_t max(int32_t a, int32_t b) {
     if (a < 0 || b < 0) {
       throw std::invalid_argument("must be non-negative");
     }
   
     if (a < b) {
       return b;
     } else {
       return a;
     }
   }
   ```

   感觉着，一个函数如果要出错就可以`throw`，然后在其调用处可以使用`try`将其包裹，
   如果函数真的`throw`了，那么就会被`catch`到；如果没有`throw`，就会正常执行
  
   类比rust的话

   ```Rust
   fn main() {
       let a: i32 = -1;
       let b: i32 = 9;
   
       let res: i32;
   
       match max(a, b) {
           Ok(m) => res = m,
           Err(msg) => {
               println!("an error occured: {}", &msg);
           }
       }
   }
   
   fn max(a: i32, b: i32) -> Result<i32, String> {
       if a < 0 || b < 0 {
           return Err("must be non-negative".to_owned());
       }
   
       if a < b {
           Ok(b)
       } else {
           Ok(a)
       }
   }
   ```
 
   不过有一点不同的是，如果一个`throw`，在外部没有`try`包裹它，而它真的出错`throw`
   了，那么整个函数就会崩溃。
 
2. catch 具体的错误类

   上面的cpp代码catch的是`std::exception`，这是最大的那个父类。可以在catch里面
   抓它的子类来捕捉具体的错误信息

   在Rust里怎么做这个呢，感觉就是定义自己的错误类型，然后将其返回出去吧

   ```
   fn max<E: std::error::Error>(a: i32, b: i32) -> Result<i32, E> {
   ```

3. what()
   
   每一个`std::exception`都有`what()`这个成员函数，可以返回`const char *`的c字符
   串

   ```cpp
   virtual const char* what() const throw(); // (until C++11)
   virtual const char* what() const noexcept; // (since C++11)
   ```

4. 程序在寻找处理代码的过程中退出
  
   也就是它会遍历所有的`catch`，像`switch`一样，如果没有匹配到则直接调用[`terminate`](https://en.cppreference.com/w/cpp/error/terminate)
   退出
