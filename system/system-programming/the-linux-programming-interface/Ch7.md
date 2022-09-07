#### Ch7: Memory allocation

> 1. program break: the mechanism behand `malloc`
> 2. two syscalls used to adjust the program break (brk and sbrk)
> 3. aligned memory
> 4. alloca(3) allocate memory on the stack, VLC in C.

1. what is program break

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-02_09-40-52.jpg)

   `Program Break` initially is the first address after `bss section`, and when 
   dynamic allocation is involved, it will change to the end of `heap`. 

   Everybody is saying `program break` is the end of `data section`, idk why.

   You can use `brk()` to reset `program break` to a new value, but the physical
   memory is not allocated after the call of `brk`. The kernel automatically 
   allocates new physical pages on the first attempt by the process to access
   addresses in those pages.

   ```c
   #include <unistd.h>
    
   int brk(void *addr);
   ```

   Since virtual memory is allocated in units of pages, `addr`(the argument of
   `brk`) is automatically rounded up to the next page boundary.

   Setting `program break` below its initial value is likely UB

   And the inital value of `program break` is a random value cause [`Address 
   space layout randomization`](https://en.wikipedia.org/wiki/Address_space_layout_randomization)

2. `sbrk`: increment the `program break` by `increment`
  
   ```c
   #include <unistd.h>
    
   void *sbrk(intptr_t increment); // intptr_t is isize
   ```

   If `increment` is set 0, `sbrk` can be used to query the current `program break`
   value

3. `malloc` is implemented using `mmap` syscall, though on some systems, they
   are implemented with `brk`.

   The behavior of `malloc(0)` is implementation-defined, on linux, it will return
   a small piece of memory that should be freed.

   [what-does-malloc0-return](https://stackoverflow.com/questions/2132273/what-does-malloc0-return)

4. `free(void *ptr)`

   In some cases, instead of reducing the `program break`, `free` adds the 
   memory block to be deallocated to a list of free blocks that are recycled 
   by future calls to `malloc`.

   This is done for few reasons:
   1. The memory being freed may not be in the end of heap but in the middle.
   2. This can reduce the number of syscalls


   We can test this using `free_and_sbrk`, for example:
   ```shell
   $ ./free_and_sbrk 1000 10240 1 1 2
   Initial program break:       0x56498b9f2000
   Allocating 1000*10240 bytes
   Program break is now:       0x56498c3ba000
   Freeing blocks from 1 to 2 in steps of 1
   After free(), program break is       0x56498c3ba000 # program break does not decrease
   ```

   But if we deallcate the memory block which is at the end of heap, then `sbrk` is 
   involved to decrease the `program break`.
   ```shell
   $ ./free_and_sbrk 1000 10240 1 500 1000
   Initial program break:       0x557688953000
   Allocating 1000*10240 bytes
   Program break is now:       0x55768931b000
   Freeing blocks from 500 to 1000 in steps of 1
   After free(), program break is       0x557688e34000
   ```

   This is a kind of optimization, I guess.

5. implementation of `malloc` and `free`

   * malloc:(glibc memory allocator is ptmalloc2)
     1. scans the list of memory blocks previously released by `free()` in order
     to find one whoese size is larger than or equal to its requirements.
        1. If found and the block is exactly fit, return it. If it is larger, split
        it and return.
        2. If no block is found, then call `sbrk` to allocate more memory. To
	reduce the number of syscalls, `sbrk` normally allocate several pages
	and put the unneeded ones into the free list.
   malloc

   * free: 
     1. If we can decrease the `program break` to deallocate the memory block,
     then call `sbrk`.
     2. If we can't, put the allocated block onto the free list.

   > How does `free()` know the size of allocated memory block?
   > When `malloc` allocates memory, it will ask some extra bytes to store the length
   > of that block.
   > ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-05_08-25-57.jpg)
   > And the address returned by `malloc` actually is not the beginning address of
   > the allocated block
   
   > When a block is put on the (double linked) free list, `free()` uses the bytes
   > of the block itself to track the previous and next node.
   > ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-05_08-47-18.jpg)
   
   > You may wanna retrieve the size yourself, like this [one](https://stackoverflow.com/questions/5451104/how-to-get-memory-block-length-after-malloc)
   > But this is implementation defined, you have to know the detail of the implementaion
   > of glibc `malloc` to correctly obtain it.

6. debugging tool for observing heap allocation
   
   page 146

7. Other functions to allocate memory on the heap

   * void *calloc(size_t nmemb, size_t size): this function allocates memory 
   for an array of type `[T; size]` where `T` has size `nmemb`. And this is similar
   to `malloc(nmemb * size)` except that the overflow of `nmemb*size` can 
   be detected in `calloc`

   * void *realloc(void *ptr, size_t size): resize the size of memory pointed by
   `ptr` to `size`, return the newly allocated memory address. The return value
   and `ptr` may have the same value (if not allocated), you should not rely on
   it.


8. allocate aligned memory
   
   In C, you can use `aligned_alloc` (since c11) and `posix_memalign`.

   In Rust, memory allocation is aligned by default

   There is a funny [post](https://stackoverflow.com/questions/227897/how-to-allocate-aligned-memory-only-using-the-standard-library)
   asking to allocate 1024 bytes which is 16 bytes aligned.

9. allocate memory on the stack
  
   ```c
   #include <alloca.h>

   void *alloca(size_t size);
   ```

   This is done by modifying the stack pointer

   And stack allocation has several advantages over the heap:
   1. It is faster cause all it has done is simply adjusting the sp
   2. Does not need to be freed.

   > Variable-length array in C99 is implemented using this.
