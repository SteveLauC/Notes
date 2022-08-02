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
