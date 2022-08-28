### Ch9: Process Credentials

* introduce the purpose of these crdentials
* syscalls used to retrieve and modify them

> $ [man 7 credentials](https://man7.org/linux/man-pages/man7/credentials.7.html)
> Seems this page lists all the IDs a process can have.

1. Each process has a set of associated numeric user identifiers (UIDs) and 
   group identifiers (GIDs). Sometimes, they are referred as `process credentials`

   * real user ID and group ID
   * effective user ID and group ID
   * saved set-user-ID and saved-group-ID
   * file-system user ID and group ID (Linux-specific)
   * supplementary group IDs

2. What are real UID and GID
   
   The real UID and GID identify the user and group to which a process belongs.

   1. A `log in` shell gets its real UID and GID from `/etc/passwd`.

      For example:
   
      ```shell
      // logging into a linux machine

      login: steve
      password: xxxxxxxxxx
      ```
   
      When I enter my user name and password, the shell reads `/etc/passwd` to
      find my UID and GID, and uses them as its real UID and GID
   
      ```
      steve:x:1000:1000:Steve:/home/steve:/usr/bin/zsh
      ```
      
      Then the real UID and GID of this log in shell are both 1000.

   2. When a new process is spawned, it inherents the real UID and GID from its
      parent.
   

3. What are EUID and EGID

   EUID and EGID, in conjunction with supplementary group IDs, are used to determine
   the permissions granted to a process when performing various operations (i.e. syscalls).

   A process with EUID 0 is a `priviledged process`.

   Normally, EUID and EGID have the same values as RUID and RGID. But this can
   be changed through:

   1. [seteuid(2) and setegid(2)](https://man7.org/linux/man-pages/man2/seteuid.2.html)
   2. If a executable has set-user-ID and set-group-ID set, then the EUID and EGID
      will have the same values as the UID and GID of that executable file.

      For example: the `passwd` command

      ```shell
      $ l $(which passwd)
      Permissions Size User Group Date Modified Name
      .rwsr-xr-x   60k root root  14 Mar 16:59   /usr/bin/passwd
      ```

      I (steve) can execute this command  as the permission for `other` is `r-x`.
      Then a process is spawned from this executable trying to modify `/etc/shadow`,
      and as the `set-UID` is set, EUID of this process is the owner of `/usr/bin/passwd`,
      which is just `root`. And *coincidenctally*, only `root` has the `w` permission
      of this file.

      ```shell
      $ l /etc/shadow
      Permissions Size User Group  Date Modified Name
      .rw-r-----  1.4k root shadow  6 Jun 19:18   /etc/shadow
      ```

4. What are set-user-ID and set-group-ID

   > set-user-ID can be abbreviated as set-UID and set-group-ID can be abbreviated
   > as set-GID.

   A process's EUID will be set to the UID of that executable file if the executable
   has set-user-ID set.
   And similar thing happens to set-group-ID

   A executable file with set-UID set can be named as set-UID program. If the
   set-GID is set, it can be named as set-GID program.

   Let's verify this:

   ```rust
   use nix::unistd::{geteuid, getegid};
    
   fn main() {
       print!("{} ", geteuid().as_raw());
       println!("{}", getegid().as_raw());
   }
   ```

   ```shell
   $ cargo b -q
   ❯ l target/debug/t
   Permissions Size User  Group Date Modified Name
   .rwxrwxr-x  6.1M steve steve 24 Aug 14:05   target/debug/t
   $ ./target/debug/t
   1000 1000

   $ sudo chown root target/debug/t
   $ sudo chmod u+s target/debug/t
   $ l target/debug/t
   Permissions Size User Group Date Modified Name
   .rwsrwxr-x  6.1M root steve 24 Aug 14:07   target/debug/t
   $ ./target/debug/t
   0 1000

   $ sudo chgrp root target/debug/t
   $ sudo chmod g+s target/debug/t
   $ l target/debug/t
   Permissions Size User Group Date Modified Name
   .rwxrwsr-x  6.1M root root  24 Aug 14:07   target/debug/t
   $ ./target/debug/t
   1000 0
   ```

   > set-UID (and set-GID) can be used to change the EUID of a process to someone
   > other than `root`. For example, to protect a resource, we can create a special
   > user (say `secret`), and set the permission of that resource to `rw-------`.
   > Then we have a executable `exe` 
   > 
   > ```shell
   > rw-------- secret secret resource
   > .r-s-----x secret secret exe
   > ```
   > so that all the people (belong to `other`) who wanna modify `resource` have 
   > to do this through `exe`. Since the `set-UID` of `exe` is set and the owner
   > of `exe` is `secret`, this can be perfectly done.

   > Sometimes, we use `set-UID-root` to distinguish the `set-UID program` whose
   > owner is `root`.

   > !NOTE: Linux **ignores** the set-UID and set-GID bits on all interpreted executables
   > [link](https://unix.stackexchange.com/a/2910/498440)

   executables with `set-UID` set:
   1. passwd
   2. mount
   3. unmount
   4. su

   executables with `set-GID` set:
   1. wall

5. What are `saved set-UID` and `saved set-GID`

   > Sometimes, `set-UID` and `set-GID` can refer to `saved set-UID` and 
   > `saved set-GID`.
  
   Saved set-UID and set-GID are designed for use with set-UID and set-GID 
   programs. If `set-UID` is not set, then the `saved-set-UID` has no effect.

   When an executable is executed, here is what happens:
   1. RUID (RGID) gets the value from the parent process. EUID (EGID) gets
      value from RUID (RGID).
   2. If the set-UID (set-GID) is set, then EUID (EGID) will have the value
      of the UID (GID) of that executable file.
   3. The value of saved set-UID (saved set-GID) copies from EUID (EGID). This
      operation happens **regardless of** whether the set-UID (set-GID) is set.
      
      > It seems this copy opreation only happens once when the program executed.
      > So EUID can differ from saved set-UID, for example, through `setuid/seteuid`
   
   Let's give an example:
   We have a set-UID-root program, user (UID: 1000) executes this binary, then
   the RUID, EUID and saved-set-UID will be:

   ```
   RUID EUID saved-set-UID
   1000  0   0
   ```

   `Saved-set-UID` is just like a **temporary variable** to store the `EUID`
   so that various syscalls can switch the `EUID` between `RUID` and 
   `saved-set-UID`. By doing so, the process can drop or regain the priviledge
   as they want.

6. filesystem user ID and group ID (Linux specific)

   On Linux, whether a file system operation (e.g. open a file/change permission/change 
   ownership) can be performed is determined by `file system user ID (and group ID)` 
   instead of `EUID` (and `EGID`).

   > Operations not related to file system relies on EUID (and EGID)

   Normally, the `file-system user ID and group ID` have the same value as EUID
   and EGID. And whenever the EUID (or EGID) is changed (via syscall or set-UID
   bit), the `file-system user ID (group ID)` will **also** be changed to that value.

   Because the above rule, Linux just behaves like other UNIX systems.

   **But** the `file-system user ID and group ID` **can** be explicitly changed
   via two Linux-specific syscalls:
   1. [setfsuid(2)](https://man7.org/linux/man-pages/man2/setfsuid.2.html)
   2. [setfsgid(2)](https://man7.org/linux/man-pages/man2/setfsgid.2.html)

   So in most cases, the `file-system user ID and group ID` will just have the
   same values as `EUID and EGID`, why does Linux need this. Well, this is a 
   historical problem (page 171). And nowadays, this problem has alredy been 
   resolved so there is no need to mention these two specific IDs.

7. Supplementary goup IDs

   This is a set of additional groups to which a process belongs. Login shell
   obtains its supplementary group IDs from `/etc/group` (check out 
   [Ch8](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch8.md):
   5 to see how to get all the groups a user belongs to), and a child process
   get this from its parent process just like `RUID` and `RGID`.

   This is used in conjunctino with `EUID (EGID)` and `file-system user ID 
   (file-system group ID)` to determine permissions for accessing various
   system resources.

   > This corresponds to the fact that a user can belong to muliple groups

8. syscalls (or lib functions) for retrieving and modifying these process 
   credentials

   > In addition to the syscalls described below, one can also get these info
   > through `/proc/$PID/status`
   > ```
   > Name: zsh
   > Umask: 0002
   > State: S (sleeping)
   > Tgid: 7376
   > Ngid: 0
   > Pid: 7376
   > PPid: 7375
   > TracerPid: 0
   > Uid: 1000 1000 1000 1000 # RUID EUID saved-set-UID filesystemUID
   > Gid: 1000 1000 1000 1000 # RGID EGID saved-set-GID filesystemGID
   > FDSize: 64
   > Groups: 4 27 121 999 1000 
   > NStgid: 7376
   > NSpid: 7376
   > NSpgid: 7376
   > NSsid: 7376
   > VmPeak:    22088 kB
   > VmSize:    21704 kB
   > VmLck:        0 kB
   > VmPin:        0 kB
   > VmHWM:    10128 kB
   > VmRSS:     9676 kB
   > RssAnon:     4496 kB
   > ```


   1. retrieve RUID and EUID

      ```c
      #include <unistd.h>
      #include <sys/types.h>
   
      uid_t getuid(void);  // retrieve RUID
      uid_t geteuid(void); // retrieve EUID
   
      gid_t getgid(void);  // retrieve RGID
      gid_t getegid(void); // retrieve EGID
      ```

   2. Modify **EUID**

      ```c
      # include <unistd.h>
   
      int setuid(uid_t uid);
      int setgid(gid_t gid);
      ```

      What effect these two syscalls have depends on whether the process is
      priviledged (priviledged means EUID==0, or more precisely, it has CAP_SETUID
      set):
      1. If unpriviledged, EUID is set to arg `uid`. And the `uid` is either 
         `RUID` or `saved set-UID` (you can set it to EUID, though meaningless).

         For a normal process, `RUID`, `EUID` and `saved set-UID` all have the same
         value. So this syscall is useful *only* in the case where this process is
         spawned from a `set-UID` program (so that RUID differs from EUID and saved
         set-UID).

      2. If priviledged and `uid` is not zero, `RUID/EUID/saved set-UID` are all
         set to `uid`. THIS IS A ONE-WAY TRIP since, once executed, the priviledge
         is dropped, and for a unpriviledged process, it can only set EUID to either
         RUID or saved set-UID, which are obviously not `0`. So no way back.

         > If you wanna a priviledged process to change just the `EUID`, use `seteuid(2)`
         > instead.

      > `setgid()` does a similar job and the above rules still apply except:
      > In rule 2, `RGID/EGID/saved set-GID` will be changed to `uid`. But EUID
      > remains unchanged so this process is **still** `priviledged`, so this 
      > is **NOT** a one-way trip.

      Example: a set-UID-root program drop its priviledge

      ```c
      // get current RUID
      uid_t cur_r_uid = getuid();

      setuid(cur_r_uid);
      ```
      
   3. Modify EUID and EGID (preferred way)
      
      ```c
      #include <unistd.h>

      int seteuid(uid_t euid);
      int setegid(gid_t egid);
      ```

      1. If unpriviledged, `EUID` can be set to either `RUID` or `saved set-UID`.
         Same as `setuid(2)`
      2. If priviledged, `EUID` can be set to any value. If this value is non-zero,
         then priviledge is dropped. But you can regain the priviledge through 
         `seteuid(2)` cause saved set-UID is still `0` so `0` is a valid argument
         for a unpriviledged process.

      ```rust
      use nix::unistd::{getresuid, seteuid, ResUid, Uid};
      
      fn main() {
          println!("This is a set-UID-root program");
          let mut res_uid: ResUid = getresuid().unwrap();
          println!(
              "Current RUID/EUID/saved set-UID {} {} {}",
              res_uid.real, res_uid.effective, res_uid.saved
          );
      
          println!("Drop priviledge");
          seteuid(res_uid.real).unwrap();
          res_uid = getresuid().unwrap();
          println!(
              "Current RUID/EUID/saved set-UID {} {} {}",
              res_uid.real, res_uid.effective, res_uid.saved
          );
      
          println!("Regain priviledge");
          seteuid(Uid::from(0)).unwrap();
          res_uid = getresuid().unwrap();
          println!(
              "Current RUID/EUID/saved set-UID {} {} {}",
              res_uid.real, res_uid.effective, res_uid.saved
          );
      }
      ```
      ```shell
      $ cargo b -q
      $ sudo chown root target/debug/rust
      $ sudo chmod u+s target/debug/rust
      $ ./target/debug/rust
      This is a set-UID-root program
      Current RUID/EUID/saved set-UID 1000 0 0
      Drop priviledge
      Current RUID/EUID/saved set-UID 1000 1000 0
      Regain priviledge
      Current RUID/EUID/saved set-UID 1000 0 0
      ```
