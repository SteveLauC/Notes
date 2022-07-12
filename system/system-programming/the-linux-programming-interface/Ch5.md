1. open的`O_CREAT`和`O_EXCL`正常情况下需要同时使用，如果`path`
   参数指向一个早就存在的文件，那么则会报错(EEXIST)。

   在正常情况下，`O_EXCL`都需要和`O_CREAT`配合使用。但有一个例
   外，在linux 2.6及以后，如果`pathname`指向的是block device，
   则可以去掉`O_CREAT`来使用

   在使用`O_EXCL`和`O_CREAT`调用`open(2)`的过程中，检查`path`
   是否存在以及创建文件是原子性的。

   > create a file exclusively指的是确保在创建这个文件时，此文
   件之前不存在

2. 在rust中exclusively创建一个文件

   `std::fs::OpenOptions`中有

   ```rust
   pub fn create_new(&mut self, create_new: bool) -> &mut Self
   ```

3. `open(1)`操作的原子性

    * append mode: when `O_APPEND` is set, the operation of `seeking to the end of`
    file and `writing bytes` is uninterruptable
    * create file exclusively: when `O_CREAT` and `O_EXLC` are both set, detecting
    the existence of a file and creating it are executed as an atomic step

4. what is race condition

   when the result of a series operations is dependent on the order of scheduling
   of the processes, this is a race condition

   > NFS does not support `O_APPEND` flag, thus using `O_APPEND` on such a fs may
   elicit data corruption. If you are using `O_APPEND` on NFS, it will just emulate
   the behavior using `lseek;write`.

5. fetch access mode

   > If you forget about what is `access mode` and the categories of `flag`,
   here you go[link](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch4.md)

   getting access mode is a little bit complex cause `O_RDONLY` has value 0

   ```
   #define O_ACCMODE	   0003
   #define O_RDONLY	     00
   #define O_WRONLY	     01
   #define O_RDWR	     02
   ```

   thus, you can not simply do the following thing:

   ```c
   int flag = fcntl(fd, F_GETFL);
   // & 0 will always return in 0
   if (flag & O_RDONLY) {
   	printf("The file is opened for reading\n");
   }
   ```

   Instead of using and operation, we first mask `flag` with `O_ACCMODE` and then 
   compare it with `O_RDONLY`

   ```c
   int flag = fcntl(fd, F_GETFL);
   int flag_after_masking = flag & O_ACCMODE;

   switch (flag_after_masking) {
   case O_RDONLY:
   	printf("read\n");
	break;
   case O_WRONLY:
   	printf("write\n");
	break;
   case O_RDWR:
   	printf("read and write\n");
	break;
   }
   ```

6. relationship between fd and open files

   Multiple file descriptors can refer to the same open file, and this is the mechanism
   behind redirection.

   There are three data structures maintained by the kernel we need to understand:
   
   * the per-process file descriptor table
   * system-wide table of open file descriptions
   > A table of all the open files
   * file system i-node table

   ![illustration](https://github.com/SteveLauC/pic/blob/main/relation_between_fd_and_open_files.jpeg)

   In the above diagram, fd 1 and 20 of process A both refer to the open file 
   labeled by 23. This could happen after a call to `dup`. Fd 2 of process A and
   fd 2 of process refer to the same open file labeled by 73, this could occur
   after a call to `fork`(i.e. process A is the parent process of process B, or 
   vice versa), or if one process passed an open file descriptor to another process
   using a UNIX domain socket

   Fd 0 of process A and fd 3 of process B refer to different open file descriptions
   but these two file descriptions do refer to the same I-node entry
   > Currently, I don't understand why does this happen.(Page 96)

   What conclusions we can draw from the processing diagram:
   1. Two different fds refer to the same open file description share a file `offset`
   value(as this is recorded in the `system-wide open file descriptions table`).
   2. Two different fds refer to the same open file description also share a status flag
   3. By contrast, the file descriptor flag is private to the fd

7. redirect stderr to stdout

   ```shell
   $ ./script 2>&1
   ```

   `&` before `1` is kind of `escape character` I guess cuase if you directly use
   `1` it will be redirected to the file named `1`


8. `$ ./script > output 2>&1`
    Note that the above command won't open file `output` twice cause it will result 2 
    different file descriptors which do not share a same offset, writing to such fds
    will elicit data corruption
