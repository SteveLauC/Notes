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

   1. seteuid(2) and setegid(2)
   2. If a executable has set-user-ID and set-group-ID set, then the EUID and EGID
      will has the same values as the UID and GID of that executable file.

      For example: the passwd command

      ```shell
      $ l $(which passwd)
      Permissions Size User Group Date Modified Name
      .rwsr-xr-x   60k root root  14 Mar 16:59   /usr/bin/passwd
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
