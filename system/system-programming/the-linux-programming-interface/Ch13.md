#### Ch13: File I/O Buffering

> 1. conception: kernel buffer cache and libc buffer
> 2. library functions (setvbuf/setbuf/setbuffer/setlinebuf) used to modify the
     buffering mode of stream buffer (libc buffer)
> 3. library function to flush the buffer (flush(3))
> 4. conception: synchronized I/O data integrity and file integrity
> 5. syscalls for controlling kernel buffer (fsync/fdatasync/sync/syncfs)
> 6. direct I/O (raw I/O)
>    * how to enable it
>    * how to test if your system supports it
>    * alignment restrictions.
> 7. mixing use of syscalls and lib functions (write order issue)

1. kernel `buffer cache`
   
   When performing disk operation, the kernel maintains a memory called 
   `buffer cache` to cache the bytes stream in order to reduce the number
   of disk I/O.

   For example, in the following statement, we are transferring 5 bytes to
   the `buffer cache` rather than directly writing them to the `disk file`.
   Later, when appropricate, the disk will flush these bytes to disk.
   (And therefore, we say disk operation is *not synchronized*)

   ```c
   write(fd, "hello", 5);
   ```

   And for input, the kernel will `prefetch` data from disk to the `buffer cache`
   so that in the subsequent `read`, we can directly read from the `buffer` instead
   of performing disk operation.

   ```c
   read(fd, buf, 5);
   ```

   > To be precise, since Kernel 2.14, Linux no longer maintains a separate
   > `buffer cache`. Instead, file I/O buffers are included in the page cache.
   > But we still use the word `buffer cache` as this is the general terminology
   > in UNIX world.

2. `write(2)`

   ```c
   fd = open(file, O_WDONLY);
   write(fd, "hello", 5);
   ``` 

   The above `write(2)` will return immediately after the `"hello"` is transferred
   to the kernel `buffer cache`. (No disk output performed when `write(2)` returns
   **NOT SYNCHRONOUS**)

   But if we use `O_SYNC` flag, then `write(2)` will return only when the data 
   (and metadata) is writen to the disk. (reaches synchronized I/O file integrity 
   completion) 

   > [Ch4 Note 5 flag arguments of open(2)](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch4.md)

   Another approach to make I/O synchronous is to modify the configuration of
   the underlying file system:
   
   ```c
   // alter the config for file system specified in `device`

   mount(device, target, fs-type, MS_REMOUNT|MS_SYNCHRONOUS, data);
   ```

3. impact of `buffer size` on the performance of I/O 

   `buffer cache` is used to decrease the amount of `disk I/O`, but in our source 
   code, we can empoly a bigger buffer to reduce the number of syscalls.

   ```rust
   /// copy.rs
   /// usage: copy source destination

    use std::env::args;
    use std::fs::{File, OpenOptions};
    use std::io::{Read, Write};
    use std::os::unix::fs::OpenOptionsExt;
    use std::process::exit;
    
    const BUF_SIZE: usize = 1024;
    
    fn main() {
        let av: Vec<String> = args().collect();
        if av.len() != 3 {
            eprintln!("usage: {} old-file new-file", av[0]);
        }
    
        let mut input_file: File =
            File::open(av[1].as_str()).expect(format!("opening file: {}", av[1]).as_str());
        let mut output_file: File = OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .mode(0o666)
            .open(av[2].as_str())
            .expect(format!("opening file: {}", av[2]).as_str());
    
        let mut buf: [u8; BUF_SIZE] = [0; BUF_SIZE];
        while let Ok(num_read) = input_file.read(&mut buf) {
            if num_read > 0 {
                let wrrite_res: Result<usize, _> = output_file.write(&buf[..num_read]);
                if wrrite_res.is_err() || wrrite_res.unwrap() != num_read {
                    eprintln!("could't write whole buffer");
                    exit(1);
                }
            }
    
            if num_read == 0 {
                break;
            }
        }
   }
   ```

   In the above program, we use a buffer of size `1024`, we can try different 
   values to measure the time consumed to copy a big file:

   Here we have two executables with bufsize of `1 bytes` and `1024 bytes` 
   respectively, and we try to copy `log` using these two binaries:

   ```shell
   $ l
   Permissions Size User  Group Date Modified Name
   .rwxr-xr-x@ 4.2M steve steve 13 Sep 10:07  big-buf-cp
   .rw-r--r--@  20M steve steve 13 Sep 11:12  log
   .rwxr-xr-x@ 4.2M steve steve 13 Sep 10:04  small-buf-cp

   $ /usr/bin/time -p ./small-buf-cp log log1
   real 34.30
   user 2.03
   sys 32.23
   
   $ /usr/bin/time -p ./big-buf-cp log log1
   real 0.07
   user 0.00
   sys 0.05
   ```

   We inspect the `program time`, which is the sum of `user time` and `sys time`.
   Notably, you can see the big difference between the `sys time` of these two 
   experiments. Well, `sys time` is the time consumed in `kernel space`, which
   is used for `preparing for syscalls` and `transferring data between user 
   space and kernel space`. That's basically the overhead of calling `read(2)` 
   and `write(2)`.

   > refer to [Ch3 note1: the process of syscall](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch3.md)


   The size of buffer can have a big impact on the performance.


4. libc buffering

   > This is exactly what we did in Note 3, using a bigger buffer to decrease
   > the number of syscalls except this is finished by the standard library 
   > developer.

   These are three types of buffering modes:

   1. unbuffered
   2. block buffered
   3. line buffered

   Any stream pointing to a regular file is `block buffered`. `stdin` and `stdout`
   are `line buffered` when they refer to the terminal. `stderr` is `unbuffered`.
   
   ```c
   #include <stdio.h>

   int setvbuf(FILE *restrict stream, char *restrict buf, int mode, size_t size);

   void setbuf(FILE *restrict stream, char *restrict buf);
   void setbuffer(FILE *restrict stream, char *restrict buf, size_t size);
   void setlinebuf(FILE *stream);
   ```

   The above lib functions can be used to modify the mode of buffering.

   `setvbuf(3)` is the general one and the other functions are based on all 
   `setvbuf(3)`. The `stream` argument identifies the stream to be modified.
   And if you wanna modify the mode, **do it after stream is open and 
   before any I/O operation has been performed on this stream**, this modification 
   will effect all the subsequent I/O functions performed on this
   stream.

   The `mode` argument can be set as:
   1. `_IONBF`: unbuffered
   2. `_IOFBF`: full (block) buffered
   3. `_IOLBF`: line buffered 

   The `buf` and `size` argument can be used in two ways:

   1. If `buf` is non-NULL, then it should points to a block of static or
      heap-allocated memory with size of `size`. (Do NOT allocate it on the
      stack).
   2. If `buf` is NULL, and if `mode` is set to either `_IOFBF` or `_IOLBF`,
      then libc will automatically allocate memory for you (How much space
      will be allocated and will `size` argument to be used to determine the
      size in this process? This is **implementation defined**!. In glibc,
      size is simply ignored). If `mode` is `_IONBF`, then no allocation is
      needed and `size` is ignored.

   
   `setbuf(FILE *restrict stream, char *restrict buf)` is just 
   `setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ)`. You should alloate
   buffer yourself and the size of this buffer should at least be `BUFSIZ`.

   `setbuffer(FILE *restrict stream, char *restrict buf, size_t size)` is just
   `setvbuf(stream, buf, buf ? _IOFBF : _IONBF, **size**)`. The only difference
   between `setbuf` and `setbuffer` is that you can control the buffer size
   rather than using `BUFSIZ` in the latter.

   `setlinebuf(FILE *stream)` is `setvbuf(stream, NULL, _IOLBF, 0)`.

   Just like `setvbuf`, all these 3 functions should be used before any I/O
   operation has been performed on `stream`. And the memory allcoated should
   be static or heap-allocated.


5. flush a stdio buffer
   
   ```c
   #include <stdio.h>

   int fflush(FILE *stream);
   ```

   If the stream argument is NULL, fflush() flushes *all* open output streams.

   If `stream` points to input stream associated with `seekable` files (not pipe
   or terminal), then the contents of buffer will be discarded.

   The buffer will be automatically flushed when the stream is closed.

   In many ligc implementation (including glibc), stdout will be flushed when 
   we read from stdin. Rust std does not follow this.

6. synchronized I/O completion

   SUSv3 defines the term `synchronized I/O completion` to mean an I/O operation
   that has either been successfully transferred to disk or diagnosed as 
   unsuccessful.

   SUSv3 also defines two different kinds of `synchronized I/O completion`:

   1. synchronized I/O data integrity completion:

      * For a read operation: this means the requested data has been transferred
        from the disk to the process. If a pending write operation will affect
        the requested data, it is performed before the read operation according
        to the requirements of `synchronized I/O *data* integrity completion`.

      * For a write operation: this means the data has been transferred to the 
        disk, and all the metadata required in the next retrieval has also been
        transferred to the disk. The key point here is only the **required** metadata
        will be transferred, not all the metadata.

   2. synchronized I/O file integrity completion:
      * For a read op: this means the requested data has been transferred
        from the disk to the process. If a pending write operation will affect
        the requested data, it is performed before the read operation according
        to the requirements of `synchronized I/O *file* integrity completion`.

      * For a write op: this means the data has been transferred. And **ALL** the
        metadata has also been transferred.

   > `synchronized I/O file integrity completion` is the superset of `synchronized
   > I/O data integrity`. And the **only difference** between them is whether **ALL**
   > metadata will be written to the disk in a write operation.


7. syscalls for controlling kernel `buffer cache`
   
   1. fsync(2)

      ```c
      #include <unistd.h>

      int fsync(int fd);
      ```

      Writes all data and metadata associated with `fd` to the disk, returns
      when the transferation is done.

      > Reaches `synchronized I/O file integrity completion`

   2. fdatasync(2)
      
      ```c
      #include <unistd.h>

      int fdatasync(int fd);
      ```

      Weirtes all data and requested metadata to the disk

      > Reaches `synchronized I/O data integrity completion`

      If you don't need all the metadata to be flushed, use `fdatasync(2)` over
      the `fsync(2)` to reduce the number of disk operation.

   3. sync(2)
      
      ```c
      #include <unistd.h>

      void sync(void);
      ```

      Writes **all kernel buffer cache containing updated file information** to the
      disk. Returns only after all data has been transferred to the disk (or its 
      cache) (This is the behavior on Linux). SUSv3 does not request this.

   4. syncfs(2) (Linux-specific)
      
      ```c
      #include <unistd.h>

      int syncfs(int fd);
      ```

      Like `sync(2)`, but just write kernel buffer cache related to file specified 
      by `fd` to the disk.

      > What is the difference between `fsync(2)` and `syncfs(2)`?
      > 
      > Fist, `fsync(2)` is a POSIX syscall, `syncfs(2)` is exclusive to Linux.
      >
      > [link](https://stackoverflow.com/q/48171855/14092446)
      >
      > Besides from its availablity, IDK if there are any other differences.
      >
      > The above question's ac sucks.

8. Performance impact of synchronous `write(2)`

   See `2` for basic information. 

   Flags about synchronous I/O:

   1. O_SYNC: make subsequent writes reach `synchronized I/O file integrity 
      completion`

   2. O_DSYNC: make subsequet writes reach `synchronized I/O data integrity
      completion`

   3. O_RSYNC: According to the standard, when used with `O_SYNC`, make subsequent
      reads reach `synchronized I/O file integrity completion`; When used with 
      `O_DSYNC`, make subsequent reads reach `synchronized I/O data integrity 
      completion.` Linux does not support this flag, in current implementaion,
      it is just an alias to `O_SYNC`.

   > O_FSYNC: IDK what does this flag mean, currently just an alias to O_SYNC 
   > under Linux (kernel 5.19.8).


9. Summary of buffering
  
   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-09-14_11-30-29.jpg)

   [article: ensuring data reaches disk](https://lwn.net/Articles/457667/)

10. Direct I/O: bypassing the kernel buffer cache

    This can be enabled through the `O_DIRECT` flag of `open(2)` when opening a
    file or through `fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_DIRECT)` for a
    already open file.

    And this is NOT supported by ALL Linux file systems (e.g., VFAT). To test 
    whether it is supported on your system, you can simply try it.

    ```c
    #define _GNU_SOURCE
    
    #include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <fcntl.h>
    
    int main(int ac, char *av[])
    {
            int res = open("new", O_CREAT | O_WRONLY | O_DIRECT, 0666);

            if (res == -1 && errno == EINVAL) {
                    // print file system name
                    printf("Current file system: ");
                    system("stat -f -c \"%T\" .");

                    printf("O_DIRECT is NOT supported\n");
            }
    }
    ```

    If a file is opened with direct I/O by a process, then opened normally by
    another process, then there is **no coherency** between the contents of the 
    buffer cache and the data read or written via direct I/O. **This should be
    avoided.**


    Alignment restrictions for direct I/O:

    > Because direct I/O involes direct accesses to disk, we must observe a 
    > number of restrictions when performing I/O

    1. the starting address of buffer being transferred must be **disk bocksize** aligned.
    2. the offset of file or device must be **disk blocksize** aligned.
    3. the length of buffer must be **disk blocksize** aligned.

    Violation of these rules will result in a failure with errno value of `EINVAL`

    > Under Linux 2.4, these alignment restrictions must be **file system** block
    > size aligned, which is more restrictive.
    >
    > file system block size is typically bigger than the disk block size.
    > 
    > [patch email link](https://lwn.net/Articles/12032/)

    > On Btrfs, Direct I/O will fall back to Buffered I/O if alignment restrictions
    > are not satisfied. See [post](https://www.spinics.net/lists/linux-btrfs/msg67293.html)

11. mixing lib functions and syscalls for File I/O

    syscalls and library functions can be used together, but you should keep 
    buffer issue in mind. NOTE: these is NO coherency issue, just that the 
    write order may be counterintuitive.

    ```c
    #include <stdio.h>
    #include <unistd.h>
    
    int main(void)
    {
            printf("hello world ");
            write(1, "HELLO ", 6);
    }
    ```

    For example, the above program will print `HELLO ` first, then print 
    `hello world ` when the process terminates (stdout is closed and the buffer
    will be flushed at that time).

    ```shell
    $ gccs main.c
    
    $ ./a.out
    HELLO hello world %
    ```

    If you wanna `hello world ` to be printed fist, you can manually flush the
    buffer, add a newline to do this since stdout is line-buffered or disable 
    the buffering through `setbuf(stream, NULL)`

12. convert between `FILE *` and `fd`
    
    ```c
    #include <stdio.h>

    int fileno(FILE *stream);
    FILE * fdopen(int fd, const char *mode); // mode should be consistent with the flag of `fd`
    ```

    `fdopen(3)` is super useful in the cases where you need to create a socket 
    and do buffered I/O on it since you just have `int socket()` rather than
    `FILE * socket()`.
