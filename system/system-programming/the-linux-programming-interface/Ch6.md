1. executable file formats: 
	1. assember output(a.out, this is the file format, not a execatable file name)(obsolete)
	2. common object file format(COFF)(obsolete)
	3. executable and linking format(ELF)

2. maximum process ID
	
   On 32-bit Linux, it is 2^15-1 = 32768-1 = 32767. While on 64 bit system, this
   changes to 2^22-1 = 4194304-1 = 4194303

   ```
   # To verify
   $ cat /proc/sys/kernel/pid_max
   4194304
   ```

3. When a new process is created, it is assigned the next sequentially available
   process ID. Each time the limit is reached, the kernel resets its process ID 
   counter so that process IDs are assigned starting from low integer values.

   Once it has reached 32767, the process ID counter is reset to 300, rather than 1
   . This is done because many low-numbered process IDs are in permanent use by 
   system processes and daemons, and thus time would be wasted searching for an
   unused process ID in this range.

   > The default reset value 300, it is correct under 32 bits. Not sure it is still
   applicable on 64 bits.

4. get the parent process id
  
   ```c
   #include <unistd.h>
   pid_t getppid(void);
   ```

   We don't have such a function in Rust std.

5. memory layout of a process

   * text segment: Machine code of the program run by the process. To prevent
   the process from accidentally modifying this segment, it is read-only. And 
   for the reason that a program can construct many processes, this segment is 
   shareable

   * initialized data segment(user-initialized data segment): Explicitly 
   initialized global and static variables. This is read(copied) from the
   executable file.

   * uninitialized data segment(zero-initialized data segment): Global and static 
   variables that are not explicitly initialized. This segment is usually called 
   the *bss segment* due to historical reasons. Before starting the process, the 
   system initializes this segment to 0. In the executable **file**, there is mere 
   space allocated for this segment cause they are not initialized(only need to 
   store how many `unitialized variables` we have instead of storing something 
   like `a = 0`), and it is mainly allocated at runtime

   * stack

   * heap

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2007-51-00.png)

   > size(1) command can list the size of text, initialized-data and bss segments

6. Virtual memory exploits a property that is typical of most 
   programs: locality(局限性，常规译为地区位置) of reference.

   * Spatial locality(空间局限性): is the tendency of a program to reference
   memory addresses that near those that were recently accessed.
   * Temporal locality(时间局限性): is the tendency of a program to access 
   the same memory in the near future

   The upshot or result of this locality is that the kernel can just maintain
   a small piece of phycial address space used by the process so that there 
   will be more free phycial space that can be used by other processes.

   Virtual memory splits the memory used by each program into small, fixed-size 
   units call pages. At any time, only some of the pages need to be loaded into 
   the *phycial memory*. Others are stored in a reserved area of disk called 
   swap area and will only be loaded when necessary.

   In order to support this feature, the kernel maintains a `page table(页表)`
   (mapping from virtual memory to phycial memory and swap area)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-07-20_09-31-45.jpg)

   If the process accesses a page that is not in the page table, it will receive
   `SIGSEGV` signal(segment fault)

    ```c
    #include <stdio.h>
    #include <stdlib.h>
    #include <signal.h>
    
    void catch (int);
    
    int main(void)
    {
	    signal(SIGSEGV, catch);
	    char *hello = "hello";
	    hello[2] = 'A';
	    return EXIT_SUCCESS;
    }
    
    void catch (int sig_num)
    {
	    printf("catched SIGSEGV signal\n");
	    exit(EXIT_FAILURE);
    }
    ```

    ```shell
    $ gcc main.c
    $ ./a.out
    catched SIGSEGV signal
    ```

    MMU is the hardware used to translate between virtual memory and physical memory,
    it relies on page table to make decisions about which page to in or out.
