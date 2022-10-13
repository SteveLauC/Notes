#### Ch16: Extended Attributes (EAs)

> Btrfs supports this, see their 
> [documentation](https://btrfs.readthedocs.io/en/latest/Common-features.html#file-attributes-xflags).


> 1. what is EA?
> 2. EA namespace and name
> 3. get or set EA using `getfattr(1)/setfattr(1)`
> 4. `user` EA can be used only on `regular files/directories`
> 5. Linux Virtual File System
> 6. limitation on EA (from VFS or underlying file system)
> 7. EA syscalls: `listxattr/getxattr/setxattr/removexattr` and their variants

1. EA
   
   Extended Attribute (xattr) is a list of `key=value` pairs associated with a 
   file, usually storing additional `metadata` related to security, access control
   list in particular (ACL) or properties

   > the actual key-value format: (namespace.name, value)

   > EA was added to Linux in version 2.6

   EA is used to implement `access control lists (ACL) (Ch17)` and `file 
   capabilities (Ch39)`

2. EA namespace

   The name of EA is in form `namespace.name`, where namespace is used to classify
   the EAs into several classes, and `name` uniquely identifies a specific EA 
   inside that `namespace`.

   EA namespaces are:

   1. user

      `user` EA can be manipulated by unpriviledged users, subject to the file 
      permission:

      1. retrieving the value needs `r` permission on the file
      2. changing the value needs `w` permissoin on the file

      > To enable `user` EA on `ext2/3` and `Reiserfs`, a monut option `user_xattr`
      > is needed when mounting the file system 
      >
      > (You don't need this on `ext4` and `btrfs`)
      >
      > ```
      > $ mount -o user_xattr device directory 
      > ```

   2. trusted
     
      Only accessable and visible to processes with `CAP_SYS_ADMIN` capability

   3. system

      Used by the kernel to associate `system objects` with a file, currently,
      the only supported object is `access control list (ACL)`.

   4. security

      Used to store `file security labels` 

3. EA `name`
  
   In `namespace` `user` and `trusted`, EA `name`s can be arbitrary strings.
   In `system`, `name` need to be permitted by the kernel.

4. get and set EA from using `setfattr(1)/getfattr(1)`

   One can use `setfattr(1)` and `getfattr(1)` to set or get EA.

   ```shell
   $ setfattr -n <namespace.name> -v <value> <target_file>
   $ getfattr -d <target_file> # list all EAs
   $ getfattr -n <namespace.name> <target_file> # list the value of <namespace.name>

   # getfattr <target_file> will not display the EA value, just the name. To
   # enable it, use the `-d` option
   ```

   ```shell
   $ touch file
   $ getfattr -d file # No EA set on a new file
   $ setfattr -n user.my_first_ea -v "this is my first EA" file
   $ getfattr -d file
   # file: file
   user.my_first_ea="this is my first EA"

   $ setfattr -n user.my_first_ea # set the value of <user.my_first_ea> to an empty string
   $ getfattr -n user.my_first_ea
   # file: file
   user.my_first_ea=""

   $ setfattr -x user.my_first_ea file # delete this EA
   $ getfattr -d file
   ```

5. `user` EA can be used only on `regular files` and `directories`

   1. `user` EA uses file permission to control who can access it, the permission
      on a `soft link` is always `0o777`, which means anyone can access the `user`
      EA of a `soft link` if this is allowed, this is dangerous, so this is disabled.

   2. For other 4 file types like `blk device/char device/FIFO/socket`, the 
      permission on them is used to control the access that users are granted
      for the purpose of performing I/O on the underlying object. Manipulating
      these permissions to control the access of `user` EA would conflict with
      this.

   > For direcories, if the `sticky bit` is set on a directory, then a user
   > can not access the `user` EA on that directory if it is owned by someone
   > else. (This can be seen as the second functionality of `sticky bit`, 
   > though it is still designed to work with the previous functionality.)

6. limitation on EA

   1. From Linux Virutal File System
      
      1. The length of an EA `name` is limited to 255 characters.
      2. An EA `value` is limited to 64 KB.

   2. From underlying file system

      1. On current `ext2/3/4`, the total bytes used by all EAs on a file is limited
         to the size of a single file system block.

      2. In Btrfs, the total size used by name, value and implementaton overhead
         is limited to the file system `node size` (16 kB by default)

         > What is `node size` on Btrfs
         >
         > From [this page](https://btrfs.readthedocs.io/en/latest/mkfs.btrfs.html)
         >
         > > Specify the nodesize, the tree block size in which btrfs stores metadata.
         >
         > Then what is `tree block size`? I think this is a implementatoin term.
  
   For more info about `limitation`, see section `Filesystem differences` in 
   [man 7 xattr](https://man7.org/linux/man-pages/man7/xattr.7.html)

7. EA-related syscalls 

   1. retrieve the `name`s of all EAs associated with a file
      
      ```c
      #include <sys/xattr.h>

      ssize_t listxattr(const char *path, char *list, size_t size);
      ssize_t llistxattr(const char *path, char *list, size_t size);
      ssize_t flistxattr(int fd, char *list, size_t size);
      ```

      Return all the EA `name`s associated with the file specified by `path`, the
      `name`s will be put in a user-allocated buffer `list`, `size` is the size
      of that buffer.

      If `size` is 0, then `list` argument will be ignored. Then the function
      will return the size of those `name`s. One can use this to set a approprate
      buffer size. (though these may be some `time-of-check-time-of-use` issues).

      `name`s will be returned in the format of `namespace.name\0namespace.name\0`.
