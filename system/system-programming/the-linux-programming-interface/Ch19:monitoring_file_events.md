#### Ch19: Monitoring File Events

> ##### 19.1 Overview
>
> ##### 19.2 The `inofity` API
>
> ##### 19.3 inotify events
>
> ##### 19.4 Reading inotify Events
>
> ##### 19.5 Queue Limits and `/proc` Files
>
> ##### Limitations of `inotify`

1. `inotify` is the successor for `dnotify`, which provides a subset of the
   functionality of `inotify`. Both two techniques are Linux-specific.

   Those BSDs (including macOS) have a similar stuff, 
   [`kqueue`](https://www.freebsd.org/cgi/man.cgi?kqueue). 

   > This book gives some brief introduction to `dnotify` in `19.6`, if you are
   > intersted in this, take a look at it.

2. In Rust, we have [`inotify`](https://github.com/hannobraun/inotify-rs) for Linux
   and a cross platform crate [`notify`](https://github.com/notify-rs/notify).

##### 19.1 Overview & 19.2 The `inotify` API

3. Steps on how to use `inotify`:
  
   1. Initialize an `inotify` instance using `inotify_init(2)/intify_init1(2)`

      ```c
      #include <sys/inotify.h>

      int inotify_init(void);
      int inotify_init1(int flags); // since 2.6.27
      ```
      These two methods return an open file descriptor representing the `inotify`
      instance.

      `inotify_init1(2)` is a nonstandard syscall, it supports two extra flags 
      which can be bitwise ORed in `flags` argument. If `flags` is set to 0,
      then `inotify_init1(2)` is equivalent to `inotify_init(2)`.

      1. `IN_NONBLOCK`

         Set the O_NONBLOCK  file status flag on the open file description 
         (see `open(2)`) referred to by the new file descriptor.  Using this 
         flag saves extra calls to `fcntl(2)` to achieve the same result.

         > See
         > [Ch5: 14](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch5:file_i_o_further_details.md)
         > for more info about `O_NONBLOCKING`.

      2. `IN_CLOEXEC`

         Set the close-on-exec (`FD_CLOEXEC`) flag on the new file descriptor.  
         See the description of the `O_CLOEXEC` flag in `open(2)` for reasons 
         why this may be useful.

   2. Add (or modify) watch to the target file using `inotify_add_watch(2)`

      > The calling process needs to have `read` permission on the file.

      ```c
      int inotify_add_watch(int fd, const char *pathname, uint32_t mask);
      ```

      ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-29_20-43-58.jpg)

      The `fd` argument refers to the `inotify` instance, the target file is 
      specified in `pathname`. You can set events you wanna monitor in `mask`.

      `pathname` can be either a file or directory. When `pathname` is a 
      directory, the application will be notified about the events for the 
      directory itself and for files inside it.

      > NOTE that the monitoring is NOT recursive. 
      > 
      > if we 
      > `inotify_add_watch(fd, "./A", mask)`, then the events of `A` and `B`
      > will be monitored, but events of `file` will not.
      >
      > ```shell
      > .
      > ├── A
      > │   └── B
      > │       └── file
      > ```
      >

      This function returns a `watch descriptor`, which is used to represent
      the `watch` in later operations.

      > `inotify_rm_watch(2)` removes an existing `watch` (the return value
      > of `inotify_add_watch(2)` from a file).
      >
      > ```c
      > int inotify_rm_watch(int fd, int wd);
      > ```
      >
      > Removing a watch would trigger an `IN_IGNORED` event.
      >
      > When a watch is removed, it can occur for two reasons:
      >
      > 1. It is explicitly removed with `inotify_rm_watch(2)`
      > 2. The kernel removes it 
      >
      >    1. the `inotify instance` was dropped
      >    2. the watched file is deleted (receive `IN_DELETE_SELF` + `IN_IGNORD`)
      >    3. the file system on which the target file resides was unmounted. (
      >       receive `IN_UNMOUNT` + `IN_IGNORED`)
      >
      > > Before kernel 2.6.36, the drop of a watch which contains `IN_ONESHOT`
      > > will not trigger this `IN_IGNORED` event. Well, starting with 2.6.36,
      > > it does.

   3. Obtain event notification using `read(2)`

      To obtain notification, you need to call `read(2)` on the `inotify` file
      descriptor (return value of `inotify_init/inotify_init1`).

      Each successful `read(2)` will return one or more `inotify_event` structures
      in an **ordered** way. (See 19.4 for more information about this structure).

   4. finish the monitoring

      Once the monitoring is done, you can close the `inotify` file descriptor
      using `close(2)`.

##### 19.3 inotify events

4. Events that can be specified in the `mask` argument of `inotify_add_watch(2)`
   and returned from `read(2)`:

   > **When monitoring a directory**:
   >
   > * the events marked with an asterisk (*) can occur both for the 
   >   directory itself and for objects inside the directory
   >
   > * the events marked with a plus sign (+) occur only for objects inside 
   >   the directory **not for the directory itself**.

   * `IN_ACCESS (+)`: File was accessed (e.g., read(2), execve(2)).

     > I guess this is consistent with `atime`. 

   * `IN_ATTRIB (*)`: Metadata  changed—for example, permissions (e.g., `chmod(2)`), 
     timestamps (e.g., `utimensat(2)`), extended attributes (`setxattr(2)`), link count
     (since Linux 2.6.25; e.g., for the target of `link(2)` and for `unlink(2)`), and 
     user/group ID (e.g., `chown(2)`).

   * `IN_CLOSE_WRITE (+)`: File opened for writing was closed.

   * `IN_CLOSE_NOWRITE (*)`: File or directory not opened for writing was closed.

   * `IN_CREATE (+)`: File/directory created in watched directory (e.g., `open(3)`
     O_CREAT, `mkdir(2)`, `link(2)`, `symlink(2)`, `bind(2)` on a UNIX domain socket).

   * `IN_DELETE (+)`: File/directory **inside watched directory** is deleted.

     > This operation (the deletion itself) is performed on the file or directories
     > inside the watched directory, this is why it is marked with `+`. But the 
     > event is notified on the watched directory, you can not get this event on the
     > file on which the deletion opertaion is performed, this is kinda contradictory.

   * `IN_DELETE_SELF`: Watched file/directory **itself** was deleted. (This event also 
     occurs if an object is moved to another filesystem, since `mv(1)` in effect 
     copies the file  to  the other filesystem and then deletes it from the 
     original filesystem.)  **In addition, an `IN_IGNORED` event will subsequently be
     generated for the watch descriptor (the watched file is gone, then the watch 
     itself is also removed)**

     > You can see that `inotify` is implemented at the `inode` level.
     > `mv(1)/rename(2)` within the same file system (i.e., the `inode` remains
     > untouched) will not trigger this event.

   * `IN_MODIFY (+)`: File was modified (e.g., write(2), truncate(2)).

   * `IN_MOVE_SELF`: Watched file/directory was **itself** moved.

     > `rename(2)` within the same file system.

   * `IN_MOVED_FROM (+)`: **Generated for the directory** containing the old filename
     when a file is renamed.

   * `IN_MOVED_TO (+)`: **Generated for the directory containing** the new filename 
     when a file is renamed.

   * `IN_OPEN (*)`: File or directory was opened.

5. Events can be only used in `inotify_add_watch(2)`:

   * `IN_ALL_EVENTS`: macro is defined as a bit mask of all of events in `4`. 

   * `IN_MOVE`: Equates to `IN_MOVED_FROM | IN_MOVED_TO`

   * `IN_CLOSE`: Equates to `IN_CLOSE_WRITE | IN_CLOSE_NOWRITE`
   
   * `IN_DONT_FOLLOW (since Linux 2.6.15)`: Don't dereference pathname if it is
     a symbolic link.

     > This does not specify event, instead, it controls the behavior of 
     > `inotify_add_watch(2)`

   * `IN_EXCL_UNLINK (since Linux 2.6.36)`: 

     > This does not specify event, instead, it controls the behavior of 
     > `inotify_add_watch(2)`

     By default, when watching events on the children of a directory, events are 
     generated for children **even after they have been unlinked from the directory**.
     This can result in **large numbers of uninteresting events** for some applications
     (e.g., if watching `/tmp`, in which many applications create temporary files 
     *whose names are immediately unlinked*). 

     Specifying `IN_EXCL_UNLINK` changes the default behavior, so that events are
     not generated for children after they have been unlinked from the watched 
     directory.

   * `IN_MASK_ADD`: If a watch instance already exists for the filesystem object 
     corresponding to pathname, **add** (OR) the events in mask to the watch mask 
     (instead of **replacing** the mask); the error EINVAL results if `IN_MASK_CREATE`
     is also specified.

     > This does not specify event, instead, it controls the behavior of 
     > `inotify_add_watch(2)`

     > So by default, if you call `inotify_add_watch` to a file that has already 
     > been added, the original setting will be **overwritten**.

   * `IN_ONESHOT`: Monitor the filesystem object corresponding to pathname for **one
     event**, then remove from watch list.

     > This does not specify event, instead, it controls the behavior of 
     > `inotify_add_watch(2)`

   * `IN_ONLYDIR (since Linux 2.6.15)`: Watch  pathname only if it is a directory; 
     the error `ENOTDIR` results if pathname is not a directory. Using this flag 
     provides an application with a race-free way of ensuring that the monitored 
     object is a directory.

     > This does not specify event, instead, it controls the behavior of 
     > `inotify_add_watch(2)`

   * `IN_MASK_CREATE (since Linux 4.18)`: 
      
     > This does not specify event, instead, it controls the behavior of 
     > `inotify_add_watch(2)`

     Watch pathname only if it does not already have a watch associated with it; 
     the error `EEXIST` results if pathname is already being watched.

     Using this flag provides an application with a way of ensuring that new 
     watches do not modify existing ones. This is useful because multiple
     paths may refer to the same inode, and multiple calls to inotify_add_watch(2)
     without this flag may clobber existing watch masks.

6. Events that can only be returned from `read(2)`
  
   * `IN_IGNORED`: Watch was removed explicitly (`inotify_rm_watch(2)`) or automatically
     (file was deleted (`IN_DELETE_SELF` + `IN_IGNORED`), or filesystem was unmounted 
     (`IN_UNMOUNT` + `IN_IGNORED`)). 
     See also `inotify_rm_watch(2)` in note 3.

   * `IN_ISDIR`: Subject of this event is a directory.

   * `IN_Q_OVERFLOW`: Event queue overflowed (wd is -1 for this event).

   * `IN_UNMOUNT`: Filesystem containing watched object was unmounted.  **In 
     addition, an IN_IGNORED event will subsequently be generated for the watch
     descriptor.**

7. Few notes on `inotify`:
  
   1. `inotify` is inode based. When a file has multiple hard links, and you 
      monitor **this file** (**rather than its parent directory**) from one hard
      link, you will receive events when some modifications are made to other 
      hard links since they have the same inode.

   2. Events made through **symlink** will NOT be notified.

   3. When monitoring a directory, and receiving events on the files or directories 
      inside this watched directory, the `name` field of `struct inofity_event` 
      will be set to indicate the specific file.

   4. If we perform an `inotify_add_watch(2)` call on a file or directory that
      has already been watched, then the original watch setting will be **replaced**
      with the new one. Unless we have `IN_MASK_ADD` set in `mask`.

##### 19.4 Reding inotify Events

8. `struct inotify_event`

   ```c
   struct inotify_event {
       int      wd;       /* Watch descriptor */
       uint32_t mask;     /* Event set */

       // Unique cookie associating related events. Currently, this field is only
       // used for `rename(2)`. If this happens, an `IN_MOVED_FROM` event will be 
       // delivered to the directory from which the file moves and an `IN_MOVED_TO`
       // event is delivered to the directory to which the file moves, such two
       // events will have the same `cookie` field so that they can be associated.
       // In other cases, this field is simply set to 0.
       uint32_t cookie;   

       uint32_t len;      /* Size of name field, may include one or more tailing NULs */
       char     name __flexarr;   /* Optional null-terminated name */
   };
   ```

   [What is `__flexarr`:](https://stackoverflow.com/q/21589449/14092446)

   ```c
   /* Support for flexible arrays.
      Headers that should use flexible arrays only if they're "real"
      (e.g. only if they won't affect sizeof()) should test
      #if __glibc_c99_flexarr_available.  */
   #if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L && !defined __HP_cc
   # define __flexarr        []
   # define __glibc_c99_flexarr_available 1
   #elif __GNUC_PREREQ (2,97) || defined __clang__
   /* GCC 2.97 and clang support C99 flexible array members as an extension,
      even when in C89 mode or compiling C++ (any version).  */
   # define __flexarr        []
   # define __glibc_c99_flexarr_available 1
   #elif defined __GNUC__
   /* Pre-2.97 GCC did not support C99 flexible arrays but did have
      an equivalent extension with slightly different notation.  */
   # define __flexarr        [0]
   # define __glibc_c99_flexarr_available 1
   #else
   /* Some other non-C99 compiler.  Approximate with [1].  */
   # define __flexarr        [1]
   # define __glibc_c99_flexarr_available 0
   #endif
   ```

   Nevermind. The actual size of such a struct is `sizeof(struct inotify_struct)`
   + `len`.

   So if you `read(2)` on an `inotify instance fd`, the memory layout of your 
   buffer would be:

   ![memory layout](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-30_20-10-05.jpg)

9. Some notes on the return value of that `mask` field

   1. An `IN_IGNORED` event will be trigger if a watch is removed. (for more info
      , see `inotify_rm_watch(2)` in note `3`)

   2. If the subject of the event is a directory, then `IN_ISDIR` is included.

   3. If the file system containing the watched files is unmounted, then `IN_UNMOUNT`
      is included. And further, an `IN_IGNORED` event will be delivered since the
      kernel implicitly removes this watch (for more info, see `inotify_rm_watch(2)`
      in note `3`)

##### 19.5 Queue **Limits** and `/proc` Files

10. `inotify` needs kernel memory to queue messages, and thus has some limits.
    User with privilege can configure these limits through the files under
    `/proc/sys/fs/inotify`

    ```shell
    $ cd /proc/sys/fs/inotify/
    $ l
    Permissions Links Size User Group Date Modified Name
    .rw-r--r--@     1    0 root root  31 Oct 16:14  max_queued_events
    .rw-r--r--@     1    0 root root  31 Oct 16:14  max_user_instances
    .rw-r--r--@     1    0 root root  31 Oct 16:14  max_user_watches
    ```

    You can see that only `root` has the write permission.

    1. `max_queued_events`:

       When `inotify_init(2)/inotify_init1(2)` is called, this value is used to
       set an upper limit on the number of events that can be queued on the new
       inotify instance. If this limit is reached, then `IN_Q_OVERFLOW` is present,
       and excess events are discarded. The `wd` field for the overflow events 
       (i.e., events with `IN_Q_OVERFLOW` set) will be `-1`.

       ```shell
       $ cat max_queued_events
       16384
       ```

    2. `max_user_instances`

       Maximum number of `inotify instances` that can be created for a user. 

       ```shell
       $ cat max_user_instances
       128
       ```

    3. `max_user_watches`
       
       Maximum number of `inotify watch`es that can be created for a user. 

       ```shell
       $ cat max_user_watches
       119664
       ```

##### Limitations of `inotify`

11. Limitations of `inotify`

    > This section does not come from the book, it comes from `man 7 inotify`.

    1. The `struct inotify_event` does not contain any informatin about `who`
       triggers this event. 

    2. `inotify` only reports events triggered by `user space` processes through
       the file system APIs. So it is unusable on network file system.

    3. Even on local file system, it still can not be used on pseudo-filesystems 
       such  as  `/proc`, `/sys`, and `/dev/pts`.

    4. The inotify API does not report file accesses and modifications that may 
       occur because of mmap(2), msync(2), and munmap(2).

    5. The inotify API identifies affected files by filename. However, by the 
       time an application processes an inotify event, the filename may already
       have been deleted or renamed.

    6. The caller program needs to memorize the realtionship between a `watch 
       descriptor` and `watched file`.

    7. Monitoring for directories is not recursive (Honestly, I dont think this 
       is a flaw).

    8. You need to take care of that kernel memory limit.

    9. If a file system is mounted at your watched directory, then NO event will
       be sent. When it is unmounted, events will be generated.
