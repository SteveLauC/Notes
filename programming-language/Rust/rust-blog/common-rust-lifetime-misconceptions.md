1. `T/&T/&mut T`的关系，`T`是包含`&T`和`&mut T`的，是后两者的超集，后两者是不相
   交的集合

   ![illustration](https://github.com/SteveLauC/pic/blob/main/Screen%20Shot%202022-04-07%20at%2010.07.27%20AM.png)


2. `T: 'static'`和`&'static'`的区别，前者包含后者和owned的类型

    > `T: 'static'`读作T is bounded by a 'static lifetime，`&'static T'`读作T has a static lifetime

   ![illustration](https://github.com/SteveLauC/pic/blob/main/Screen%20Shot%202022-04-07%20at%2010.31.04%20AM.png)


   理清这两者之间的关系之后再来看看`&'static T'`，它是一个不可变的引用，这个引用
   指向一个块可以存活无限长时间的内存，那么对于`T`这块内存的要求就是1要求其不可
   变，2要求在`&'static T'`创建后`T`不会move掉。所以`&'static T'`这种引用是可以在
   运行时创建的，只要内存不可变并且我们主动地进行内存泄露(防止被move掉)。

   ![illustration](https://github.com/SteveLauC/pic/blob/main/Screen%20Shot%202022-04-07%20at%2011.00.46%20AM.png)

   ```rust
   // 演示`T: 'static`是可变的
   fn foo<'a, T>(mut item: T)
   where T: 'static + From<&'a str> 
   {
       item = T::from("ab");
   }

   fn main() {
       let str: String = String::new();
       foo(str);
   }
   ```

   ```rust
   // 演示`T: 'static`包含`&'static T'`
   fn main() {
       let literal: &'static = "";
       foo(literal);
   }```


5. 如果我们有一个`struct`并且它是generic over `'a`的，然后我们想写一个方法，其参数
   是`&'a mut self`，那么我们就是在向rust说这个方法会可变独占地借用这个struct，在
   整个struct的生命周期中

   ```rust
   #[derive(Debug)]
   struct RefNum<'a> (&'a i32);

   impl<'a> RefNum<'a> {
       fn foo(&'a mut self){}
   }

   fn main() {
       let mut r = RefNum(&8);
       r.foo();
       r.foo();  // can not borrow r as mutable more than once
       println!("{:?}", r);
   }
   ```

   解决办法也很直接，那就是把方法的`'a`生命周期给去掉，手动写一个别的生命周期或者
   让生命周期缺省规则帮你写

   ```rust
   impl<'a> RefNum<'a> {
       fn foo(&mut self){}
   }
   ```
