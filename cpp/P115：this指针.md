我们已经知道类的非静态成员函数是放在code area中的，**那这节课我要说的是**，**这个函数只会在data area里放一份**，也就是说，**这个类的所有实例用的函数都是data area里的这一个函数**，大家**共用一份**。

> 好家伙，原来非静态成员函数也是一份啊，以前只知道静态成员函数只是一份

那么this指针是用来干嘛的呢，设想这样一个场景，所有的实例共用一个func函数(见code snippet1)，假设有两个实例p0/p1，p0调用func()，编译器怎么知道要改的是p0的mAge而不是p1的mAge呢，这就是用this指针做到的，this指针指向的是调用函数的那个对象。谁调用函数，this指针指向谁，而且是非静态的成员函数，编译器自动生成this指针，不需要我们去手动写这个东西

> 静态成员函数中编译器没有提供this指针
>
> ```cpp
> //code snippet6
> class Animal{
> public:
>     int m_age = 0;
>     static int func(){
>         this->m_age = 0;
>     }
>     
> };
> //代码第6行报错
> ```
>
> 

```cpp
//code snippet1
class Person{
public:
    int mAge;
    void func(int age){
        mAge = age;
    }
};
```



我们这节课介绍this指针的两个用途

1. 当函数的形参和成员变量重名时，可以用this指针来加以区分

   ```cpp
   //code snippet2
   #include <iostream>
   using namespace std;
   
   class Person{
   public:
       int age;
       Person(int age) {
           age = age;
       }
   };
   
   int main(void){
       Person p(10);
       cout << p.age << endl;
       return 0;
   }
   
   
   //运行结果 254201893
   //这就是age = age 和 形参中的age都是一个age
   //并没有给p.age赋值 所以 是内存里的垃圾值
   ```

   这个时候就得使用this指针 有点像swift里的self.xxx之类的东西

   ```cpp
   //code snippet3
   class Person{
   public:
       int age;
       Person(int age) {
           (*this).age = age; //又因为this是一个指向class的类 this->age的语法也可以
       }
   };
   ```

2. 在类的非静态成员函数返回对象本身时，使用this指针，return *this，返回引用

   ```cpp
   //code snippet4
   #include <iostream>
   using namespace std;
   
   class Person{
   public:
       int age;
       Person& returnTheInstance(){
           return *this;
       }
   };
   
   
   int main(void) {
       Person p0;
       Person &p1 = p0.returnTheInstance();
       cout << &p1 << " " << &p0 << endl;
       return 0;
   }
   
   //运行结果
   //0x7ffee8abfb48 0x7ffee8abfb48
   ```

   先来个代码看下如果用this指针返回对象本体，这里用到了引用，p1和p0是都是那一个东西

   ```cpp
   //code snippet5
   #include <iostream>
   using namespace std;
   
   class Person{
   public:
       int age;
       Person& returnTheInstance(){
           return *this;
       }
   
       Person& add10ToAge(){
           age+=10;
           return *this;
       }
   };
   
   
   int main(void) {
       Person p0;
       p0.age = 10;
       p0.add10ToAge().add10ToAge().add10ToAge(); //链式编程思想
       cout << p0.age << endl;
       return 0;
   }
   
   //运行结果 40 加了3次10
   ```

   链式编程思想也算是得益于**引用**才可以实现吧，你想一下

   假如我们没有引用，想要创建两个地址相同的实例，有点困难？
   
   感觉这是编译器层面的东西，在源代码层面不好搞