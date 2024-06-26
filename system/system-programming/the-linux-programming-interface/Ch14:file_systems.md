#### Ch14: File Systems

> 1. concept: device speical file
> 2. concept: disk and partition
> 3. concept: file system in userspace (FUSE)
> 4. concept: file system and its components (bootblock/superblock/inode/datablock)
> 5. concept: I-node (storing metadata and ptrs to data block)
> 6. concept: VFS (an abstraction layer)
> 7. the advantage of journaling file systems: consistency check
> 8. list all the mount points: `mount(8)`, `/etc/mounts`, `/etc/mtab`, `/etc/fstab`
> 9. `mount(2)` and `unmount(2)/umount2(2)` syscalls
> 10. advanced mount features
>     1. mount the same file system at multiple locations
>     2. stack multiple file systems at the same mount point
>     3. per-mount `mount(2)` flags
>     4. bind mounts (created using `mount(2)` `MS_BIND` flag)
>     5. recursive bind mounts
> 11. relationship between `file system` and `mount points`
> 12. tmpfs: a virtual memory file system
>     > `/tmp` uses it to speed it up.
> 13. obtain file system info using `statvfs(3)/fstatvfs(3)`
> 14. OS-specific syscalls `statfs/fstatfs`
   
1. what is `device special file`
  
   Just a file representing a real or virtual device. For each device, there are 
   two numbers:
   1. major ID number: identifies the general class of the device, and is used to
      look up the appropriate drivers for this type of device.
   2. minor ID number: The minor ID number is used to uniquely identify a specific
      device within a device class.

   ```shell
   $ cd /dev/input
   $ ls -l
   total 0
   drwxr-xr-x. 2 root root     140 Sep 17 07:06 by-id
   drwxr-xr-x. 2 root root     160 Sep 17 07:06 by-path
   crw-rw----. 1 root input 13, 64 Sep 17 07:06 event0
   crw-rw----. 1 root input 13, 65 Sep 17 07:06 event1
   crw-rw----. 1 root input 13, 74 Sep 17 07:06 event10
   crw-rw----. 1 root input 13, 75 Sep 17 07:06 event11
   crw-rw----. 1 root input 13, 76 Sep 17 07:06 event12
   crw-rw----. 1 root input 13, 66 Sep 17 07:06 event2
   crw-rw----. 1 root input 13, 67 Sep 17 07:06 event3
   crw-rw----. 1 root input 13, 68 Sep 17 07:06 event4
   crw-rw----. 1 root input 13, 69 Sep 17 07:06 event5
   crw-rw----. 1 root input 13, 70 Sep 17 07:06 event6
   crw-rw----. 1 root input 13, 71 Sep 17 07:06 event7
   crw-rw----. 1 root input 13, 72 Sep 17 07:06 event8
   crw-rw----. 1 root input 13, 73 Sep 17 07:06 event9
   crw-rw----. 1 root input 13, 63 Sep 17 07:06 mice
   crw-rw----. 1 root input 13, 32 Sep 17 07:06 mouse0
   crw-rw----. 1 root input 13, 33 Sep 17 07:06 mouse1
   ```

   You can see that all device files under `/dev/input` have the same major ID
   number, meaning they belong to the same class. But they do have different
   minor ID numbers as they are different devices.

   > On Linux 2.4 and eariler, the total number for the devices are limited by
   > the fact that `major ID number` and `minor ID number` are all represented
   > in a 8-bit number. This was eased in kernel 2.6 as they switched to more
   > bits (12 and 20 respectively).
   >
   > Now they all have 32 bits, see [In Posix how is type dev_t getting used][link]
   > [link]: https://stackoverflow.com/a/73923376/14092446

   How to create a `device special file`?

   You can do this through `mknod(1)` or `mknod(2)`.

   > Actually, you can create regular file (which will be created empty),  
   > *character  special  file, block special file*, FIFO (named pipe), or 
   > UNIX domain socket throught `mknod(2)`.
   >
   > Seems that you cannot create regular file through `mknod(1)`.

2. each partition is treated as a separate device residing under `/dev`

   One can use `fdisk` command to operate on partitions.

   The Linux-specific file `/proc/partitions` lists major and minor device number,
   number of blocks and name.

   ```
   major minor  #blocks  name
   
    259        0  488386584 nvme0n1
    259        1     614400 nvme0n1p1
    259        2    1048576 nvme0n1p2
    259        3  486722560 nvme0n1p3
    252        0    8388608 zram0
   ```

   A partition may contain any type of information, but usually contains one
   of the following:
   * a file system
   * a data area (accessed as raw-mode device, typicall used by a DBMS)
   * a swap area

3. Linux supports a variety of file systems. To view them:
  
   ```shell
   $ cat /proc/filesystems
   nodev        sysfs
   nodev        tmpfs
   nodev        bdev
   nodev        proc
   nodev        cgroup
   nodev        cgroup2
   nodev        cpuset
   nodev        devtmpfs
   nodev        configfs
   nodev        debugfs
   nodev        tracefs
   nodev        securityfs
   nodev        sockfs
   nodev        bpf
   nodev        pipefs
   nodev        ramfs
   nodev        hugetlbfs
   nodev        devpts
    ext3
    ext2
    ext4
   nodev        autofs
   nodev        efivarfs
   nodev        mqueue
   nodev        selinuxfs
   nodev        binder
   nodev        resctrl
    btrfs
   nodev        pstore
    fuseblk
   nodev        fuse
   nodev        fusectl
    vfat
   nodev        rpc_pipefs
   ```

4. FUSE (file system in userspace)

   Allows the non-priviledged user to create their own file system without editing
   the kernel code.

   [homepage](https://github.com/libfuse/libfuse)

   > Future steve: On Linux, mounting and unmounting a normal file system requires
   > priviledge (`CAP_SYS_ADMIN`). However, with FUSE, this is not needed.

5. file system structure

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-09-21_10-47-52.jpg)

   * boot block: This is always the first block in a file system. The boot block
     is for the OS. Though there is only one boot block that will be used by the
     OS, all files systems contain a boot block.

   * super block: contains the following information
     * the size of the i-node table
     * logical block size (file system block size)
     * the size of file system in logical blocks

   * i-node table (i-list): each file or directory will has a unique entry in 
     this table. (i stands for *index*)

     > The i-node table is numbered starting at 1 instead of 0, because 0 is 
     > used to indicate that this entry is unused (like NULL). I-node 1 is 
     > used to track bad blocks. On ext* file system, the root of a partition
     > has i-node 2. Btrfs uses 256.

   * Data blocks: data that forms the files or directories
     

6. i-node entry

   A i-node table contains a lots of i-node entries (each for a file), and each 
   i-node entry is identified by a unique number called inode number or i-number.

   What a i-node entry contains:

   1. File type
   2. UID
   3. GID
   4. Access Permission
   5. Timestamps: access time (last read, atime), change time (last write to 
      metadata, ctime), modification time (last write to file contents, mtime)
   6. Number of hard links
   7. Size of the file in bytes
   8. Number of allocated blocks (block size = 512 bytes)
   9. Pointer to data blocks.

   > I-node entry does not contain `filename`, it is stored `directory`. because
   > of this, we can have multiple `filename`s referring to the same i-node entry,
   > this is `hard link` (hard link is not a kind of file type).


7. Pointers to data blocks in i-node entry

   Take ext2 as an example:
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-09-21_12-32-29.jpg)

   Under ext2, each i-node entry has 15 pointers. The first 12 are direct pointers.
   Number 12 (the thirteenth one) is an indirect pointer pointing to a block of 
   pointers. Number 13 is a double-indirect pointer pointing to an indirect pointer.
   Number 14 is a triple-indirect pointer.

   Such a design has the following advantages:
   1. Small files can be accessed rapidly because 12 direct pointers are sufficient
      for them.
   2. It allows i-node entry to be **fixed-sized**.
   3. File can reside on the disk incontiguously.
   4. Data can be randomly accessed through `lseek(2)` (just calculate which 
      pointer to follow)

8. Virtual File System (VFS) (or Virtual File Switch)

   Different file systems differ in its detailed implementations, VFS is an 
   abstraction layer to hide such differences.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-09-23_09-26-20.jpg)

   This is actually a collection of APIs, such as:
   `open/read/write/mount/lseek/close/truncate/stat/unmount/mmap/mkdir/link/unlink`, etc.

   > Some file system may not support all the APIs requested by VFS (e.g., 
   > Microsoft VFAT does not support symbolic links). In this case, then
   > underlying file system will return an error to VFS, and VFS will transfer
   > this to the Application.

9. The advantage of journaling file systems

   Normally, a consistency check is performed on reboot in order to ensure
   the integrity of the file system. This should be done because, at the 
   time of the system crash, a file update may be partially completed.

   Such a consistency check needs to check the whole file system, which is
   extermely time-consuming for traditional UNIX file systems (e.g, ext2).

   But this task is super easy for journaling file system as a **log** will be
   performed **before** the time of updating. Due to the log, a consistency 
   check can be easily executed by checking logs.

10. list all the mount points 
    
    1. using `mount(8)`

       ```shell
       $ mount
       /dev/sda6 on / type ext4(rw)
       proc on /proc type proc (rw,nosuid,nodev,noexec,relatime)
       sysfs on /sys type sysfs (rw,nosuid,nodev,noexec,relatime,seclabel)
       devtmpfs on /dev type devtmpfs (rw,nosuid,seclabel,size=4096k,nr_inodes=1048576,mode=755,inode64)
       securityfs on /sys/kernel/security type securityfs (rw,nosuid,nodev,noexec,relatime)
       tmpfs on /dev/shm type tmpfs (rw,nosuid,nodev,seclabel,inode64)
       devpts on /dev/pts type devpts (rw,nosuid,noexec,relatime,seclabel,gid=5,mode=620,ptmxmode=000)
       tmpfs on /run type tmpfs (rw,nosuid,nodev,seclabel,size=3151336k,nr_inodes=819200,mode=755,inode64)
       ...
       ```

    2. read from the Linux-specific file `/proc/mounts`

       ```shell
       $ ls /proc/mounts
       Permissions Links Size User Group Date Modified Name
       lrwxrwxrwx@     1   11 root root  23 Sep 07:12  /proc/mounts -> self/mounts

       $ cat /proc/mounts
       proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0
       sysfs /sys sysfs rw,seclabel,nosuid,nodev,noexec,relatime 0 0
       devtmpfs /dev devtmpfs rw,seclabel,nosuid,size=4096k,nr_inodes=1048576,mode=755,inode64 0 0
       securityfs /sys/kernel/security securityfs rw,nosuid,nodev,noexec,relatime 0 0
       tmpfs /dev/shm tmpfs rw,seclabel,nosuid,nodev,inode64 0 0
       devpts /dev/pts devpts rw,seclabel,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000 0 0
       tmpfs /run tmpfs rw,seclabel,nosuid,nodev,size=3151336k,nr_inodes=819200,mode=755,inode64 0 0
       ...
       ```

       > The reason why `/proc/mounts` is a soft link is that Linux introduces
       > [per-process mount namespace](https://man7.org/linux/man-pages/man7/mount_namespaces.7.html)

       > For the file format, see note 12.

    3. read from `/etc/mtab`

       This file is maintained by `mount(8)` and `unmount(8)` and silimar to 
       `/proc/mounts` but more detailed.

       ```shell
       $ cat /etc/mtab
       proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0
       sysfs /sys sysfs rw,seclabel,nosuid,nodev,noexec,relatime 0 0
       devtmpfs /dev devtmpfs rw,seclabel,nosuid,size=4096k,nr_inodes=1048576,mode=755,inode64 0 0
       securityfs /sys/kernel/security securityfs rw,nosuid,nodev,noexec,relatime 0 0
       tmpfs /dev/shm tmpfs rw,seclabel,nosuid,nodev,inode64 0 0
       devpts /dev/pts devpts rw,seclabel,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000 0 0
       tmpfs /run tmpfs rw,seclabel,nosuid,nodev,size=3151336k,nr_inodes=819200,mode=755,inode64 0 0
       ...
       ```

       > NOTE: `mount(2)` and `unmount(2)` will not edit this file so it may be
       > inaccurate if the caller forget to update it.

       > For the file format, see note 12.

11. `/etc/fstab`
  
    This file contains all the descriptions of  available file systems on a system.
    It should be edited manually by the system administrator. `mount(8)`, `unmount(8)`
    and `fsck(8)` relies on it.

    ```shell
    $ cat /etc/fstab
    #
    # /etc/fstab
    # Created by anaconda on Wed Aug 24 18:04:46 2022
    #
    # Accessible filesystems, by reference, are maintained under '/dev/disk/'.
    # See man pages fstab(5), findfs(8), mount(8) and/or blkid(8) for more info.
    #
    # After editing this file, run 'systemctl daemon-reload' to update systemd
    # units generated from this file.
    #
    UUID=6d21f1be-9640-4f47-8d22-847269e24fc6 /                       btrfs   subvol=root,compress=zstd:1 0 0
    UUID=6aa0611f-8869-468e-acf4-acf0dfd66d1f /boot                   ext4    defaults        1 2
    UUID=0296-E38B          /boot/efi               vfat    umask=0077,shortname=winnt 0 2
    UUID=6d21f1be-9640-4f47-8d22-847269e24fc6 /home                   btrfs   subvol=home,compress=zstd:1 0 0
    ```

    > functions like `getfsent(3)` and `getmntent(3)` can be used to sequencely
    > scan `/etc/fstab`

    > For the file format, see note 12.

12. file format of `/proc/mount`, `/etc/mtab` and `/etc/fstab`

    ```
    /dev/sda9 /boot ext3 rw 0 0
    ```

    * device special file
    * mount point
    * file system type
    * mount flags: `rw` in the above example indicates that the file system was 
      mounted read-write. (see note 13)
    * A number used to control the operation of file system backups by `dump(8)`.
      This field and the next are used only by `/etc/fstab`. For `/proc/mounts`
      and `/etc/mtab`, these fields are always `0`.
    * A number used to control the order in which `fsck(8)` checks file systems
      at boot.

    For detailed format, see [man 5 fstab](https://man7.org/linux/man-pages/man5/fstab.5.html)

13. mount a file system using `mount(2)`
    
    ```c
    #include <sys/mount.h>

    int mount(const char *source, const char *target,
              const char *filesystemtype, unsigned long mountflags,
              const void *data);
    ```

    Arguments: 
    * source: device special file
    * target: target directory
    * filesystemtype: string representing a file-system type
    * mountflags: mount option
    * data: buffer whose contents will be interpreted by the underlying file system.

    Some Mount flags: (For full information, see 
    [man 2 mount](https://man7.org/linux/man-pages/man2/mount.2.html) and TLPI 
    page: 264-267)

    * MS_BIND: Create a bind mount. If this flag is present, `fstype`, `mountflags`
      and `data` are ignored.

      > [what is bind mount](https://unix.stackexchange.com/q/198590/498440)
      >
      > Or see `Ch14: 15 4`

    * MS_DIRSYNC: make directory updates synchronous. Similar to `O_SYNC` of 
      `open(2)` but just for direcoties.
   
    * MS_SYNCHRONOUS: make data and directory modifications synchronous.

    * MS_NOATIME: don't update atime in this file system.
      
      > This can improve the performance significantly in some applications.

    * MS_NOEXEC: don't allow binaries or scripts to be executed from this file
      system.

    * MS_NOSUID: Disable set-UID and set-GID programs in this fs.
      > Useful if you don't trust the contents of an external disk.

    * MS_RDONLY: mount the file system read-only.

14. unmount a fs using `umount(2)/umount2(2)`
    

    ```c
    #include <sys/mount.h>

    int umount2(const char *target, int flags);

    // `umount()` is `umount2()` with empty `flags`
    int umount(const char *target);
    ```

    > Before kernel 2.2, the `target` argument can be either the `device special 
    > file` or the `mount point`. Since single file system can be mounted at 
    > multiple locations, this flexibility has been not allowed since kernel 2.4.
    > For the info about this feature, see `note 15 1`


    > And it's impossible to `unmount` a busy file system (i.e., if there are
    > open files on this fs, call `umount(2)` will yeild `EBUSY`)


    Availables `flags` to `umount(2)`:
    
    * MNT_DETACH: Performs a lazy unmount. Mark this file system unavailable to
      new processes. For processes that are currently using this fs, does nothing.
      And the fs will be unmounted after these processes are terminated.

    * MNT_EXPIRE: Mark the mount as expired.  If a mount is not currently in use, 
      then an initial call to umount2() with this flag fails with the error EAGAIN, 
      but marks the mount as `expired`. The mount remains `expired` as long as it 
      isn't accessed by any process.  A second umount2() call specifying MNT_EXPIRE
      unmounts an expired mount. This flag cannot be specified with either MNT_FORCE
      or MNT_DETACH. **This provides a mechanism to unmount a file system that
      hasn't been used for some period of time.**

      > So the typical usage of this flag would be: 
      >
      > 1. call `umount2("mount_point", MNT_EXPIRE)`, you get `EAGAIN`, and this
      >    fs is marked as `expired`.
      > 2. after a period a time, call `umount2("mount_point", MNT_EXPIRE)` again.
      >    If no operation performed on this fs in this period of time, so the
      >    fs remains `expired`, then this fs will be unmounted.

    * MNT_FORCE: forcefully unmount this file system, supported on only few file
      systems, see man page (man 2 umount).

    * UNMOUNT_NOFOLLOW: don't dereference if `target` is a soft link. This is 
      needed to prevent symlink attacks in unprivileged unmounts


15. some advanced `mount` features
    
    1. mount the same file system at multiple locations
       
       And changes made to one location will also be visible to other places.

       Demo:

       ```shell
       $ mount |grep /dev/sda3
       /dev/sda3 on /run/media/steve/writable type ext4 (rw,nosuid,nodev,relatime,seclabel,errors=remount-ro,uhelper=udisks2)

       $ mkdir test
       $ sudo mount /dev/sda3 test
       /dev/sda3 on /run/media/steve/writable type ext4 (rw,nosuid,nodev,relatime,seclabel,errors=remount-ro,uhelper=udisks2)
       /dev/sda3 on ./test type ext4 (rw,relatime,seclabel,errors=remount-ro) # precise path is omitted.

       $ cd test
       $ touch file
       $ l
       .rw-r--r--@     1    0 root root  25 Sep 11:13  file

       $ l /run/media/steve/writable 
       .rw-r--r--@     1    0 root root  25 Sep 11:13  file
       ```

    2. stack multiple file systems at the same mount point
       
       > Before kernel 2.4, a mount point can be used only once. Since 2.4, Linux
       > allows multiple mounts to be stacked on a single mount point.

       ```shell
       $ sudo mount /dev/sda3 test # first fs at `test`
       
       $ l test
       Permissions Links Size User Group Date Modified Name
       .rw-r--r--@     1    0 root root  25 Sep 11:13  file
       drwx------      2    - root root  25 Aug 19:54  lost+found
       
       $ sudo mount /dev/sda1 test # second fs at `test`
       
       # files are OVERWRITTEN

       $ l test
       Permissions Links Size User  Group Date Modified Name
       dr-xr-xr-x@     1    - steve steve 11 Aug 14:03  boot
       lr-xr-xr-x@     1   41 steve steve 11 Aug 14:03  casper -> casper_pop-os_22.04_amd64_intel_debug_178
       dr-xr-xr-x@     1    - steve steve 11 Aug 14:03  casper_pop-os_22.04_amd64_intel_debug_178
       dr-xr-xr-x@     1    - steve steve 11 Aug 14:03  dists
       dr-xr-xr-x@     1    - steve steve 11 Aug 14:03  efi
       dr-xr-xr-x@     1    - steve steve 11 Aug 14:03  isolinux
       .r--r--r--@     1  43k steve steve 11 Aug 14:03  md5sum.txt
       dr-xr-xr-x@     1    - steve steve 11 Aug 14:03  pool


       $ sudo umount test
       [sudo] password for steve:
       
       $ l test
       Permissions Links Size User Group Date Modified Name
       .rw-r--r--@     1    0 root root  25 Sep 11:13  file
       drwx------      2    - root root  25 Aug 19:54  lost+found
       ```

       ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-09-25%2011-42-21.png)

       Combined with `unmount2("point", MNT_DETACH)`, this can provide a smooth
       migration off a file system.

       
    3. per-mount `mount(2)` flags

       Since a file system can be mounted at multiple mount points, there are some
       flags that only apply to that `mount point`. (i.e., if you mount `D` at `dir1`
       with `flag F`, and mount `D` at `dir2`, then the `flag F` will only be 
       available to `dir1`).

       These flags are:

       * MS_NOATIME
       * MS_NODEV
       * MS_NODIRATIME
       * MS_NOEXEC
       * MS_NOSUID
       * MS_RONLY
       * MS_REALTIME

       Demo (with `MS_NOEXEC` flag):
       
       ```shell
       $ sudo mount -o noexec /dev/sda3 dir1
       
       $ sudo mount /dev/sda3 dir2
       
       $ tree
       .
       ├── dir1
       │   ├── file
       │   └── lost+found  [error opening dir]
       └── dir2
           ├── file
           └── lost+found  [error opening dir]
       
       5 directories, 7 files
       
       $ vim dir1/exec
       $ chmod +x dir1/exec
       $ cat dir1/exec
       
       #!/usr/bin/env bash
       
       echo "hello world";
       
       $ cd dir1 
       $ ./exec
       zsh: permission denied: ./exec
       $ sudo ./exec
       sudo: unable to execute ./exec: Permission denied
       
       $ cd ../dir2
       $ ./exec
       hello world # can be executed in `dir2`
       ```

    4. bind mounts (created using `mount(2)` `MS_BIND` flag)

       What bind mount does is to mount a dir or file that has been already in your
       file hierarchy to a new place. It is kinda similar to `hard link` but different
       in:

       1. bind mount can be performed across different file systems. (hard link can not
          as it relies on `i-node`, which is unique in a file system.)

       2. bind mount can be applied to directories. (hard link can not because it
          will break the file system in multiple ways:
          [Why are hard links not allowed for directories](https://askubuntu.com/q/210741/1417245)
          [Why are hard links to directories not allowed in UNIX/Linux?](https://unix.stackexchange.com/q/22394/498440)
          )

       Bind mount is slightly different from other features as the `source` argument
       of `mount(2)` is a `directory or regular file` rather than a `device special
       file`.

       A typical useage of `bind mount` is [`chroot jails`](https://unix.stackexchange.com/q/105/498440).
       For information about `chroot(1)/chroot(2)`, see Ch18: 39.
 
       Demo for directory source:
       
       ```shell
       $ touch dir1/file
       
       $ l dir2
       
       $ sudo mount --bind dir1 dir2
       
       $ l dir2
       Permissions Links Size User  Group Date Modified Name
       .rw-r--r--@     1    0 steve steve 25 Sep 14:48  file
       
       $ touch dir2/file2
       
       $ l dir1
       Permissions Links Size User  Group Date Modified Name
       .rw-r--r--@     1    0 steve steve 25 Sep 14:48  file
       .rw-r--r--@     1    0 steve steve 25 Sep 14:50  file2
       ```
 
       Demo for file source:
 
       ```shell
       $ cat > f1
       hence is powerful
       
       $ cat f1
       hence is powerful
       
       $ l
       Permissions Links Size User  Group Date Modified Name
       .rw-r--r--@     1   18 steve steve 25 Sep 15:14  f1
       
       $ touch f2
       
       $ sudo mount --bind f1 f2
       
       $ mount | grep f2
       /dev/nvme0n1p3 on ./f2 type btrfs (rw,relatime,seclabel,compress=zstd:1,ssd,space_cache=v2,subvolid=256,subvol=/home)
       
       $ l
       Permissions Links Size User  Group Date Modified Name
       .rw-r--r--@     1   18 steve steve 25 Sep 15:14  f1
       .rw-r--r--@     1   18 steve steve 25 Sep 15:14  f2
       
       $ cat f2
       hence is powerful
       
       $ cat >> f2 # append to f2
       new contents
       
       $ cat f2
       hence is powerful
       new contents
       
       $ cat f1
       hence is powerful
       new contents
       
       $ rm f2
       rm: cannot remove 'f2': Device or resource busy
       
       $ sudo umount f2
       
       $ rm f2
       
       $
       ```
    5. recursive bind mounts
       
       By default, if you have `bind mount`ed a directory and wanna perform 
       `bind mount` again under that directory, this is not allowed.

       Linux 2.4.11 added a flag `MS_REC` which could be used in conjunction
       (ORed) with `MS_BIND` to make that work. This is referred as `recursive
       bind mount`.
      

16. relationship between `file system` and `mount point`
   
    ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-09-25%2011-54-27.png)


17. tmpfs: a virtual memory file system

    All the file systems we have seen are disk-based. But we also have file 
    systems reside in memory (e.g., tmpfs)

    tmpfs is a `virtual memory` file system, which means it not only uses 
    memory, but also uses the swap partition.

    To create a tmpfs:

    > Note that you don't need to create a file system using `fdisk` first,
    > because the kernel will do this for you.

    ```shell
    $ mkdir mount_point
    # the `source` can be any name, it does not matter
    $ sudo mount -t tmpfs source mount_point
    $ cd mount_point
    $ echo "hello tmpfs" > hello
    $ cat hello
    hello tmpfs

    $ mount | grep source
    source on /home/steve/Desktop/point type tmpfs (rw,relatime,seclabel,inode64)


    $ cd ../
    $ sudo umount mount_point # all data is lost.
    $ rmdir mount_point
    ```

    A usage of `tmpfs` is to improve the performance of some applications, for 
    example, if an app uses `/tmp` frequently, we can stack a tmpfs on it to 
    improve the performance.

    Aside from the user app usage, `tmpfs` also servers two special purposes:
    1. impl System V shared memory insdie the kernel
    2. a tmpfs fs mounted at `/dev/shm` or `/run/shm` is used for the glibc
       implementation of POSIX shared memory and POSIX semaphores.

       ```shell
       $ mount|grep /dev/shm
       tmpfs on /dev/shm type tmpfs (rw,nosuid,nodev,seclabel,inode64)
       ```

18. obtain file system info using `statvfs(3)/fstatfs(3)`

    ```c
    #include <sys/statvfs.h>

    int statvfs(const char *restrict path, struct statvfs *restrict buf);
    int fstatvfs(int fd, struct statvfs *buf);
    ```

    `path` can be any file path inside the target file system and `fd` is a
    file descriptor referring to any file in that fs.

    ```c
    struct statvfs {
        unsigned long  f_bsize;    /* Filesystem block size */

        unsigned long  f_frsize;   /* Fragment size */
        // `f_frsize` and `f_bsize` will have the same value 
	// under most Linux file systems.
	// [What do f_bsize and f_frsize in struct statvfs stand for](https://stackoverflow.com/q/54823541/14092446)

        fsblkcnt_t     f_blocks;   /* Size of fs in f_frsize units */
        fsblkcnt_t     f_bfree;    /* Number of free blocks */
        fsblkcnt_t     f_bavail;   /* Number of free blocks for
			             unprivileged users */
        // Some systems allow root-reserverd space, in such cases, `f_bfree`
	// will be different from `f_bavail`

        fsfilcnt_t     f_files;    /* Number of inodes */
        fsfilcnt_t     f_ffree;    /* Number of free inodes */
        fsfilcnt_t     f_favail;   /* Number of free inodes for
			             unprivileged users */

        unsigned long  f_fsid;     /* Filesystem ID */
        unsigned long  f_flag;     /* Mount flags */
	// `f_flag` is the same as the `mountflag` argument of `mount(2)` except that
	// those constants are started with `ST_` rather than `MS_`

        unsigned long  f_namemax;  /* Maximum filename length */
    };
    ```

    > `fsblkcnt_t` and `fsfilcnt_t` types are integer types specified by SUSv3
    >
    > ```rust
    >
    > // GNU/Linux amd64
    > pub type fsblkcnt_t = u64;
    > pub type fsfilcnt_t = u64;
    > ```

19. `statfs(2)/fstatfs(2)`

    > On Linux, `statvfs(3)/fstatvfs(3)` are based on `statfs/fstatfs`. 
    > On some UNIX implementations, `statfs/fstatfs` are provided and
    > `statvfs/fstatvfs` are not.

    `statfs` is OS-specific, and `statvfs` is POSIX-conforming.

    [Difference between statvfs() and statfs() system calls](https://stackoverflow.com/q/1653163/14092446)


