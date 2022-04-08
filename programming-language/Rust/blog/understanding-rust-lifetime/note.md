1. coercion和subtyping很像，但其有显著的区别，在于coercion会改变类型，而subtyping
   则不会。更具体地讲，当执行coercion时，编译器会插入一些代码来实行底层类型的转换
   ，但subtyping则仅仅是编译器的一项检查罢了。

   > 诡异的是，我们在这里区分coercion和subtype的区别。但rust reference里coercion
   第一条就是subtype: Coercion is allowed between the following types: T to U if 
   T is a subtype of U (reflexive case)


2. variance决定paramter的subtyping如何决定最终类型的subtyping。如果最终类型有很多
   parameter，那么分别计算其对最终类型的影响


3. `'longer`是`'shorter`的子类型，由于`&'a T`对于`'a`是covariance，所以`&'longer T`
   是`&'shorter T`的子类型


4. lifetimes enable limited subtyping on types that are parameterized with lifetimes.
   
   > 好奇在rust里面，带有lifetime parameter的type有哪些?
   嗯，现在知道了
   


5. 具有多个生命周期参数的类型判断子类型
    
   1. transform方法，使用了`F(G(x))`这样一层一层的结构，需要一层一层地判断，从内到外。
   
   比如`Box<&'longer str>`是否是`Box<&'shorter str>`的子类型？  
   我们从里到外来分析，`'longer`是`'shorted`的子类型，由于`&'lifetime T`对于`lifetime`
   是covariant的，所以`&'longer str`是`&'shorted str`的子类型。又由于`Box<T>`对于`T`是
   covariant的，所以`Box<&'longer str>`是`Box<&'shorter str>`的子类型。

   再比如`Cell<&'longer bool>`是不是`Cell<&'shorter bool>`的子类型，利用上一个推导的过
   程，我们已经知道`&'longer T`是`&'shorter T`的子类型，所以`&'longer bool`是`&'shorter
   bool`的子类型，又由于`Cell<T>`对于`T`是invariant的，所以它俩之间不是父子也不是子父的
   类型关系。

   > 虽然可以推导子类型了，但仍要有一层宏观上的是否出现悬垂指针的感觉，而不是去细扣代码
   中谁是谁的子类型，比如下面的代码片段
   ```rust
   fn main() {
       let longer_str = String::new();
       let mut longer_box = Box::new(&longer_str);
       {
           let shorter_str = String::new();
           let shorter_box = Box::new(&shorter_str);
           longer_box = shorter_box;
       }

       println!("{}", longer_box);
   }
   ```
   可以看出来，`longer_box = shorter_box`是将父类型赋给了子类型。但也要看出来如果赋值成功
   将父类型赋值给了子类型，那么`longer_box`中拿到的就是悬垂指针了

   2. struct/enum/union则使用第二种方法，GLB，greatest lower bound  
   这部分这文章基本没有讲....


6. 已经知道`covariant`和`invariant`的区别，是前者具有subtyping的能力，而后者没有，所以在
   对lifetime parameter实例化的时候，如果要实例化的生命周期参数是covariant的，并且有多个
   引用传给这个参数用来实例化，那么会取最短的生命周期。如果是invariant的话，那么invariant
   的那个部分就直接被赋予了这个参数的生命周期
    
   ```rust
   struct MutStr<'a >{
       s: &'a mut &'a str,
   }

   fn main() {
       let mut s: &'static str = "hello";

       MutStr{
           s: &mut s,
       };
       println!("{}", s);
   }
   ```

   比如上面这个代码，结构体字段`&'a mut &'a str`是invariant over`&'a str`的，那么在初始化
   赋值的时候，`&mut s`，`s`是invariant的部分，被初始化为了`&'static str`，那么`'a`这个参
   就直接被实例化为`'static`了，所以初始化的部分`&mut s`这里的引用，对`s`的引用的生命周期
   同样是`'a`，但不及`'static`，所以编译报错说s这个东西(引用而不是被硬编码到二进制的字符
   串常量)活得不够久，这是一种思考方式。即遇到invariant的实例化部分，直接将生命周期参数实
   例化为被传入参数的生命周期。

   又或者我们认为在rust初始化`'a`时，总会选择最短的那个生命周期，被传入的对`s`的引用参数和
   `'static`中选择短的`'lifetime_to_s`，将`'a`实例化为`'lifetime_to_s`，然后我们发现我们
   的结构体中初始化语句做了这样的操作`&'lifetime_to_s mut &'lifetime_to_s str = &'lifetime_to_s
   mut &'static str`，因为这个类型是invariant的，两者完全没有subtyping的关系，赋值语句是
   错误的，rustc发现不对，然后告诉你其中短的那个活得不够长

   > 对于上面的两个思路，我感觉第一个更像是程序员依据行为总结的，第二个是borrow checker的做
   法。
