#### Ch15: File Attributes

> 1. retrieve file information using `stat(2)/lstat(2)/fstat(2)` from `i-node`
> 2. `stat` struct
> 3. get birthtime of a file using `statx(2)`
> 4. change `atime` and `mtime` using `utime(2)/utimes(2)/futimes(2)/lutimes(2)/utimensat(3)/futimens(3)`

1. `stat/fstat/lstat`
   
   ```c
   #include <sys/stat.h>

   int stat(const char *restrict pathname, struct stat *restrict statbuf);
   // Rust: std::fs::metadata()

   int fstat(int fd, struct stat *statbuf);
   int lstat(const char *restrict pathname, struct stat *restrict statbuf);
   // Rust: std::fs::symlink_metadata()
   ```

   `fstat(2)` is similar to `stat(2)` except the target file is specified using a 
   `file descriptor` rather than a `path`.

   When `pathname` is a softlink, `lstat(2)` will not deref it and return the
   information about the link itself.

   Requested permission: When using `stat(2)/lstat(2)`, effective user should 
   have executable permission of the parent directory of `pathname`, or one will
   get `EACCES`.

   `fstat(2)` will always succeed.

2. lstat(2) and soft link

   SUSv3 only requests that `st_size` and `st_mode` are valid when using `lstat(2)`
   on a soft link.

   For a specific OS, whether other fields can be maintained is implementation 
   dependent. Soft link can be implemented using either `i-node` or `directory`,
   for the latter, it is impossible to impl other fields.

   Linux's `lstat(2)` returns **all** the fields.

   > SUSv4 requests `lstat` returns all the fields except `st_mode`.

3. struct stat
   
   > In Rust, you should use [`std::os::linux::fs::MetadataExt`](https://doc.rust-lang.org/std/os/linux/fs/trait.MetadataExt.html)
   > This is slightly different from `std::os::unix::fs::MetadataExt` in the doc
   > description, though they are the same thing under the hood.

   ```c
   // stat struct

   struct stat {
       dev_t     st_dev;         /* ID of device containing file, this encodes both major ID and minor ID */
       ino_t     st_ino;         /* Inode number */
       mode_t    st_mode;        /* File type and permission */
       nlink_t   st_nlink;       /* Number of **hard** links */
       uid_t     st_uid;         /* User ID of owner */
       gid_t     st_gid;         /* Group ID of owner */
       dev_t     st_rdev;        /* Device ID (if target is a device special file) */

       off_t     st_size;        /* Total size, in bytes */
       // if target is a regular file, then this field is the size of that file
       // if target is a soft link, then this is the length of the pathname pointed by that link
       // For a shared memory object, this is the size of that object.

       blksize_t st_blksize;     /* Block size for filesystem `I/O` */

       // Number of 512B blocks allocated for the whole file entry (not just data blks)
       // see [Ch14:](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch14:file_systems.md)
       // note 6 and 7.
       blkcnt_t  st_blocks;    
       // the unit of 512B is a historical legacy. SUSv3 does not specify this unit.
       // Most UNIX implementations use 512B, HP-UX11 uses file system-specific units.

       /* Since Linux 2.6, the kernel supports nanosecond
          precision for the following timestamp fields.
          For the details before Linux 2.6, see NOTES. */

       struct timespec st_atim;  /* Time of last access (read) */
       struct timespec st_mtim;  /* Time of last modification (data/file contents) */
       struct timespec st_ctim;  /* Time of last status change (metadata) */

   #define st_atime st_atim.tv_sec      /* Backward compatibility */
   #define st_mtime st_mtim.tv_sec
   #define st_ctime st_ctim.tv_sec
   };
   ```

   1. dev_t type

      `dev_t` in current glibc is 64-bit, with 32-bit major and minor numbers. glibc's 
      default encoding is MMMM Mmmm mmmM MMmm, where M is a hex digit of the major 
      number and m is a hex digit of the minor number. This is backward compatible 
      with legacy systems where dev_t is 16 bits wide, encoded as MMmm. It is also 
      backward compatible with the Linux kernel, which for some architectures uses 
      32-bit dev_t, encoded as mmmM MMmm.
      
      One can use major(3) and minor(3) to decompose a dev_t:
      
      ```shell
      $ cat main.c
      #include <assert.h>
      #include <sys/stat.h>
      #include <sys/sysmacros.h>
      #include <stdio.h>
      
      int main(void)
      {
              struct stat buf;
              assert(0 == stat(".", &buf));
      
              printf("Major device ID: %d\n", major(buf.st_dev));
              printf("Minor device ID: %d\n", minor(buf.st_dev));
      }
      
      $ gccs main.c && ./a.out
      Major device ID: 0
      Minor device ID: 39
      ```

   2. st_ino

      The i-node number of this file, this and `st_dev` uniquely identifies this
      file across all the file systems

   3. `mode_t` type
      
      > refer to [uulp Ch3.md: 14](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/understanding-unix-linux-programming/Ch3.md)

      In current implementation, `mode_t` is defined as `u32`. [Though it is a
      32-bit type, only the low 16 bits are used](https://stackoverflow.com/q/9602685/14092446)

      ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-02_20-54-58.jpg)

      1. File type (4 bits)
         
         4 bits can encode at most 16 file types, currently Linux only support 7 types.

         ```c
         #define        __S_IFDIR        0040000        /* Directory.  */           (00)0 100 
         #define        __S_IFCHR        0020000        /* Character device.  */    (00)0 010
         #define        __S_IFBLK        0060000        /* Block device.  */        (00)0 110
         #define        __S_IFREG        0100000        /* Regular file.  */        (00)1 000
         #define        __S_IFIFO        0010000        /* FIFO.  */                (00)0 001
         #define        __S_IFLNK        0120000        /* Symbolic link.  */       (00)1 010
         #define        __S_IFSOCK        0140000        /* Socket.  */              (00)1 100
         ```

         Since a file can only be of one file sype, we have a mask for these 4 bits:

         ```c
         #define        __S_IFMT        0170000        /* These bits determine file type.  */
         ```

         Determine a file type:

         ```c
         #include <assert.h>
         #include <sys/stat.h>
         #include <stdio.h>
 
         int main(void)
         {
                 struct stat buf;
                 assert(0 == stat("main.c", &buf));
 
                 mode_t mode = buf.st_mode;
                 switch (mode & S_IFMT) {
                 case S_IFDIR:
                         printf("dir\n");
                         break;
                 case S_IFCHR:
                         printf("char device\n");
                         break;
                 case S_IFBLK:
                         printf("block device\n");
                         break;
                 case S_IFREG:
                         printf("regular file\n");
                         break;
                 case S_IFIFO:
                         printf("fifo\n");
                         break;
                 case S_IFLNK:
                         printf("soft link\n");
                         break;
                 case S_IFSOCK:
                         printf("socket\n");
                         break;
                 default:
                         printf("unknown file type\n");
                         break;
                 }
         }
         ```

         Or one can directly use those convenient macros:
        
         ```c
         #include <assert.h>
         #include <sys/stat.h>
         #include <stdio.h>
         
         int main(void)
         {
                 struct stat buf;
                 assert(0 == stat("main.c", &buf));
                 mode_t mode = buf.st_mode;
         
                 if (S_ISDIR(mode)) {
                         printf("dir\n");
                 } else if (S_ISCHR(mode)) {
                         printf("char device\n");
                 } else if (S_ISBLK(mode)) {
                         printf("block device\n");
                 } else if (S_ISREG(mode)) {
                         printf("regular file\n");
                 } else if (S_ISFIFO(mode)) {
                         printf("fifo\n");
                 } else if (S_ISLNK(mode)) {
                         printf("soft link\n");
                 } else if (S_ISSOCK(mode)) {
                         printf("socket\n");
                 } else {
                         printf("unknown file type\n");
                 }
         }
         ```

         > File type `S_IFLNK` is only returned by `lstat(2)` since `stat(2)/fstat(2)`
         > always follow soft link.

      2. 3 special bits  
         
         * set-UID bit
         * set-GID bit
         * sticky bit

         ```c
         #include <sys/stat.h>
         #include <assert.h>
         #include <stdio.h>
         
         int main(void)
         {
                 struct stat buf;
                 assert(stat("'", &buf) == 0);
         
                 mode_t mode = buf.st_mode;
                 if (mode & S_ISUID) {
                         printf("set-UID bit is set\n");
                 }
         
                 if (mode & S_ISGID) {
                         printf("set-GID bit is set\n");
                 }
         
                 if (mode & S_ISVTX) {
                         printf("sticky bit is set\n");
                 }
         }
         ```

         ```shell
         $ ls -l \'
         -rwSr-Sr-T. 1 steve steve 133 Oct  3 07:53 "'"
 
         $ gccs main.c && ./a.out
         set-UID bit is set
         set-GID bit is set
         sticky bit is set
         ```

      3. permissions for user, group and others

         ```c
         #include <sys/stat.h>
         #include <assert.h>
         #include <stdio.h>
 
         int main(void)
         {
                 struct stat buf;
                 assert(stat("'", &buf) == 0);
 
                 mode_t mode = buf.st_mode;
 
                 {
                         // owner
                         printf("Owner: ");
                         if (mode & S_IRUSR) {
                                 printf("read ");
                         }
                         if (mode & S_IWUSR) {
                                 printf("write ");
                         }
                         if (mode & S_IXUSR) {
                                 printf("execute");
                         }
                         printf("\n");
                 }
                 {
                         // group
                         printf("Group: ");
                         if (mode & S_IRGRP) {
                                 printf("read ");
                         }
                         if (mode & S_IWGRP) {
                                 printf("write ");
                         }
                         if (mode & S_IXGRP) {
                                 printf("execute");
                         }
                         printf("\n");
                 }
                 {
                         // other
                         printf("Other: ");
                         if (mode & S_IROTH) {
                                 printf("read ");
                         }
                         if (mode & S_IWOTH) {
                                 printf("write ");
                         }
                         if (mode & S_IXOTH) {
                                 printf("execute");
                         }
                         printf("\n");
                 }
         }
         ```

         ```shell
         $ gccs main.c && ./a.out
         Owner: read write
         Group: read
         Other: read
         ```

   4. st_blksize is **NOT** the block size of underlying file system. Instead, it
      is the optimal block size for I/O on files on this file system. 

      > The meaning of this field varies across UNIX implementations, at least on
      > Linux, this is the optimal block size for I/O.

      This is the development result of Berlekey `fast file system`. 

      Your I/O operation buffer (userspace buffer: std buffer or buffer manually 
      allcoated by the programmer) size should be at least this value.

      ```rust
      use nix::sys::stat::stat;
      
      fn main() {
          println!(
              "Optimal blk size for I/O: {}",
              stat(".").unwrap().st_blksize
          );
      }
      ```

      ```shell
      $ cargo r -q
      Optimal blk size for I/O: 4096
      ```

4. birthtime (creating time)

   Some BSD systems support this field in struct `stat`. Linux does not.

   But Linux has a extended API `statx(2)`, which is capable of retrieving 
   this birthtime.

   Currently (2022-10-5), only glibc has a wrapper for this syscall. On other
   platforms, you have to manually call it through `syscall(2)`.

   ```c
   int statx(int dirfd, const char *restrict pathname, int flags,
                 unsigned int mask, struct statx *restrict statxbuf);
   ```

   ```c
   struct statx {
       __u32 stx_mask;        /* Mask of bits indicating
                                 filled fields */
       __u32 stx_blksize;     /* Block size for filesystem I/O */
       __u64 stx_attributes;  /* Extra file attribute indicators */
       __u32 stx_nlink;       /* Number of hard links */
       __u32 stx_uid;         /* User ID of owner */
       __u32 stx_gid;         /* Group ID of owner */
       __u16 stx_mode;        /* File type and mode */
       __u64 stx_ino;         /* Inode number */
       __u64 stx_size;        /* Total size in bytes */
       __u64 stx_blocks;      /* Number of 512B blocks allocated */
       __u64 stx_attributes_mask;
                              /* Mask to show what's supported
                                 in stx_attributes */

       /* The following fields are file timestamps */
       struct statx_timestamp stx_atime;  /* Last access */
       struct statx_timestamp stx_btime;  /* Creation */
       struct statx_timestamp stx_ctime;  /* Last status change */
       struct statx_timestamp stx_mtime;  /* Last modification */

       /* If this file represents a device, then the next two
          fields contain the ID of the device */
       __u32 stx_rdev_major;  /* Major ID */
       __u32 stx_rdev_minor;  /* Minor ID */

       /* The next two fields contain the ID of the device
          containing the filesystem where the file resides */
       __u32 stx_dev_major;   /* Major ID */
       __u32 stx_dev_minor;   /* Minor ID */
       __u64 stx_mnt_id;      /* Mount ID */
   };
   ```

   ```c
   struct statx_timestamp {
       __s64 tv_sec;    /* Seconds since the Epoch (UNIX time) */
       __u32 tv_nsec;   /* Nanoseconds since tv_sec */
   };
   ```

5. nanosecond accuracy

   Though those time structs `timespec` and `timestamp` have a `nanosecond`
   field, this is not supported by all file systems.

   For example, `JFS`, `XFS`, `ext4` and `Btrfs` do, but `ext2`, `ext3`
   and `Reiserfs` do not.


6. change atime and mtime using `utime(2)`, `utimes(2)`, `lutimes(2)`, `futimes(2)`,
   `utimenstat(3)` and `futimens(3)`
   
   > This is useful for apps such as `tar(1)` or `unzip(1)`, they use such calls
   > to reset file timestamps when unpacking an archive.

   1. utime(2)

      > Obsolete
      
      > accuracy: second

      ```c
      #include <utime.h>

      int utime(const char *filename, const struct utimbuf *times);
      ```

      ```c
      struct utimbuf {
          time_t actime;       /* access time */
          time_t modtime;      /* modification time */
      };
      ```
      
      Change the atime of `filename` to `times->actime` and `mtime` to 
      `times->modtime`

      If `times` is `NULL`, then `atime` and `mtime` will be set to the current
      time.

      NOTE: the accuracy supported by `utime(2)` is 1 second.

   2. utimes(2)
      
      > Obsolete

      > accuracy: microsecond

      ```c
      #include <sys/time.h>

      int utimes(const char *filename, const struct timeval times[2]);
      ```

      ```c
      struct timeval {
          long tv_sec;        /* seconds */
          long tv_usec;       /* microseconds */
      };
      ```

      `utimes(2)` is rather similar to `utime(2)` except that the second argument 
      changes to an array of `timeval`. Such a change brings higher accuarcy as 
      `tiemval` has a field for `microsecond`.

      `times[0]` specifies the new `atime`, `times[1]` specifies the new `mtime`.

      > Order of magnitude (time):
      > (1) second = (10^3) milisecond = (10^6) microsecond = (10^9) nanosecond

      ```c
      #include <sys/time.h>

      int futimes(int fd, const struct timeval tv[2]);
      int lutimes(const char *filename, const struct timeval tv[2]);
      ```

      `futimes(2)` is similar to `utimes(2)`, with the difference that the file
      whose timestamp is about to be modified is specified via `file descriptor`.

      `lutimes(2)` doesn't follow soft link.


   3. utimensat(3) or futimens(3)

      > accuracy: nanosecond

      ```c
      #include <fcntl.h>            /* Definition of AT_* constants */
      #include <sys/stat.h>

      int utimensat(int dirfd, const char *pathname,
                    const struct timespec times[2], int flags);
      int futimens(int fd, const struct timespec times[2]);
      ```

      1. utimenstat
         
         If `pathname` is a relative path, if `dirfd` is `AT_FDCWD`, then it is relative
         to the current working directory of the process. Or, it is relative to the 
         directory specified by `fd`.

         `times[0]` specifies `atime` and `times[1]` specifies `mtime`. If `times` is
         NULL, then both timestamps will be set to the current time.

         If `tv_nsec` field of any struct is `UTIME_NOW`, then `tv_sec` is ignored and
         the corresponding timestamp will be set to the current time. If `tv_nsec` field
         of any struct is `UTIME_OMIT`, the `tv_sec` is ignored and the corresponding
         timestamp is left unchaged.

         > This is a BIG advantage over `utimes(2)`. Using `utimes(2)`, if you just 
         > wanna set one timestamp, then you have to retrieve the privious value 
         > through `stat(2)` or somthing similar. With `utiemnstat(3)`, you can easily
         > set the `tv_nsec` field of that corresponding timestamp to `UTIME_OMIT`.

         If the `flags` argument is `AT_SYMLINK_NOFOLLOW` and the target file is a soft 
         link, then this syscall will update the timestamp of that soft link rather than
         dereferencing it.

      2. futimens(2)
         
         Update the timestamps of the target referred to by the open file descriptor `fd`.

         `times[0]` specifies `atime` and `times[1]` specifies `mtime`. If `times` is
         NULL, then both timestamps will be set to the current time.

         If `tv_nsec` field of any struct is `UTIME_NOW`, then `tv_sec` is ignored and
         the corresponding timestamp will be set to the current time. If `tv_nsec` field
         of any struct is `UTIME_OMIT`, the `tv_sec` is ignored and the corresponding
         timestamp is left unchaged.


7. When a new file is created, the owner (UID) of that file will be set to the 
   effective UID of the process that trying to create this file.

   > This `effective UID` should be replaced with `file system UID` to be accurate.
   > 
   > For more information, see 
   > [Ch9: 6](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch9:process_credentials.md)

   The GID will be set to the effective GID when system V semantic applies, or 
   will be set to the GID of the parent directory if BSD semantic applies.

   > `effective GID` should be replaced with `file system GID`.

   > On Linux, the behavior depends on whether the `set-group-ID` mode bit is set
   > on the parent directory: if that bit  is  set,  then BSD  semantics  apply;
   > otherwise, System V semantics apply.  For some filesystems, the behavior 
   > also depends on the bsdgroups and sysvgroups mount options described in mount(8).

   ```rust
   // Test the default System V behavior

   use nix::unistd::{geteuid, getegid};
   
   use std::{env::args, fs::OpenOptions};
   
   fn main() {
       println!("EUID: {} EGID: {}", geteuid(), getegid());
       let file = args().nth(1).unwrap();
       OpenOptions::new()
           .write(true)
           .create(true)
           .open(file)
           .unwrap();
   }
   ```

   ```shell
   $ cargo b -q
   $ cp target/debug/rust ./
   $ sudo chown root rust
   $ sudo chgrp root rust
   $ sudo chmod +s rust

   $ l rust
   Permissions Links Size User Group Date Modified Name
   .rwsr-sr-x@     1 6.2M root root   5 Oct 20:21  rust

   $ ./rust test
   EUID: 0 EGID: 0
   $ l test
   Permissions Links Size User Group Date Modified Name
   .rw-r--r--@     1    0 root root   5 Oct 20:26  test
   ```

   ```
   # Test the BSD semantics

   $ mkdir dir
   $ chmod g+s dir
   $ l -i dir
   Permissions Links Size User  Group Date Modified Name
   drwxr-sr-x@     1    - steve steve  5 Oct 20:29  dir
   $ cd dir
   $ ../rust test
   EUID: 0 EGID: 0

   $ l test
   Permissions Links Size User Group Date Modified Name
   .rw-r--r--@     1    0 root steve  5 Oct 20:29  dir/test
   ```
