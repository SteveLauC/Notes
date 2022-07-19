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

   Once it has reached 32767, the process ID counter is reset to 300, rather than
   1. This is done because many low-numbered process IDs are in permanent use by 
   system processed and daemons, and thus time would be wasted searching for an
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
   the process from accidentally modifying this segment, it is read-only. And for the
   reason that a program can construct many processes, this segment is 
   shareable

   * initialized data segment: Explicitly initialized global and static variables.
   This is read from the executable file.

   * uninitialized data segment: Global and static variables that are not explicitly
   initialized. This segment is usually called the *bss segment* due to historical
   reasons. Before starting the process, the system initializes this segment to 0.
   In the executable file, there is mere space allocated for this segment cause 
   they are not initialized, and it is mainly allocated(more space for the value) 
   at runtime

   * stack

   * heap

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-20%2007-51-00.png)
