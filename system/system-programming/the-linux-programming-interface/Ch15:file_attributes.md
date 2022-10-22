#### Ch15: File Attributes

> 1. Retrieve file information using `stat(2)/lstat(2)/fstat(2)/fstatat(2)` from `i-node`
> 2. `stat` struct
> 3. Get birthtime of a file using `statx(2)`
> 4. Change `atime` and `mtime` using `utime(2)/utimes(2)/futimes(2)/lutimes(2)/utimensat(3)/futimens(3)`
>    
>    > You should use `utimensat(3)` or `futimens(3)`
>
> 5. The ownership of a new file: EUID, EGID (system V) or GID of parent dir (BSD)
> 6. Propagation of the `set-GID` bit on directories.
> 7. Change the ownership using `chown/fchown/lchown/fchownat`
> 8. Permission on directories
>
>    > The semantics differ from the permission on files
>
> 9. Permission checking algorithm
> 10. Check whether a process has permissions at runtime using `access(2)` or `eaccess(2)`
>        
>     > `access(2)` is based on `RUID` and `eaccess(2)` is based on EUID.
>
> 11. functionality of `sticky bit` (`restricted deletion bit`)
> 12. file creatoin mask (`umask(1)` and `umask(2)`)
> 13. change file permission (last 12 bits) using `chmod(2)/fchmod(2)/fchmodat(2)`
> 14. permission of a softlink: always 0o777
> 16. I-node flags (ext2 extended file attributes): `lsattr(1)/chattr(1)`
>     
>     File Attributes:
>     1. stat struct
>     2. I-node flags
>     3. Extended Attributes (EA), see Ch16
>
> 16. summarize the usage of `set-UID/set-GID/sticky bit`.

1. `stat/lstat/fstat/fstatat`
   
   ```c
   #include <sys/stat.h>

   int stat(const char *restrict pathname, struct stat *restrict statbuf);
   // Rust: std::fs::metadata()

   int fstat(int fd, struct stat *statbuf);
   int lstat(const char *restrict pathname, struct stat *restrict statbuf);
   // Rust: std::fs::symlink_metadata()

   #include <fcntl.h>           /* Definition of AT_* constants */
   #include <sys/stat.h>

   int fstatat(int dirfd, const char *restrict pathname,
              struct stat *restrict statbuf, int flags);
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
      file **across all the file systems**

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

	 > `Hard Link` is not a kind of file type (kind of reference counter)

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

   Currently (2022-10), only `glibc` (since 2.28) has a wrapper for this syscall.
   On other platforms (`musl` and `uClibc`), you have to manually call it through
   `syscall(2)`.

   ```c
   #include <fcntl.h>           /* Definition of AT_* constants */
   #include <sys/stat.h>

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
         
         If `pathname` is a relative path, and if `dirfd` is `AT_FDCWD`, then it is relative
         to the current working directory of the process. Or, it is relative to the 
         directory specified by `fd`.

         If `pathname` is absolute, then `dirfd` is ignored.

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
   > also depends on the `bsdgroups` and `sysvgroups` mount options described in mount(8).

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

8. When the `set-GID` bit is set on a directory, then it is also set on its child
   directories.

   ```shell
   $ mkdir dir1
   $ chmod g+s dir1
   $ l -d dir1
   Permissions Links Size User  Group Date Modified Name
   drwxr-sr-x@     1    - steve steve  7 Oct 07:11  dir1

   $ cd dir1
   $ mkdir dir2
   $ l -d dir2
   Permissions Links Size User  Group Date Modified Name
   drwxr-sr-x@     1    - steve steve  7 Oct 07:13  dir2
   ```

   This `set-GID` can be propagated.

9. Change the ownership of a file

   > Only  a privileged process (Linux: one with the CAP_CHOWN capability) may 
   > change the **owner** of a file.  The owner of a file may change the **group** of 
   > the file to **any group of which that owner is a member**.  A **privileged process
   > (Linux: with CAP_CHOWN)** may change the group **arbitrarily**.
   >
   > If the UID **or** GID of a file is changed, then the `set-UID` **and** `set-GID`
   > will be discarded (if it has) for security reason. This is actually to 
   > prevent the case where a normal user has a set-UID program and want to
   > change the UID of that program to someone with priviledge (e.g., root)
   > , thereby gaining that priviledge.
   > 
   > One exception: When the `group-execute` permission is already turned off
   > or the file itself is a directory, then changing the ownership of that
   > file will not turn off the set-GID bit. In both cases, the set-GID is not
   > used to create a set-GID program, so there is no permission security issue 
   > here.
   >
   > SUSv3 leave it unspecified whether the set-UID and set-GID bits should be
   > turned off when the ownership is changed by a priviledged user. Linux
   > does do this.

   > The relationship between the following syscalls are similar to the one in
   > the `stat()` family.
   
   1. chown(2)
      
      ```c
      #include <unistd.h>

      int chown(const char *pathname, uid_t owner, gid_t group);
      ```

      This call will follow soft link, if `onwer` or `group` is set `-1`, then
      the corresponding ownership will not be changed.

      > -1 is the maximum value

   2. fchown(2)
      
      ```c
      #include <unistd.h>

      int fchown(int fd, uid_t owner, gid_t group);
      ```

      If `onwer` or `group` is set `-1`, then the corresponding ownership will 
      not be changed.

      > -1 is the maximum value

   3. lchown(2)
      
      ```c
      #include <unistd.h>

      int lchown(const char *pathname, uid_t owner, gid_t group);
      ```

      If `onwer` or `group` is set `-1`, then the corresponding ownership will 
      not be changed.

      > -1 is the maximum value

      Similar to `chown(2)`, except that this will not follow softlink.

   4. fchownat(2)
      
      ```c
      #include <fcntl.h>           /* Definition of AT_* constants */
      #include <unistd.h>

      int fchownat(int dirfd, const char *pathname,
                   uid_t owner, gid_t group, int flags);
      ```

      If `pathname` is relative, `pathname` is relative to `dirfd`, if `dirfd` 
      is `AT_FDCWD`, then `pathname` if relative to the current working directory 
      of the process.

      If pathname is absolute, then dirfd is ignored

      If `onwer` or `group` is set `-1`, then the corresponding ownership will 
      not be changed.

      > -1 is the max value.

      `flags`:

      * AT_EMPTY_PATH (since Linux 2.6.39): If pathname is an empty string, operate
        on the file referred to by dirfd (which may have been obtained using the 
        open(2) O_PATH flag).  In this case, **dirfd can refer to any type of file**, 
        not just a directory.  If dirfd is AT_FDCWD, the call operates on the current
        working directory.  This  flag  is  **Linux-specific**; define **_GNU_SOURCE**
        to obtain its definition.

        > Then why not use `fchown(2)`? the `fd` argument of `fchown(2)` has to be an
        > open file descriptor. This is not necessarily needed for `fchownat(2)`

      * AT_SYMLINK_NOFOLLOW: If  pathname  is  a symbolic link, do not dereference
        it: instead operate on the link itself, like lchown().  (By default, fchownat()
        dereferences symbolic links, like chown().)

        > Same as `lchown`, you should use this whening trying to change the ownership
        > of a soft link.


10. obtain the file descriptor from filepath without opening it
    
    `open(2)` has a flag `O_PATH`, for more info, see `man 2 open`.

11. Permissions on directories

    > Permissions on directories use the same schema as files. However, they
    > meaning changed a lot.

    * read: the contents (directory itself) of the directory can be listed 
      (e.g., using `ls(1)`), can not access its file because that needs `x`
      permission.
      
      ```shell
      $ mkdir dir
      $ touch dir/file
      $ chmod -x dir
      $ l -d dir
      Permissions Links Size User  Group Date Modified Name
      drw-r--r--@     1    - steve steve  8 Oct 08:26  dir

      # `ls(1)` can read a directory when `r` is given.
      $ /usr/bin/ls dir
      file

      # For some distro or shell, `ls` is aliased to include flags (e.g., `-F`)
      # Such a flag will query the i-nodes of the files in the directory, which needs the `x` permission.
      $ ls dir
      ls: cannot access 'dir/file': Permission denied
      file

      # exa also needs the `x` permission
      $ exa dir
      [dir/file: Permission denied (os error 13)]
      ```

    * write: files can be added or removed in that direcotry
      
      > `x` is also necessary to create or delete a file.

    * execute: files within the directory may be accessed. It is also
      called search permission.

    > For a directory, if you don't have the `x` permission, then all you can do is
    > using `ls(1)` to read it.
      

    When accessing a file, execute permission is required on all of the directories
    listed in the pathname. For example, reading `/home/steve/Desktop/file` will need
    the execution permission for `/`, `/home`, `/home/steve` and `/home/steve/Desktop`.

    If the current working directory is `/home/steve/dir1`, and we access 
    `../dir2/file`, we need the `x` permissions for `/home/steve` and `/home/steve/dir2`.

    If we have the `x` permission for a directory and `r` permission for its file,
    and we know the name of that file, we can read it.

    ```shell
    $ l -d dir
    Permissions Links Size User  Group Date Modified Name
    d-wx--x--x@     1    - steve steve  8 Oct 09:07  dir

    $ cat dir/file
    hel
    ```

12. Permission checking algorithm

    1. If the process is priviledged, then **all permissions** are granted.

       > Not that correst, for a file that is NOT directory, Linux grants execution
       > permission only if at least one `x` permission is granted to the three categories
       >
       > This may not be true on other UNIX implementations.

       ```shell
       $ touch test
       $ echo "echo \"hello\"" > test
       $ chmod 000 test
       $ sudo ./test
       sudo: ./test: command not found

       $ chmod o+x test
       $ sudo ./test
       hello
       ```

    2. If the EUID (file system UID on Linux) matches the owner of that file,
       then the permissions for owners are granted.
    3. If the EGID (file system GID on Linux) or supplementary groups match
       the group of that file, then group permissions are granted.
    4. Otherwise, permissions for others are granted.

    > This check process is executed in order and **stops as soon as the rule
    > matches**. And this may lead to some unexpected behavior:
    >
    > ```shell
    > $ l test
    > Permissions Links Size User  Group Date Modified Name
    > .r--rw-r--@     1    0 steve steve  8 Oct 09:25  test
    > 
    > $ echo "hello" > test
    > warning: An error occurred while redirecting file 'test'
    > open: Permission denied
    > ```
    >
    > EUID (fs UID to be accurate) matches the owner, then the owner permission
    > is granted, `r--`, thereby writing failed even though `group` has the write
    > permission.

    > Linux 2.6 introduces `access control lists (ACL)`, which makes it possible
    > to define file permissions on a per-user and per-group basis.
    > for more info, see [Ch17]

13. check whether a process has permission at runtime using `access(2)`

    > This check is based on REAL UID rather than the EFFECTIVE UID.
    >
    > This  allows set-user-ID programs and capability-endowed programs to easily 
    > determine the **invoking user**'s authority.  In other words, access() does not
    > answer the "can I read/write/execute this file?" question.  It answers a 
    > slightly different  question:  "(assuming  I'm  a  setuid  binary)  can  the
    > **user  who  invoked  me** read/write/execute  this file?", which gives set-user-ID
    > programs the possibility to **prevent malicious users from causing them to read 
    > files which users shouldn't be able to read** (prevent you from using the super
    > power gained by set-UID bit).
    >
    > For the check based on `EUID`, use `eaccess(3)` or `euidaccess(3)`
    >
    > ```c
    > #define _GNU_SOURCE             /* See feature_test_macros(7) */
    > #include <unistd.h>
    >
    > int euidaccess(const char *pathname, int mode);
    > int eaccess(const char *pathname, int mode);
    > ```

    > You have already used this in c to check whether a file exists
    >
    > [What's the best way to check if a file exists in C](https://stackoverflow.com/a/230068/14092446)

    ```c
    #include <unistd.h>

    // follow soft links
    int access(const char *pathname, int mode);

    #include <fcntl.h>            /* Definition of AT_* constants */
    #include <unistd.h>

    int faccessat(int dirfd, const char *pathname, int mode, int flags);
                    /* But see C library/kernel differences, below */
    ```

    `mode` argument:

    * F_OK: does the file exist?
    * R_OK: can the file be read?
    * W_OK: can this file be wrtten?
    * X_OK: can this file be executed? 

    > This time gap between this syscall and the subsequent file operations means
    > there is no guarantee that this info is still valid at that time.
    > 
    > For example, EUID (fs UID) is changed after `access(2)`.
    >
    > For info about modification to `EUID`, see 
    > [Ch9 8 2](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch9:process_credentials.md)

14. functionality of `sticky bit`
  
    On some older UNIX implementatoins, `sticky bit` is used to make process faster.
    Once the `sticky bit` of a executable file is set, then this file is saved to
    the swap space (sticks in swap), and loads faster on subsequent executions.
    Mode UNIX impls use a more sophisticated memory management, making this 
    function of `sticky bit` obsolete.

    > The constants `S_ISVTX`, derives from an alternative name for the sticky
    > bit, `the saved-text bit`.

    Nowodays, this bit is only useful on directories, and gains a new name:
    restricted deletion and rename bit. When this bit is set on a dir, unpriviledged
    users can `unlink (unlink()/rmdir())` or `rename (rename())` files in the
    directory only if they have `w + x` permisions on that dir **and** own
    either the file or directory. **This makes it possible to create a directory
    that is shared by many users, who can create and delete their own files
    in the directory but can't delete files owned by other users**.

    > `/tmp` is an example
    >
    > ```shell
    > l -d /tmp
    > Permissions Links Size User Group Date Modified Name
    > drwxrwxrwt@    25    - root root   8 Oct 15:59  /tmp
    > ```
    > Normal users have `w + x` permissions for `/tmp`, but they dont't own `/tmp`.
    > So for files that are owned by others, they have no ability to delete or 
    > rename those files.

    
15. file creation mask 
    
    When creating new files or directories, one can explicitly set the permission
    (last 12 bits in mode_t) in the `mode` argument of `open(2)` or `mkdir(2)`.

    ```c
    int open(const char *pathname, int flags, mode_t mode);
    int mkdir(const char *pathname, mode_t mode);
    ```

    But this is just a request, the actual permission will be `mode - umask`.
   
    `umask` is a **process attribute** that specifies which permission bits should
    always be turned off when creating new files or directories.

    For a new process, this attribute is inherited from its parent process.
    
    > What about the first process, I guess it uses the default value `0o222`

    A process can change this inheritance through `umask(2)`.

    > Shell usually has a built-in command `umask`, which is a wrapper around
    > this syscall.

    ```c
    #include <sys/stat.h>

    mode_t umask(mode_t mask);
    // always successful and return the previous umask
    ```

    > How can we just request current `umask` value without changing it? Call it 
    > twice!
    >
    > ```c
    > mode_t fetch_umask(void) {
    >     mode_t prev = umask(0);
    >     umask(prev);
    >     return prev;
    > }
    > ```
    > 
    > But such a method is dangerous in multi-threads programs.
    > There is a [GNU extension function](https://man7.org/linux/man-pages/man3/getumask.3.html) 
    > tries to provide this functionality in a thread-safe way:
    >
    > ```c
    > #define _GNU_SOURCE
    > #include <sys/types.h>
    > #include <sys/stat.h>
    >
    > mode_t getumask(void);
    > ```
    >
    > Though there in no wrapper in current glibc.


16. change file permissions using `chmod/fchmod/fchmodat`

    > use `fchmodat` rather than other functions
    
    ```c
    #include <sys/stat.h>

    int chmod(const char *pathname, mode_t mode);
    int fchmod(int fd, mode_t mode);

    #include <fcntl.h>           /* Definition of AT_* constants */
    #include <sys/stat.h>

    int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
    ```

    To change the permission of a file, a process should be either:
    1. priviledged (CAP_FOWNER)
    2. EUID (fs UID) matches the owner of that file.

    > A security measure: when a process tries to set the permission of a file
    > and the group of that file does not match EGID or any of supplementary
    > group IDs, the `set-GID` bit is awlays cleared.
    >
    > ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-08_21-15-56.jpg)
    > 
    > I think this is a flaw of that BSD semantics

17. When a soft link is created, all permissoins are enabled by default.
    And these permissions can NOT be changed.

    ```shell
    $ ln -s test link
    $ l link
    Permissions Links Size User  Group Date Modified Name
    lrwxrwxrwx@     1    4 steve steve  8 Oct 20:54  link -> test

    $ chmod 666 link
    $ l link
    lrwxrwxrwx@     1    4 steve steve  8 Oct 20:54  link -> test
    ```

18. I-node flags (ext2 extended file attributes)

    > Some file systems support i-node flags, `ext2` was the first fs supporting
    > this, and thus this attribute is also  called `ext2 extended file attributes`.
    > Subsequently, support for i-node flags on other file systems are added.
    > Btrfs supports this.
    > 
    > The supported flags varies across file systems.


    One can use `lsattr(1)` or `chattr(1)` to list or modify flags on a file.

    ```shell
    $ touch test
    $ lsattr test
    ---------------------- test
    
    # make this file append only and immutable
    $ sudo chattr +ai test
    
    $ lsattr test
    ----ia---------------- test
    ```

    Within a program, one can use [`ioctl_iflags(2)`](https://man7.org/linux/man-pages/man2/ioctl_iflags.2.html) 
    to retrieve and modify flags.

    Flags:

    > The detailed meaning of the following flags can be found in the above man page.

    ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-09_10-48-55.jpg)

    > `FS_IMMUTABLE_FL`: when this flag is set, envn a priviledged process can not
    > change the file contents and metadata.


19. summarize the usage of `set-UID/set-GID/sticky bit`.

    * set-UID: gain priviledge (only useful for compiled binaries on Linux
      because on interptreted scripts this bit is always ignored)

    * set-GID:

      1. gain priviledge (for compiled binaries)

      2. Once is set on a directory, the new files inside that directory will have
         the same group as this directory instead of the using the EGID of the 
         process that are creating this file.

	 > And the `set-GID` can propatate on directories, see note 8.

      TODO: update this when another usage is present.

    * sticky bit (restricted deletion bit) (only useful on directories nowadays)
      
      prevent users deleting or renaming files that are not owned by them.

      For detailed explanation and examples, see note 14.

      > If the `sticky bit` is set on a directory, then a user (unpriviledged) 
      > can not add `user` EA on this directory if this is owned by others, 
      > even though this `user` has the corresponding permission.
      >
      > For what is EA (Extended Attributes), see 
      > [note Ch16](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch16:extended_attributes.md)
