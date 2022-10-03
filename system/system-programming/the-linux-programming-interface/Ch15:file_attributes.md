#### Ch15: File Attributes

> 1. retrieve file information using `stat(2)/lstat(2)/fstat(2)` from `i-node`

1. `stat/fstat/lstat`
   
   ```c
   #include <sys/stat.h>

   int stat(const char *restrict pathname, struct stat *restrict statbuf);

   int fstat(int fd, struct stat *statbuf);
   int lstat(const char *restrict pathname, struct stat *restrict statbuf);
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

      This is the development result of Berlekey `fast file system`. 

      Your I/O operation buffer (userspace buffer: std buffer or buffer manually 
      allcoated by the programmer) size should be at least this value.

      ```rust
      use nix::sys::stat::stat;
      
      fn main() {
          println!(
              "file system blk size obtained through `statfs`: {}",
              stat(".").unwrap().st_blksize
          );
      }
      ```

      ```shell
      $ cargo r -q
      file system blk size obtained through `statfs`: 4096
      ```
