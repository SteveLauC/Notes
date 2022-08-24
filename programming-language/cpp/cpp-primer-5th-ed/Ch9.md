#### Ch9: 顺序容器

1. 顺序容器类型以及其在Rust中的对照
  
   * vector: std::vec::Vec
   * deque: (not a continuous memory container) std::collections::VecDeque 
     > Rust中的VecDeque也是不连续的，甚至它提供了一个方法依VecDeque的值返回连续
     > 的slice
     > ```rust
     > pub fn make_contiguous(&mut self) -> &mut [T]ⓘ
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
