#### Ch18: Directories and Links

> 18.1: Directories and (Hard) Links  
> 18.2: Symbolic (Soft) Links  
> 18.3: Creating and Removing (Hard) Links: `link(2)`, `linkat(2)`, `unlink(2)`, `unlinkat(2)`  
> 18.4: Changing the Name of a File: `rename(2)` `renameat(2)` `renameat2(2)` (`mv` command)  
> 18.5: Working with Symbolic Links: `symlink(2)/symlinkat(2)` and `readlink(2)/readlinkat(2)`  
> 18.6: Creating and Removing directories: `mkdir(2)/mkdirat(2)` and `rmdir(2)`  
> 18.7: Removing a File or directory: `remove(3)` (a wrapper for `unlink(2)` and `rmdir(2)`)  
> 18.8: Reading Direcotries: `opendir(3)/fopendir(3)/readdir(3)/rewinddir(3)/closedir(3)`  
> 18.9: File Tree Walking: `nftw(3)/ftw(3)`  
> 18.10: The Current Working Directory of a Process  
> 18.11: Operating Relative to a Directory File Descriptor  
> 18.12: Changing the Root Directory of a Process: `chroot(2)`  
> 18.13: Resolving a Pathname: `realpath(1)/realpath(3)`  
> 18.14: Parsing Pathnames Strings: `dirname(3)` and `basename(3)`  

> 1. How to differentiate between a regular file and directory in the disk level
> 2. What is hard link
> 3. How can I find the filename associated with a file descriptor?
> 4. The limitations of hard link
> 5. What is soft link (a file type whose content is a path it points to)
> 6. What is soft link (a file whose constent is a path), we can read it through
>    `readlink(1)/readlink(2)`
> 7. What is dangling symlink
> 8. How many layers of symlink Linux can resolve (TL;DR: After kernel 4.2, 40)
> 9. Permission (0o777) and ownership (only useful in a rare scene) of a symlink

##### 18.1: Directories and (Hard) Links

1. How to differentiate between a regular file and directory in the disk level
  
   > A direcotry is stored in a similar way to a regular file, except:

   1. The first 4 bits of `st_mode` identify this is a directory
   2. The contents of a regular file is unorganized, while a direcotry basically
      can be seen as a table, consisting of filenames and i-node numbers.

      > An entry in this table is a hard link to this file.

      | filename                  | I-node number|
      |---------------------------|--------------|
      | file1                     | 1            |
      | file2                     | 2            |
      | file3(hard link to file1) | 1            |
      | dir1                      | 3            |

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-20_09-57-37.jpg)

2. I-node entry **does not** store `filename`, **it is stored in the direcotry**.
   Because of this, we can create multiple `filename`s having the same
   i-node number, **which is `hard link`**.

   > Most Linux and UNIX file systems support hard link, Microsoft VFAT does not.

   ```shell
   $ touch file
   $ ln file link
   $ l -i
     inode Permissions Links Size User  Group Date Modified Name
   4474947 .rw-r--r--@     2    0 steve steve 20 Oct 11:19  file
   4474947 .rw-r--r--@     2    0 steve steve 20 Oct 11:19  link
   ```


3. When the number of hard links of a file decreases to 0, and there is no process
   holding open file descriptors pointing to it, the disk allocated for that
   file can be deallocated. Kinda similar to `reference count`.

4. How can I find the filename associated with a file descriptor?

   [question: Retrieve filename from file descriptor in C](https://stackoverflow.com/q/1188757/14092446)

   Generally speaking, we can NOT achive this. A file desriptor refers to a 
   i-node entry, while multiple files can share the same i-node.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/relation_between_fd_and_open_files.jpeg)

   But on Linux, with the `/proc` virtual file system, we can scan the entries of 
   `/proc/PID/fd` (using `readlink(1)`):

   ```shell
   # Note that you can not use `l /proc/self/fd` since this will print the open
   # file descriptors of `l`.
   $ cd /proc/self/fd
   $ l
   Permissions Links Size User  Group Date Modified Name
   lrwx------@     1   64 steve steve 20 Oct 07:31  0 -> /dev/pts/7
   lrwx------@     1   64 steve steve 20 Oct 11:36  1 -> /dev/pts/7
   lrwx------@     1   64 steve steve 20 Oct 11:36  2 -> /dev/pts/7
   ```

5. The limitations of `hard link`s

   1. Hard link can not across multiple file systems, since I-node number is 
      guaranteed to be unique only within a single file system.
   
   2. Hard link can not be applied to `direcotries`, this is used to prevent
      from `circular` links.

      > Early UNIX implementations do allow hark link on directories if it is
      > created by privileged users, this is because there is no `mkdir(2)`
      > at that time.
      >
      > Future Steve: On a normal UNIX file system, the hard link for a directory
      > is at least 2, one in its parent directory, one in itself (the dot `.`),
      > so directory can have hard links, users cannot create it.
      >
      > And for BTRFS, 
      > [the hard link for a dir will always be 1](https://linux-btrfs.vger.kernel.narkive.com/oAoDX89D/btrfs-st-nlink-for-directories)


      See
      [Ch14: 15 4 for more information about `bind mount`](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch14:file_systems.md)

   > All these limitations can be resolved by soft link.

##### 18.2: Symbolic (Soft) Links

6. What is soft link?

   A soft link is a type of file whose content is the name of another file.

   We can use `readlink(2)` to read it (or `readlink(1)` from shell):

   > You can not use `open(2)` with argument `O_NOFOLLOW`, this will return `ELOOP`.

   ```c
   #include <stdlib.h>
   #include <unistd.h>
   #include <stdio.h>
   
   int main(void) {
           char buf[BUFSIZ];
           int res = readlink("link", buf, BUFSIZ);
           if (res == -1) {
                   perror("readlink:");
                   exit(EXIT_FAILURE);
           }
           // explicitly set NUL
           buf[res] = '\0';
   
           printf("The contents of this soft link: %s\n", buf);
           return 0;
   }
   ```
   ```shell
   $ touch file
   $ ln -s file link
   $ gccs main.c && ./a.out
   $ The contents of this soft link: file
   ```

   Rust has a similar function [`std::fs::read_link`](https://doc.rust-lang.org/std/fs/fn.read_link.html),
   which is basically a wrapper for `readlink(2)` on UNIX platforms:

   ```rust
   use std::fs::read_link;
    
   fn main() {
       let contents = read_link("link").unwrap();
       println!("{}", contents.display());
   }
   ```
   ```shell
   $ cargo r -q
   file
   ```

7. The contents (path) of a soft link can be either `relative` or `absolute`,
   if it is a relative path, then it is relative to `link` itself.

   See [exercise 18-2 for a great example](https://github.com/SteveLauC/The-Linux-Programming-Interface/tree/main/exercise/Ch18/18-2)

8. dangling link
  
   When a soft link refers to a path that does not exist, we say this link is
   a dangling link, just like the dangling pointer.

   It is even possible to create a dangling link:

   ```shell
   $ l

   # Note that `file` does not exist
   $ ln -s file link

   # `link` is a dangling link that points to a file that does not exist.
   $ l
   Permissions Links Size User  Group Date Modified Name
   lrwxrwxrwx      1    4 steve steve 20 Oct 14:01  link -> file
   ```
9. Soft link can be used across file systems (partitions), since it uses `path`
   rather than I-node number.

10. Soft links can be chained (i.e., `a` is a soft link pointing to `b`, `b` is a 
    softlink pointing to `c`), and the kernel can automatically dereference this 
    for us. 

    There is a limit on how many layers of symlink can Linux resolve? POSIX defines
    this limit to be `8`, [Linux (after kernel 4.2) chose `40`](https://unix.stackexchange.com/q/721724/498440).
    If you get a chain longer than this, you will get:

    ```
    ELOOP  Too many symbolic links were encountered in resolving pathname.
    ```

    > Create a soft link pointing to itself, then you will see that `ELOOP`:
    >
    > ```shell
    > $ ln -s link link
    > $ cat link
    > cat: link: Too many levels of symbolic links
    > ```

11. Permission and ownership of a symlink

    Symlink always has permission `0o777`. This means that you can always see the 
    contents of this link (i.e., dereference it). So if you wanna operate a symlink,
    whether this is allowed depends on the permission and ownership of the file 
    referred to by that link.
 
    The ownership of symlink is uesful only when the link itself is being removed
    or renamed in a direcotry with sticky bit (restricted deletion or renaming bit)
    set.
 
    ```shell
    $ echo "hello" > a
    
    $ ln -s a b
    
    $ l
    Permissions Links Size User  Group Date Modified Name
    .rw-r--r--@     1    6 steve steve 21 Oct 07:46  a
    lrwxrwxrwx@     1    1 steve steve 21 Oct 07:46  b -> a
    
    $ chmod 000 a
    
    $ cat b
    cat: b: Permission denied
    ```

12. Whether a syscall follows symlink
 
    ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-21_08-06-41.jpg)

##### 18.3: Creating and Removing (Hard) Links: `link(2)`, `linkat(2)`, `unlink(2)`, `unlinkat(2)`

13. Create hard links

    ```c
    #include <unistd.h>

    int link(const char *oldpath, const char *newpath);

    #include <fcntl.h>           /* Definition of AT_* constants */
    #include <unistd.h>

    int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
    ```

    If `newpath` already exists, it is not overwritten. Instead, an error `EEXIST`
    returns.

    `link(2)` does not follow symlink, this is different from the standard (Though
    the standard has been changed, making this implementation dependent). If the 
    `AT_SYMLINK_FOLLOW` is given to `linkat(2)` and `oldpath` is a symlink, then 
    it is dereferenced.

14. Remove hard links (or simply remove files)

    > This syscall simply decrease the number of hard links by 1. After this
    > operation, if it reaches 0 (**and no open file descriptor refers to it**),
    > then this file is removed from the underlying file system.

    ```c
    #include <unistd.h>

    int unlink(const char *pathname);

    #include <fcntl.h>           /* Definition of AT_* constants */
    #include <unistd.h>

    int unlinkat(int dirfd, const char *pathname, int flags);
    ```

    `unlinkat(2)` is similar to `unlink(2)` except that:

    1. An extra `dirfd` argument.
    2. `unlink(2)` **can not remove dir**, while `unlinkat(2)` **can**. If `AT_REMOVEDIR`
       is present in `flags`, then `unlinkat(2)` remove direcotries, which is 
       equivalent to `rmdir(2)`. NOTE that then `unlinkat(fd, path, AT_REMOVEDIR)`
       can NOT be used to remove files, or you will get `ENOTDIR`. And `target dir`
       must be empty, just like the `rmdir(2)`.

    An open file is deleted only when all file descriptors referring to it are 
    closed:
    
    ![diagram](https://github.com/SteveLauC/pic/blob/main/relation_between_fd_and_open_files.jpeg)

    When the number of hardlink reaches 0, if there is still open file descriptor
    referring to it, then it will NOT be deleted. This is a useful feature, it
    makes a process who wanna delete a file does not need to worry about whether
    some other processes have it open.

15. How to create a temporary file?
  
    Since a file won't be deleted if there is still open file descriptor referring
    to it even though the number of hard links decreases to 0, we can create a 
    temporary file and `unlink(2)` it immediately, the file is still valid. When
    our `fd` is closed, this file will be deleted.

    This is exactly `tmpfile(3)` does.


##### 18.4: Changing the Name of a File: `rename(2)` `renameat(2)` `renameat2(2)` (`mv` command)

16. rename a file (Manipulate the contents of directories, rather than file data)

    ```c
    #include <stdio.h>
 
    int rename(const char *oldpath, const char *newpath);
 
    #include <fcntl.h>           /* Definition of AT_* constants */
    #include <stdio.h>
 
    int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);
    int renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, unsigned int flags);
    ```
 
    `renameat(2)` is similar to `rename(2)` except two extra `fd` arguments.
 
    `renameat2(2)` is similar to `renameat(2)` except an extra `flags` argument
 
    Some rules (full list see `man 2 rename`):
 
    0. [rename(2) is atomic](https://stackoverflow.com/q/7054844/14092446)
 
    1. If a process holds an open file descriptor to file `file`, then we rename it
       to `file2`, this won't effect that file descriptor because `fd` refers to the
       open file description, which has no connection with filename.
 
       ![diagram](https://github.com/SteveLauC/pic/raw/main/relation_between_fd_and_open_files.jpeg)
 
    2. All those three syscalls will not dereference symlinks in either of its 
       arguments.
 
    3. If `oldpath` is a file, then `newpath` can not be a directory (or you will
       get `EISDIR`).
 
    4. If `oldpath` is a directory, then `newpath` either must not exist or must
       be the name of an empty directory (or you will get `ENOTDIR` and `ENOTEMPTY`
       respectively).
 
    5. If `oldpath` is a directory, then `newpath` can not contain a direcotry prefix
       that is same as `oldpath` (`EINVAL`)

       ```c
       rename("/home/steve", "/home/steve/Desktop");
       ```
 
    6. `oldpath` and `newpath` must reside on the **same** file system.
 
       If you use `mv(1)` to move a file across different file systems, then what
       `mv(1)` does is to explicitly `copy`(open/read/write) and `delete`(unlink)
       that file, which obviously is not atomic.

##### 18.5: Working with Symbolic Links: `symlink(2)/symlinkat(2)` and `readlink(2)/readlinkat(2)`

17. Create a symlink
   
    ```c
    #include <unistd.h>
 
    int symlink(const char *target, const char *linkpath);
 
    #include <fcntl.h>           /* Definition of AT_* constants */
    #include <unistd.h>
 
    int symlinkat(const char *target, int newdirfd, const char *linkpath);
    ```

    If linkpath exists, it will **NOT** be overwritten (`EEXIST`). 

    `target` is not requested to exist when calling `syslink(target, linkpath)`,
    we use this to create dangling symlink.

18. Read a symlink

    ```c
    #include <unistd.h>

    ssize_t readlink(const char *restrict pathname, char *restrict buf,
                    size_t bufsiz);

    #include <fcntl.h>            /* Definition of AT_* constants */
    #include <unistd.h>

    ssize_t readlinkat(int dirfd, const char *restrict pathname,
                     char *restrict buf, size_t bufsiz);
    ```

    How to pick an appropriate buffer size, use `lsatt(2)` to retrieve file length
    first. Or we can use `PATH_MAX`.
    
    > There is another limit constant `SYMLINK_MAX`, which defines the maximum
    > bytes that can be placed in a symlink
    >
    > ```c
    > #include <unistd.h>
    > #include <stdio.h>
    >
    > int main(void) {
    >     long res = sysconf(_PC_SYMLINK_MAX);
    >     if (res == -1) {
    >         perror(NULL);
    >     }
    >
    >     printf("_PC_SYMLINK_MAX: %ld", res);
    >     return 0;
    > }
    > ```
    > 
    > ```shell
    > $ gccs main.c && ./a.out
    > _PC_SYMLINK_MAX: 200809⏎
    > ```

    If the length of the link exceeds `bufsiz`, then a **truncated** string is 
    placed in `buf`. Because `readlink(2)/readlinkat(2)` will not place a terminating
    `NUL` in the buffer, so there is not way to tell whether the contents returned 
    is truncated or not. If you wanna check this, allocating a bigger buffer and
    call it again, then compare the results to see if they are identical.

19. To remove a symlink, use `unlink(2)` and its variants

##### 18.6: Creating and Removing directories: `mkdir(2)/mkdirat(2)` and `rmdir(2)`

20. Create a directory 
   
    ```c
    #include <sys/stat.h>
 
    int mkdir(const char *pathname, mode_t mode);
 
    #include <fcntl.h>           /* Definition of AT_* constants */
    #include <sys/stat.h>
 
    int mkdirat(int dirfd, const char *pathname, mode_t mode);
    ```
 
    The owner of this new direcotry is the EUID of the calling process, group 
    depends on the semantics (BSD or System v).
 
    The `mode` argument, if contains `set-UID bit`, is ignored, since `set-UID`
    is useless for direcotries. `set-GID bit`, is also ignored. But if the parent
    directory has `set-GID` set, then it automatically inherit this.
 
    > How `mkdir(2)` treats `set-UID/set-GID/sticky bit`, is implementation defined.
 
    > For the usage of `set-UID/set-GID/sticky bit`, see 
    > [Ch15](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch15:file_attributes.md)

21. Create a temporary direcotry with a template

    ```c
    #include <stdlib.h>

    char *mkdtemp(char *template);


    int mkstemp(char *template);
    int mkostemp(char *template, int flags);
    int mkstemps(char *template, int suffixlen);
    int mkostemps(char *template, int suffixlen, int flags);
    ```

22. Remove an empty directory

    ```c
    #include <unistd.h>

    int rmdir(const char *pathname);
    ```

    `pathname` has to be an empty directory, this is consistent with `rmdir(1)`.

    If `pathname` is a symlink, it is not dereferenced and `ENOTDIR` (not a 
    directory) is returned.

    `unlinkat(fd, path, AT_REMOVEDIR)` is equivalent to `rmdir(2)`.


##### 18.7: Removing a File or directory: `remove(3)` (a wrapper for `unlink(2)` and `rmdir(2)`)

23. Remove a file or an empty direcotry using `remove(3)`

    ```c
    #include <stdio.h>

    int remove(const char *pathname);
    ```

    This function calls `unlink(2)` for files and `rmdir(2)` for directories.

    ```rust
    use std::{
        fs::{remove_dir, remove_file},
        io::Result,
        path::Path,
    };
    
    fn remove<P: AsRef<Path>>(path: P) -> Result<()> {
        let metadata = path.as_ref().symlink_metadata()?;
    
        if metadata.is_dir() {
            remove_dir(path.as_ref())
        } else {
            remove_file(path.as_ref())
        }
    }
    ```

    
##### 18.8: Reading Direcotries: `opendir(3)/fopendir(3)/readdir(3)/rewinddir(3)/closedir(3)`

24. Iterate over the entries of a directory

    ```c
    #include <sys/types.h>
    #include <dirent.h>
    
    DIR *opendir(const char *name);
    DIR *fdopendir(int fd);

    struct dirent *readdir(DIR *dirp);

    struct dirent {
        ino_t          d_ino;       /* Inode number */
        off_t          d_off;       /* Not an offset; see below */
        unsigned short d_reclen;    /* Length of this record */
        unsigned char  d_type;      /* Type of file; not supported
                                       by all filesystem types */
        char           d_name[256]; /* Null-terminated filename */
    };


    // rewind to the first entry
    void rewinddir(DIR *dirp);

    int closedir(DIR *dirp);
    ```

    > All the return values are statically-allocated.

    See `d_name` field of struct `dirent`, directory stores `file name`, i-node
    does not.

    The order of entries returned by `readdir(3)` is just the order how entries
    are stored in the disk. (`ls -f` list all entries in directory order) 

25. How can I thread-safely use `readdir(3)`?

    `readdir(3)` returns a pointer pointing to a buffer inside its `DIR` argument
    , and can be overwritten by subsequent calls. 

    So having multiple threads read the same `DIR` would be unsafe, this means 
    that you can not share a `DIR` object among multiple threads. 

    Multiple `DIR` objects pointing to the same directory in multiple threads, 
    this is safe. 

    If you have to read the same `DIR` from multiple threads, add some external
    synchronization.

    > In Rust, thanks to the exclusive reference `fn next(&mut self)`, it is guaranteed
    > that simultaneous access to `DIR` won't happen, so no extra external 
    > synchronization is needed.


26. list directory entries from shell

    Other than `ls(1)`, you can also use `dir(1)`, which is equivalent to
    `ls -C -b`. Anyway, I use `exa`.

27. Scan and filter directory entries using `scandir(3)`

    ```c
    #include <dirent.h>

    int scandir(const char *restrict dirp,
                struct dirent ***restrict namelist,
                int (*filter)(const struct dirent *),
                int (*compar)(const struct dirent **,
                              const struct dirent **));
    ```


28. Random access within a direcotry

    ```c
    #include <dirent.h>

    // return current location in directory stream
    long telldir(DIR *dirp);


    void seekdir(DIR *dirp, long loc);
    ```

29. Extract the file descriptor from a `DIR`

    ```c
    #include <sys/types.h>
    #include <dirent.h>

    int dirfd(DIR *dirp);
    ```

##### 18.9: File Tree Walking: `nftw(3)/ftw(3)`

30. File tree walking

    ```c
    #include <ftw.h>
 
    int nftw(const char *dirpath,
            int (*fn)(const char *fpath, const struct stat *sb,
                      int typeflag, struct FTW *ftwbuf),
            int nopenfd, int flags);


    int ftw(const char *dirpath,
            int (*fn)(const char *fpath, const struct stat *sb,
                      int typeflag),
            int nopenfd);
    ```

    `nftw(3)` preorder traverse the sub-tree located in `dirpath`, and call `fn()`
    on each of them. To avoid reaching the open file desciptor limit, one can
    set a threashold in `nopenfd` to specify the maximum number of open fd that 
    this process can hold (since we are operating directories, so this is the 
    maximum number of directories that are open simultaneously).

    `nftw(3)` is the enhanced version (**n**ew ftw) of `ftw(3)`, and `ftw(3)` is
    marked obsolete in SUSv4.

    There is Rust [crate: walkdir](https://github.com/BurntSushi/walkdir) providing similar functionality.


##### 18.10: The Current Working Directory of a Process

31. A new process inherit this from its parent.

32. Get current working directory in C

    ```c
    #include <unistd.h>

    // All these three functions return an absolute path that is the 
    // current working directory

    char *getcwd(char *buf, size_t size);

    // derived from BSD, deprecated
    char *getwd(char *buf); 

    char *get_current_dir_name(void);
    ```

    As  an extension to the POSIX.1-2001 standard, glibc's `getcwd()` allocates 
    the buffer dynamically using `malloc(3)` **if buf is NULL**.  In this case, the
    allocated buffer has the length size unless size is zero, when buf is 
    allocated as big as necessary.  **The caller should free(3) the returned buffer**.

    `get_current_dir_name()` will `malloc(3)` an array big enough to hold the 
    absolute pathname of the current working directory.  If the environment 
    variable PWD is set, and its value is correct, then that value will be returned.
    **The caller should free(3) the returned buffer**

    In Rust, we have `std::env::current_dir()`:

    ```rust
    pub fn current_dir() -> Result<PathBuf>
    ```

    > QUES: very curious how this attribute is stored in the kernel?
    >       Waht is that `dentry` struct?
    >      
    >       Looking at its source code, `dentry` stores the `inode` assocaiated
    >       with it.

33. Get current working directory from the Linux `/proc` virtual file system

    `/proc/PID/cwd` is a symlink pointing to the actual `cwd`.

    ```shell
    $ readlink /proc/self/cwd
    /home/steve/Documents
    ```

34. Change current working directory using `chdir(2)` or `fchdir(2)`
  
    ```c
    #include <unistd.h>

    int chdir(const char *path);
    int fchdir(int fd);
    ``` 

    If `path` is a symlink, it is dereferenced.

    > QUES: which is faster? I think it depends on how this attribute is stored
    >       in the kernel.
    >
    >       If `dentry` uses `inode` to identify the pwd, then `fchdir()` should
    >       be faster? Since `chdir()` has to `open()` the file.
    >
    >       To future steve, when you figured this out, do also update the tlpi
    >       exercise 18.9. 

##### 18.11: Operating Relative to a Directory File Descriptor

![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-22_17-39-57.jpg)

35. Those `xxxat(2)` syscalls normally provide some extra functionalities:

    1. If `path` argument is a relative path, then it is interpreted relative to the
       directory specified in `dirfd` rather than the currrent working directory of
       the process.
    
    2. If `path` is a relative path, and `dirfd` is the special value `AT_FDCWD`,
       then `path` is interpreted relative to the current working directory.
    
    3. If `path` is an absolute path, then `dirfd` is ignored.

36. Why did they exist?

    > Copied from the book, currently I don't quite understand this.
  
    1. Using openat() allows an application to avoid certain race conditions 
       that can occur when open() is used to open files in locations other 
       than the current working directory. These races can occur because some 
       component of the directory prefix of pathname could be changed in parallel
       with the open() call. By opening a file descriptor for the target directory,
       and passing that descrip- tor to openat(), such races can be avoided.
  
    2. In Chapter 29, we’ll see that the working directory is a process attribute
       that is shared by all threads of the process. For some applications, it is 
       useful for dif- ferent threads to have different “virtual” working directories.
       An application can emulate this functionality using openat() in conjunction 
       with directory file descriptors maintained by the application.


##### 18.12: Changing the Root Directory of a Process: `chroot(2)`

37. Current working directory is the start point from which relative path is
    interpreted. Root directory is the point from which absolute path is 
    interpreted.

    By default, the root dir of every process is the real root directory on
    the file system

    > A new process inherit this from its parent directory.

38. A privileged process (`CAP_SYS_CHROOT`) can do this using the `chroot(2)`
    syscall.

    ```c
    #include <unistd.h>

    int chroot(const char *path);
    ```

    If `path` is a symlink, then it is dereferenced.

    This operation is referred to as `chroot jail`.

    In Rust, we have `std::os::unix::fs::chroot()`, though you can still use 
    the Rusty wrapper from `nix`.

39. Run a shell command with customized root using `chroot(1)`

    ```shell
    $ chroot --help
    Usage: chroot [OPTION] NEWROOT [COMMAND [ARG]...]
      or:  chroot OPTION
    Run COMMAND with root directory set to NEWROOT.
    
          --groups=G_LIST        specify supplementary groups as g1,g2,..,gN
          --userspec=USER:GROUP  specify user and group (ID or name) to use
          --skip-chdir           do not change working directory to '/'
          --help     display this help and exit
          --version  output version information and exit
    
    If no command is given, run '"$SHELL" -i' (default: '/bin/sh -i').
    
    GNU coreutils online help: <https://www.gnu.org/software/coreutils/>
    Full documentation <https://www.gnu.org/software/coreutils/chroot>
    or available locally via: info '(coreutils) chroot invocation'
    ```

40. Read the root directory of any process from the `/proc` vfs

    `/proc/PID/root` is symlink pointing to the real root directory of this
    process.

    ```
    $ readlink /proc/self/root

    /
    ```

41. chroot jail is useless for privileged processes, since they can easily 
    break it.

    For unprivileged processes, we should take care of the following things:

    1. `chroot(2)` does not change current working directory, so one need to call
       `chdir("/")` to ensure this. If this is not done, then it can use relative
       path to access the files outside of the jail. (Some BSDs prevent this
       possibility, if the OS find that `cwd` lies outside the root, then `chroot(2)`
       will set cwd to "/" automatically.)

    2. If it has open file descriptors that lie outside the root, then `fchdir(2)`
       can break the jail. Should note that the file descriptors of a new process
       are inherited from its parent, should close them beforehand.

    3. A process can receive file descriptors from other process through `socket()`,
       then call `fchdir(2)` to break the jail.


##### 18.13: Resolving a Pathname: `realpath(1)/realpath(3)`

42. What this does
   
    1. Expand all the symlinks
    2. Interpret all "." and ".." items.
    3. Remove extra "/"

    ```c
    #include <limits.h>
    #include <stdlib.h>

    char *realpath(const char *restrict path,
                   char *restrict resolved_path);
    ```

    Would fail if `path` does not exist.

    If `resolved_path` is specified as `NULL`, then `realpath(3)` would allocate
    a sufficient buffer to hold the result, user should call `free(3)` to deallocate
    it.

43. `std::fs::canonicalize()` in Rust
   
    > This function currently corresponds to the `realpath` function on Unix

    Besides this, Rust has a [crate: path-clean](https://github.com/danreeves/path-clean) 
    that can be used even when `path` does not exist.

    > path-clean does not dereference symlink


##### 18.14: Parsing Pathnames Strings: `dirname(3)` and `basename(3)`

44. `dirname(3)` and `basename(3)`

    For a pathname, the series of component filenames preceeding the final slash
    is sometimes referred to as the `directory` part of a pathname, while the
    name following the final slash is sometimes referred to as the file or the 
    base part of the pathname.

    ```c
    #include <libgen.h>
    
    char *dirname(char *path);
    char *basename(char *path);
    ```
    
    Both `dirname(3)` and `basename(3)` may modify the `path` argument, if we wanna
    preserve them, we must pass copies of it to `dirname(3)` and `basename(3)`.
    And the return value is statically allocated, can be overwritten by subsequent
    calls.
    
    |path       |    dirname   | basename|
    |-----------|--------------|---------|
    | /usr/lib  | /usr         | lib     |
    | /usr/     | /            | usr     |
    | usr       | .            | usr     |
    | /         | /            | /       |
    | .         | .            |  .      |
    | ..        | .            | ..      |
    | "" (enpty)| .            | .       |
    | NULL      | .            | .       |
    
    Concatenating the string returned by `dirname(3)`, a slash (`/`), and the string
    returned by `basename(3)` yields a complete pathname. Note that `dirname("/")` + 
    "/" + `basename("/")` yields `///`, this is a valid pathname since multiple slashes
    will be seen as a single one.
    
    See also `dirname(1)` and `basename(1)`

    > In Rust, we have `Path::parent()` and `Path::file_name()`.

45. duplicate a string in C
  
    ```c
    #include <string.h>

    // call `malloc(3)` to duplicate string
    char *strdup(const char *s);

    // Similar to `strdup(3)`, but at least dup `n` bytes
    char *strndup(const char *s, size_t n);

    // There two methods use `alloca(3)` to allocate stack memory
    char *strdupa(const char *s);
    char *strndupa(const char *s, size_t n);
    ``` 
