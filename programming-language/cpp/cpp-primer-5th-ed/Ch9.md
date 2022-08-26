#### Ch9: 顺序容器

1. 顺序容器类型以及其在Rust中的对照
  
   * vector: std::vec::Vec
   * deque: (not a continuous memory container) std::collections::VecDeque 
     > Rust中的VecDeque也是不连续的，甚至它提供了一个方法依VecDeque的值返回连续
     > 的slice
     > ```rust
     > pub fn make_contiguous(&mut self) -> &mut [T]
     > ```
     > Rearranges the internal storage of this deque so it is one contiguous
     > slice, which is then returned.
     
     > deque在内存中不连续，但却支持快速随机访问，这是怎么实现的阿?

   * list: std::collections::LinkedList
   * forward_list: 单链表 Rust std 没有单链表
   * [array](https://en.cppreference.com/w/cpp/container/array): [T;n]
     
     注意这个不是C array:

     ```cpp
     template< class T, std::size_t N> struct array; (since C++11)
     ```

     > 这个array有`begin/end/cbegin/cend`方法，虽然返回的也是`T *`.

   * string: std::string::String

   > 除array外，其他类型均具有高效的堆内存管理

2. 容器的典型member type:
   
   ```
   iterator: 迭代器类型
   const_iterator: 只读的迭代器
   size_type: 一种无符号整数，可以保证足够保存最大容器的数量
   difference_type: 有符号整数，保存两个元素之间的距离
   value_type: 容器内元素类型 T
   reference: value_type& 左值类型
   const_reference: const value_type&
   ```

3. 容器典型构造函数

   ```
   C c; 默认构造
   C c1(c2) 拷贝构造
   // array不支持
   C c(b, e) 将迭代器[b, e]内的元素拷贝到新的容器c中
   C c{a, b, c} 列表初始化
   ```

4. 容器的赋值与swap

   ```
   c1 = c2; 将c1用c2覆盖
   c = {a, b, c} 将c1中的元素替换为列表中元素
   c1.swap(c2) 将c1与c2进行替换 
   std::swap(c1, c2)  将c1与c2进行替换 
   ```

5. 容器的大小操作

   ```
   c.size() c中元素的数目 (forward_list不支持此操作)
   c.max_size() 容器中最多可以放置的元素数量
   c.empty() c.size() == 0
   ```

6. 容器的修饰操作

   ```
   c.insert 插入
   c.clear 清除
   c.emplace 使用args来构造pos前面的元素
   c.erase 清除具pos处的元素
   ```

7. 运算符

   ```
   == !=
   < > <= >=  map/unordered_map不支持
   ```

8. 迭代器
   
   ```
   c.begin c.end // const or non-const depends the container itself
   c.cbegin c.cend // const iterator
   c.rbegin c.rend // reversed iterator
   ```
   
   `std::forward_list`不支持`--`运算，因为是单链表。


