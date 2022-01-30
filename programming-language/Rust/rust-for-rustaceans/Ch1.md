1. string literal是readonly的，在c中，试图修改它会产生UB

   > he C and C++ standards just say that string literals have static storage 
   duration, any attempt at modifying them gives undefined behavior, and 
   multiple string literals with the same contents may or may not share the 
   same storage.

   ```c
   int main() {
       char * str = "hello";
	   *str = 'H';
	   return 0;
   } 
   [1]    7114 bus error  ./main
   ```

   ```rust
   fn main() {
       let str: &str = "hello";
   }
   ```
   而在rust中，你跟本就没有办法去修改它，因为其类型是`&str`，如果硬改成`&mut str`
   则无法通过编译。

   还有一点和c不同的是，C中的`str`只是一个指针，而rust中的`str`则是一个胖指针，除
   指针那一部分，还有字符串的长度。c的字符串比较特殊，null-terminated string，就没
   必要保存长度了大概？

2. 关于变量
   作者认为随着对rust理解的逐渐深入，会对变量的理解有更好的理解，并在心中形成mental
   model，这种model可以大概分为两类: 
   1. high-level model: 更高的视角，不那么具体，更关心代码的总体结构。对变量而言，就
      是lifetime和borrows。
   2. low-level model: 更加具体，更加底层，注重实现的细节，在处理unsafe code和raw ptr
      时，这个model大有裨益。


