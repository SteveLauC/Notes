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

   But if we deallcate the memory block which is at the end of heap:
   ```shell
   $ ./free_and_sbrk 1000 10240 1 1 2
   Initial program break:       0x557688953000
   Allocating 1000*10240 bytes
   Program break is now:       0x55768931b000
   Freeing blocks from 500 to 1000 in steps of 1
   After free(), program break is       0x557688e34000
   ```

   This is a kind of optimization, I guess.