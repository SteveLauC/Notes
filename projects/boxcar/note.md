1. Why `std::vec::Vec` can not be lock-free when it comes to the push operation

   1. Memory Layout

      `std::vec::Vec` is a chuck of continuous memory, when it is full, it will 
      allocate a block of memory which is twice big than the previous one, so 
      that all references to the previous memory are dangling, it has to ensure
      that no one is holding references to the previous memory.

   2. Adding new elements will increase the `len` field, which is a `usize` in 
      `std::vec::Vec`'s implementation.

      Multiple threads cannot increment it safely.

2. How can we make `std::vec::Vec` lock-free?

   1. It has to be some kind of "linked list" so that we can allocate memory for
      new elements without invaliden the previous allocated memory.

      Linked list here is quoted because it is not necessaryly to be a real linked
      list, I mean linked list does not support random access, and a Vector, I 
      believe, has to support it.

      Another approach is, we don't allocate new memory by making it a fixed-length
      array.

   2. The `len` field, or let's call it `counter`, has to be an `AtomicXXX` type
      so that multiple threads can add it safely.

3. Let's start with the simpiest one


