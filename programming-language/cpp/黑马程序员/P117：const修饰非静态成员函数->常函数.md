## 常函数

什么是常函数，在class的**非静态**成员函数的形参列表的括号后面加一个const关键字，这个函数就变成了常函数，常函数比普通的成员函数多一个限制，**那就是不能修改成员变量的值**。

> 静态成员函数不可以加const修饰字，会报错，因为常函数的底层实现是对this指针进行const限制，而静态成员函数编译器默认没有提供this指针
>
> ```cpp
> //code snippet0
> class Animal{
> public:
>     static void func() const {
>         m_age = 9;
>     }
> };
> //error: Static member function cannot have 'const' qualifier
> ```
>
> 

```cpp
//code snippet1
class Person{
public:
    int m_age = 0;
    void func1() const {  //这是一个常函数 不可以修改m_age，故报错
        m_age = 9;
    }
};
//results：error
```

> 这里解释下为什么常函数不能修改普通的成员变量
>
> 以前提到过，在每一个非静态成员函数内都有一个this指针，以Person类为例，它的原型是Person * const this(所以this指针是不能修改其指向的)，当我们创建一个常函数时，this指针变成了Person const * const this，这次this的指向的那个东西的值也不可以变了，也就是说对这个指针进行解引用并进行修改是不可以的。
>
> 当我们在成员函数里写成员变量时，如code snippet1中的第6行代码，其更本质的书写形式是this->m_age = 9，这就是在对this指针进行解引用修改，不被允许，故报错。

虽说常函数不能修改普通成员变量的值，但当你想修改的话，也是可以的，只要在成员变量的定义前写上mutable就可。

```cpp
//code snippet2
class Person{
public:
    mutable int m_age = 0; //加上了mutable关键字 常函数也可以修改了
    void func1() const {
        m_age = 9;
    }
};

//build succeed
```



## 常对象

常函数是给成员函数加上const关键字，而常对象则是给类的实例加上const关键字。

* 常对象不可以对普通的成员变量进行修改，可以对添加了mutable关键字的成员变量进行修改

* 常对象只能调用常函数

```cpp
//code snippet2
class Person{
public:
    mutable int m_age = 0;
    int m_height = 0;
    void func1() const {
        m_age = 9;
    }
    void func2() {

    }
};

int main() {
    const Person p0;
    p0.m_height = 9; //不可修改 常对象不可以对普通的成员变量进行修改 只可以改mutable的
    p0.func2(); //不可调用 常对象只能调用常函数
    return 0;
}

//results: error
```

