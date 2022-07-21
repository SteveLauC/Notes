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

   You may note that the top most area stores cmd arguments and environment 
   variables, that's where `av` and `environ` point to.

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

    MMU is the hardware used to translate between virtual memory and physical 
    memory, it relies on page table to make decisions about which page to in 
    or out.

7. Stack and stack frame
   
   In most implementations, the stack won't decrease in size after these frames 
   are dealloacted. And the memory is simply reused when new stack frame is 
   allocated.

   Kernel stack: the kernel maintains a per-process memory region in the kernel
   space that is used for execution of functions called internally during the
   execution of a syscall. And when we say stack, we are talking about the user
   stack instead of kernel stack

8. cli arguments
   
   ```c
   #include <assert.h>
   int main(int ac, char *av[]) {
   	assert(av[ac]==NULL);
   }
   ```

   `av` is NULL terminated.

   ![d](https://github.com/SteveLauC/pic/blob/main/photo_2022-07-21_07-36-47.jpg)


   Some hacky ways to get arguments:
   
   1. read file `/proc/self/cmdline`, all arguments are stored in a single line,
   with each terimated by a '\0' byte.

      ```rust
      use std::{fs::File, io::Read};
    
      fn main() {
           let mut cmdline: File = File::open("/proc/self/cmdline").expect("open");
    
           let mut buf: String = String::with_capacity(512);
           cmdline.read_to_string(&mut buf).unwrap();
    
           buf.split('\0').for_each(|str: &str| println!("{}", str));
      }
      ```

      ```shell
      $ cargo run 1 2
      target/debug/t
      1
      2
      ```

   2. GNU C library provides two global variables to access the first arugment
      
      ```c
      #define _GNU_SOURCE
      
      #include <stdio.h>
      #include <stdlib.h>
      #include <errno.h>
      
      int main(void)
      {
	      printf("%s\n", program_invocation_name);
	      printf("%s\n", program_invocation_short_name);
	      return EXIT_SUCCESS;
      }
      ```
   
      ```shell
      $ gcc main.c
      $ ./../c/a.out
      ./../c/a.out
      a.out
      ```
   
   There is an upper limit on the maximum number of bytes that can be stored
   in the top most area:

   ```c
   #include <stdio.h>
   #include <stdlib.h>
   #include <linux/limits.h>
   
   int main(void)
   {
    printf("%d\n", ARG_MAX);
    return EXIT_SUCCESS;
   }
   ```
   ```shell
   $ gcc main.c && ./a.out
   131072
   ```

   On Linux, this limit can be changed via the `RLIMIT_STACK` variable

9. hacky way to get environment variables, read `/proc/self/environ`
	
   All the environment variables are stored in a single line, wich each separated
   by a `\0` byte. This is pretty similar to the hacky way to get `cmdline`


   Well, there is a way I have never seen before:

   ```c
   int main(int ac, char *av[], char *envp[]) {
   	char **p = envp;
	while(*p != NULL) {
		puts(*p);
		p += 1;
	}
	return 0;
   }
   ```

   It works like how we iterate over variables using `environ` but `environ` is
   global and `envp` is only local to the main function. And this method is not
   a standard way, so don't do it.

   C has a function `getenv` to get the particular environment variable, do not
   modify it.
   > As typically implemented, getenv() returns a pointer to a string within the 
   environment list.  The caller must take care not to modify this string, since 
   that would change the environment of the process.

10. `putenv(char *str)`
	
    ```c
    #include <stdlib.h>

    int main(void) {
	char new_env[] = "a=b";
	putenv(new_env);
        // we are actually modifying the environment variable
	new_env[0] = 'A'; 
    }
    // When the stack frame of main is deallocated, the pointer stored in environ
    // becomes a dangling pointer
    ```

   What `putenv(char *str)` actually does is to add str(the address) to `environ`
   instead of storing the contents of this key-value pair to that space. So the 
   subsequent modification to `str` will effect the envionment variables. And 
   `str` has to be static cause `environ` will have dangling pointer if `str` is 
   auto

   IF YOU WANNA MODIFY ENVIRONMENT VARIABLES, USE `setenv(char *name, char *value,
   int overwrite)` INSTEAD

