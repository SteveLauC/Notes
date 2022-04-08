1. coercion和subtyping很像，但其有显著的区别，在于coercion会改变类型，而subtyping
   则不会。更具体地讲，当执行coercion时，编译器会插入一些代码来实行底层类型的转换
   ，但subtyping则仅仅是编译器的一项检查罢了。

   > 诡异的是，我们在这里区分coercion和subtype的区别。但rust reference里coercion
   第一条就是subtype: Coercion is allowed between the following types: T to U if 
   T is a subtype of U (reflexive case)


2. variance决定paramter的subtyping如何决定最终类型的subtyping。如果最终类型有很多
   parameter，那么分别计算其对最终类型的影响

