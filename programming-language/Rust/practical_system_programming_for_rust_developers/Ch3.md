1. rust的std大概分为这么几个部分：
   1. primitive types: 这是最基础的数据类型，数，布尔值，char，数组等等
   2. core crate: 是在primitive types的基础上建造的，与操作系统无关的一些东西
   3. alloc crate: 这部分内容涉及到了heap内存的分配，那些和heap相关的智能指针，都在这里面，不过alloc中的东西又被re-exported到
      std中的。
   4. modules：rust的std按不同的功能分成了不同的module，比如关注于并发的`std::thread`。不论底部多么复杂，提供给程序员的，是按
      功能分类的一个又一个的module。

2. `std::error`这个module是和错误有关的，其中有一个trait`std::error::Error`，任何错误的类型均需要实现这个trait。

3. ffi不只可以和其他语言通信，还可以和下面的系统调用通信。










