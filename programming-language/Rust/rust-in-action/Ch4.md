##### 4.1.1
1. rebinding并不会将之前的变量回收
   ```rust
   let x = String::from("abc");
   let x = String::from("efg");
   ```
   如果这两行代码在一个函数中的话，当函数return时，栈帧被释放，析沟的顺序是`efg`先析构，然后才是`abc`，也就是说x被rebind时并不会导致
   之前的东西立刻被析构。

2. 关于lifetime
   以前在学rust的时候看到生命周期老是过分的关注borrow的声明周期，而忽略了和所有权有关的那一部分。rust和其他编程语言的一大不同之处就是
   它其中的变量的声明周期并不仅局限于声明变量或者说发生绑定时的那个作用域，因为有所有权的存在，你可以将所有权传递，从而延伸变量的生命
   周期.



##### 4.5.3
1. `std::marker::Copy`是继承于`std::clone::Clone`的，也就是说任何想要实现copy这个trait的东西必须先实现clone。这就是derive时要同时de
   rive clone 和 copy。


