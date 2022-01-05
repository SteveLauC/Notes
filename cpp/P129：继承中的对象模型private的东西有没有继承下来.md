```cpp
//code snippet1
#include <iostream>
#include <string>
using namespace std;

class Parent{
public:
    int m_public;
protected:
    int m_protected;
private:
    int m_private;
};

class Child: public Parent{
public:
    int m_child;
};



int main(void){
    Child c;
    cout << sizeof c << endl;
    return 0;
}

//results: 16
```

为什么子类的大小是16呢，父类有3个int，子类有1个int，16

这说明父类的private的东西子类也继承到了，只是编译器不允许你去访问它

是编译器在从中约束你



非静态的成员属性全部继承下来了