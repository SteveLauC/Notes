在上一个职工管理系统中，我们创建一个类，类的声明(成员参数和成员函数的声明)写在.h文件中，类的成员函数的实现放在.cpp文件中，但当我们的类不是一个普通的类，而是一个类模版的时候，这样的做法就不可行了

```cpp
//code snippet1
//main.cpp
#include <iostream>
using  namespace std;
#include "person.h"


int main(){
    Person<string, int> p("steve", 18);
    p.show_person();
    return 0;
}
```

```cpp
//code snippet2
//person.h
#pragma once
#include <iostream>

template<class T1, class T2>
class Person{
public:
    Person(T1 name, T2 age);
    void show_person();
    T1 m_name;
    T2 m_age;
};
```

```cpp
//code snippet3
//person.cpp
#include "person.h"
using namespace std;

template<class T1, class T2>
Person<T1, T2>::Person(T1 name, T2 age){
    this->m_name = name;
    this->m_age = age;
}

template<class T1, class T2>
void Person<T1, T2>::show_person() {
    cout << "name: " << this->m_name << "age: " << this->m_age << endl;
}
```

普通的类变成了类模版，编译通过，但run不起来，为什么？

：会链接失败，因为类模版的成员函数只有在调用的时候才会创建

解决方法：

1. main.cpp中不引用person.h而是引用person.cpp，因为这样直接有了函数的实现

2. 将.h文件和.cpp文件放在同一个文件中，文件命名为perosn.hpp，然后在main.cpp就去include这个hpp文件

   (但要去掉person.cpp中的#include "person.h")



> 推荐使用第2种方法，hpp文件约定俗称就是类模版的声明和实现





使用第2种方法解决问题

```cpp
//code snippet4
//main.cpp
#include <iostream>
using  namespace std;
#include "person.hpp"


int main(){
    Person<string, int> p("steve", 18);
    p.show_person();
    return 0;
}
```

```cpp
//code snippet5
//person.hpp

#pragma once
#include <iostream>

template<class T1, class T2>
class Person{
public:
    Person(T1 name, T2 age);
    void show_person();
    T1 m_name;
    T2 m_age;
};

using namespace std;

template<class T1, class T2>
Person<T1, T2>::Person(T1 name, T2 age){
    this->m_name = name;
    this->m_age = age;
}

template<class T1, class T2>
void Person<T1, T2>::show_person() {
    cout << "name: " << this->m_name << "age: " << this->m_age << endl;
}
```



运行成功

```cpp
name: steveage: 18
```

