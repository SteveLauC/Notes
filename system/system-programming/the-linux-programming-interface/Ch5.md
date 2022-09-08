#### Ch5: File I/O Further Details

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

9. duplicate the fd

   * dup2

     ```c
     int dup2(int oldfd, int newfd);
     ```

     You can specify the new fd instead of being allocated by the os. If the new
     fd specified by `newfd` is already open, it is silently closed before 
     duplication. Any error encountered during the `close` will be **ignored**

     When `oldfd` equals `newfd`, `dup2` does nothing.

   * fcntl

     ```c
     newfd = fcntl(oldfd, F_DUPFD, startfd);
     ```
     
     This call makes a duplicate fd using the lowest unused fd greater than or euqal
     to `startfd`. This is useful when we wanna ensure that the new fd is in a certain
     range

   * dup3(linux-specific)
   Allow us to control the `fd flag` of the new duplicate file descriptor
 
       ```c
       int dup3(int oldfd, int newfd, int flags);
       ```

   Currently, the `flags` argument supports only one flag(`O_CLOEXEC`)

   > duplicate file descriptors share the same file offset and status flag 
   cause they refer to the same open file description. But they do not share
   `fd flags` for the reason that this flag is private to the fd.
   

10. read or write at a specific offset

    ```c
    ssize_t pread(int fd, void *buf, size_t count, off_t offset);

    ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
    ```

    There two syscalls operate just like `read(2)/write(2)` except that they
    read or write at a specific offset insead of current offset.

    Also note, seeking to `offset` and read/write are performed as an atomic
    operation.

11. scatter-gather I/O(vectored I/O)


    ```c
    #include <sys/uio.h>

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
    ```

    There functions transfer a cluster of buffers to the file `fd`, rather than
    a single buffer.

    ```c
    struct iovec {
        void  *iov_base;    /* Starting address */
        size_t iov_len;     /* Number of bytes to transfer */
    };
    ```

    Argument `iov` is an array of struct `iovec` containing the starting address
    and buffer length of those buffers. The number of buffers is specified in
    `iovcnt`

    example of `readv`

    ```c
    #include <stdio.h>
    #include <assert.h>
    #include <sys/uio.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <fcntl.h>
    
    int main(void)
    {
	    int fd = open("test", O_RDONLY);
    
	    if (fd == -1) {
		    fprintf(stderr, "can not open");
		    exit(1);
	    }
    
	    char buf1[10];
	    char buf2[10];
	    struct iovec buffers[] = {
		    { buf1, sizeof(buf1) },
		    { buf2, sizeof(buf2) },
	    };
    
	    ssize_t n_read = readv(fd, buffers, 2);
	    assert(n_read == 20);
    
	    write(1, buf1, sizeof(buf1));
	    write(1, buf2, sizeof(buf2));
	    close(fd);
	    return EXIT_SUCCESS;
    }
    ```

    Scatter-gather IO in rust

    ```rust
    // from `std::io::Read`
    fn read_vectored(&mut self, bufs: &mut [IoSliceMut<'_>]) -> Result<usize>
    // from `std::io::Write`
    fn write_vectored(&mut self, bufs: &[IoSlice<'_>]) -> Result<usize>
    ```

    > Scatter input or output is performed as an atomic operation

12. perform scatter I/O at a specific offset

    ```c
    ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
    ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
    ```

    > Added in kernel 2.6.30

13. truncate a file to a specific length
   
    ```c
    #include <unistd.h>
    #include <sys/types.h>

    int truncate(const char *path, off_t length);
    int ftruncate(int fd, off_t length);
    ```

    If the file is longer than `length`, then the excess data is lost. If the file
    is shorter than `length`, then it is extended with bytes null or hole.

    The file offset is untouched

14. Non-blocking I/O
	
    when `O_NONBLOCK` is used inside `open`, it has 2 functionalities:
    1. if the file can not be opened immediately, the `open` will return without
    block.
    2. if the file is successfully opened, then the subsequent I/O operations
    performed on this file are also non-blocking.

    > O_NONBLOCK is ignored on regular files as the kernel buffer cache ensures
    that I/O on regular file does not block.(One exception: when file is being 
    )

15. I/O on large files

    For a 32-bit system, it uses a 32-bit number to represent file size and its
    offset. Normal, such a type is named `off_t` in 1386-linux.

    ```c
    struct stat{
    	off_t st_size;
    }

    off_t lseek(int fd, off_t offset, int whence);
    ```
  
    `off_t`, an alias to `i32`, whose max value is `2GB`, limits the max file
    size to 2GB. If you wanna create a file larger than 2GB(large files), you 
    need Large File Summit(LFS)

    > LFS is created by OS vendors

    > 64-bit arch does not have the problem LFS tries to address because the capacity
    of most disks used in such arch is smaller than the file size limit.

    
    Approaches to perform I/O on large files:
    1. use the transitional LFS api(obsolete)
    2. define `_FILE_OFFSET_BITS` to `64`(recommended)


    > Rust does not have such problem cause even though in a 32-bit system, they
    still use 64-bit number to represent file size and offset.
    > [link](https://docs.rs/rustc-std-workspace-std/latest/i686-unknown-linux-gnu/std/io/prelude/trait.Seek.html)

    If we define `_FILE_OFFSET_BITS` to 64, all the native I/O apis will be converted
    to the LFS version(e.g. open to open64). And if we define this whne compiling the
    code instead of using it in our source code like:
    ```c
    gcc -D_FILE_OFFSET_SET=64 main.c
    ```
    Then the source code can be used on both 32-bit and 64-bit arches without
    any portability issues.


16. `/dev/fd`

    Ther kernel maintains such a diretory for each process using soft link(link
    to `/proc/self/fd`, which is also a soft link to `/proc/PID/fd`)
    The files under this directory is named `n` where n is an open file descriptor.

    If we open such a file using `fd = open()` then we are actually duplicating it.

    ```c
    fd = open("/dev/fd/0", O_RDONLY);
    // is equivalent to 
    fd = dup(0);
    ```

    ```shell
    $ cd /dev/fd
    $ l
    Permissions Size User  Group Date Modified Name
    lrwx------@   64 steve steve  8 Sep 06:18  0 -> /dev/pts/8
    lrwx------@   64 steve steve  8 Sep 07:26  1 -> /dev/pts/8
    lrwx------@   64 steve steve  8 Sep 07:26  2 -> /dev/pts/8
    lrwx------@   64 steve steve  8 Sep 07:26  10 -> /dev/pts/8
    
    $ cd ..
    $ l|grep fd
    lrwxrwxrwx@     13 root  root     8 Sep 06:16  fd -> /proc/self/fd

    $ l stdin
    Permissions Size User Group Date Modified Name
    lrwxrwxrwx@   15 root root   8 Sep 06:16  stdin -> /proc/self/fd/0
    
    $ l stdout
    Permissions Size User Group Date Modified Name
    lrwxrwxrwx@   15 root root   8 Sep 06:16  stdout -> /proc/self/fd/1
    
    $ l stderr
    Permissions Size User Group Date Modified Name
    lrwxrwxrwx@   15 root root   8 Sep 06:16  stderr -> /proc/self/fd/2
    ```

    ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-09-08%2008-29-43.png)

    > For more information about `/proc/self/fd`, 
    > go [here](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch12.md)

17. weird behavior when `ls` `/proc/self/fd`
   
    `/dev/fd` is a softlink to `/proc/self/fd` so we will use the letter here.

    ```shell
    $ cd /proc/self/fd
    $ ls -l
    total 0
    lrwx------ 1 steve steve 64 Jul 16 09:59 0 -> /dev/pts/1
    lrwx------ 1 steve steve 64 Jul 16 09:59 1 -> /dev/pts/1
    lrwx------ 1 steve steve 64 Jul 16 09:59 10 -> /dev/pts/1
    lr-x------ 1 steve steve 64 Jul 16 09:59 13 -> /usr/share/zsh/functions/Completion.zwc
    lr-x------ 1 steve steve 64 Jul 16 09:59 15 -> /usr/share/zsh/functions/Completion/Base.zwc
    lr-x------ 1 steve steve 64 Jul 16 09:59 16 -> /usr/share/zsh/functions/Misc.zwc
    lr-x------ 1 steve steve 64 Jul 16 09:59 17 -> /usr/share/zsh/functions/Zle.zwc
    lr-x------ 1 steve steve 64 Jul 16 09:59 18 -> /usr/share/zsh/functions/Completion/Zsh.zwc
    lr-x------ 1 steve steve 64 Jul 16 09:59 19 -> /usr/share/zsh/functions/Completion/Unix.zwc
    lrwx------ 1 steve steve 64 Jul 16 09:59 2 -> /dev/pts/1
    lr-x------ 1 steve steve 64 Jul 16 09:59 20 -> /usr/share/zsh/functions/Completion/Darwin.zwc

    $ ls -l /proc/self/fd
    total 0
    lrwx------ 1 steve steve 64 Jul 16 10:13 0 -> /dev/pts/1
    lrwx------ 1 steve steve 64 Jul 16 10:13 1 -> /dev/pts/1
    lrwx------ 1 steve steve 64 Jul 16 10:13 2 -> /dev/pts/1
    lr-x------ 1 steve steve 64 Jul 16 10:13 3 -> /proc/1507812/fd
    ```

    The reason why the outputs of the above two commands are different is that
    `/proc/self` is a magic directroy. When `ls` is executed without `path` argument,
    and the current working dir is `/proc/self/fd`, seems it will print the open 
    file descriptors of its parent process(i.e. the shell)

18. make a temporary file

    ```c
    int mkstemp(char *template);
    FILE *tmpfile(void);
    ```

    * mkstemp:
    The argument `template` is a modifiable string which must ends with `XXXXXX`
    , like `char tmp[]="/tmp/a_file_XXXXXX"`. And the last 6 characters will be
    modified to generate a unique temporary file name. And the tempoprary file 
    will be created as `open(template, O_CREAT|O_RDWR|O_EXCL, 0600)`, note that
    the `O_EXCL` flag warrants that this file is created exclusively, and the
    mode of this file is `rw-------`.
    ```c

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    
    int main(void)
    {
	    char tmp_file_name[] = "/tmp/tmp_file_XXXXXX";
	    int fd = mkstemp(tmp_file_name);
	    if (fd == -1) {
		    perror("mkstemp");
		    exit(EXIT_FAILURE);
	    }
    
	    printf("The generated unique file name is %s\n", tmp_file_name);
    
	    close(fd);
	    return EXIT_SUCCESS;
    }
    ```

    * tmpfile is just like `mkstemp` but returns a FILE stream.

19. unlink(2)
	
    delete a `file name` from the file system. If this name is the last link referring
    to this file and no processes have the file open, the space used by it is deallocated.
