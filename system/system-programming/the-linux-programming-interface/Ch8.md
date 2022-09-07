#### Ch8: Users and Groups

> 1. Purpose of introducing Users and Groups
> 2. `/etc/passwd` and `/etc/group` databases
> 3. functions for retrieving user and group infos
> 4. `/etc/shadow` how is password stored (crypt) on Linux 
> 5. How does the verification happen when you are asked to input your password

1. the primary purpose of user and group IDs is to determine ownership of 
various system resourses and to control the permissions granted to processes
accessing those resourses.

2. each file has a user and group ID and each process has a number of user and
group IDs(real, effective, saved)

3. `/etc/passwd` file format, each line contains 7 fields separated by colon.

   In order, these fields are:

   1. Login name: user must enter this name to login. (can be seen as a 
   human-readable name for UID)

   2. Encrypted password: If `shadow password` is enabled, then this field is 
   ignored and has a literal value of `x` in most cases. If this field
   is emtpty, then no password is needed to log this account in.

   3. UID: if this field is 0, then this account has superuser privileges. On 
   Linux (after kernel 2.4), uid is stored using 32 bits, `u32` on Linux.

      > It is possible to have more than one accounts with the same UID.

   4. GID: The ID of the first group that this user is in.

   5. Comment: Extra information about the user. 
      
      > The book says this field is used by `finger(1)`, though I didn't find it.

   6. Home directory: $HOME

   7. Login shell: $SHELL. If this field is empty, then the login shell defaults
   to `/bin/sh`.

   > On a stand-alone system, all the password information resides in the file
   > `/etc/passwd`. However, if we are using a system such as `Network Information
   > System (NIS) or Lightweight Directory Access Protocol (LDAP)` to distribute
   > passwords in a network environment, part or all of this information resides
   > on a remote system.


   > [uulp ch2 note 4](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch2.md)

4. The shadow password file: `/etc/shadow`

   Initially, all the password info is stored in `/etc/passwd`, this presented a
   security problem. The shadow password file `/etc/shadow` is designed to address
   this problem. And the basic idea is that all the non-sensitive user info resides
   in the public readable password file, while the encrypted passwords are 
   maintained in the shadow file which is readable only by the superuser and 
   priviledged programs(EUID)

   ```shell
   $ l shadow
   Permissions Size User Group  Date Modified Name
   .rw-r-----  1.4k root shadow  6 Jun 19:18   shadow
   ```

   ```shell
   $ sudo cat /etc/shadow |head -5
   root:*:19042:0:99999:7:::
   daemon:*:19042:0:99999:7:::
   bin:*:19042:0:99999:7:::
   sys:*:19042:0:99999:7:::
   sync:*:19042:0:99999:7:::
   ```

   We will talk about the field of this file in detail in Section 8.5.

   > This is not a standard file.

5. The group file: `/etc/group`

   One line for each group, each line contains 4 colon-separated fields:

   1. Group name

   2. Encrypted password: With the advent of multiple group memberships, group
   password is rarely used. If `password shadowing` is enabled, then it is 
   encrypted and stored in `/etc/shadow`, and any string literal (Mostly `x`, 
   including empty string) will be left on this field.

      > If this field is `empty`, it does not mean that this group does not have
      > password. This is different from the password field of `/etc/passwd`
      
      ```shell
      $ l gshadow
      Permissions Size User Group  Date Modified Name
      .rw-r-----   836 root shadow 14 Jun 13:24   gshadow
      ```

   3. Group ID: u32 (after kernel 2.4)

   4. User list (separated by comma)

   ```shell
   cat /etc/group
   root:x:0:
   daemon:x:1:
   bin:x:2:
   sys:x:3:
   adm:x:4:syslog,steve
   ```

   Find the groups to which steve belongs:
   If we simply grep `steve` in `/etc/group`, according to the field definition,
   he belongs to `admin/sudo/lpadmin/docker`. But `steve` is definitely included
   in group `group`...

   ```shell
   $ cat group|grep steve
   adm:x:4:syslog,steve
   sudo:x:27:steve
   lpadmin:x:121:steve
   steve:x:1000:         # does not list `steve`..
   docker:x:999:steve
   ```

   So the correct way to do this is: (what a stupid method...)

   1. check out the group id of steve
      
      ```shell
      $ awk 'BEGIN {FS = ":" }; { if ($1 == "steve") print $3}' /etc/passwd
      1000
      ```

   2. Then print the group name of that gid
      
      ```shell
      $ awk 'BEGIN {FS = ":" }; { if ($3 == "1000") print $1}' /etc/group 
      steve
      ``` 
   3. Then other groups
      
      ```shell
      $ cat /etc/group|grep steve|awk 'BEGIN {FS=":"}; {if ($1 != "steve") print $1}'
      adm
      sudo
      lpadmin
      docker
      ```
6. retrieve user and group information

   ```c
   struct passwd* getpwnam(const char* name);
   struct passwd* getpwuid(uid_t uid);

   struct group *getgrnam(const char *name);
   struct group *getgrgid(gid_t gid);
   ```

   ```rust
   // impl User
   pub fn from_name(name: &str) -> Result<Option<Self>>
   pub fn from_uid(uid: Uid) -> Result<Option<Self>>

   // impl Group
   pub fn from_gid(gid: Gid) -> Result<Option<Self>>
   pub fn from_name(name: &str) -> Result<Option<Self>>
   ```

   ```rust
   #[repr(C)]
   pub struct passwd {
       pub pw_name: *mut c_char,
       pub pw_passwd: *mut c_char,
       pub pw_uid: uid_t,
       pub pw_gid: gid_t,
       pub pw_gecos: *mut c_char,
       pub pw_dir: *mut c_char,
       pub pw_shell: *mut c_char,
   }

   #[repr(C)]
   pub struct group {
       pub gr_name: *mut c_char,
       pub gr_passwd: *mut c_char,
       pub gr_gid: gid_t,
       pub gr_mem: *mut *mut c_char,
   }

   pub struct User {
       pub name: String,
       pub passwd: CString,
       pub uid: Uid,
       pub gid: Gid,
       pub gecos: CString,
       pub dir: PathBuf,
       pub shell: PathBuf,
   }
   pub struct Group {
       pub name: String,
       pub passwd: CString,
       pub gid: Gid,
       pub mem: Vec<String>,
   }
   ```

   > The weird name of `pw_gecos` comes from an eraly UNIX implementation, P157.

   NOTE: The structure returned by these functins are statically allocated. And
   the memory pointed by fields like `pw_name`/`pw_passwd`... are also static.
   


7. use `getpwnam_r`, `getpwuid_r`, `getgrnam_r` and `getgruid_r` instead
   
   ```c
   int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen,
                  struct passwd **result);

   int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, 
                  struct passwd **result);


   int getgrnam_r(const char *name, struct group *grp,
                  char *buf, size_t buflen, struct group **result);

   int getgrgid_r(gid_t gid, struct group *grp,
                  char *buf, size_t buflen, struct group **result);
   ```

   These functions' arguments contain a `passwd/group` structure, and a buffer to store
   the `string` fields.

   The recommended additional buffer size can be retrieved through 
   `sysconf(_SC_GETPW_R_SIZE_MAX)/sysconf(_SC_GETGR_R_SIZE_MAX)`.

   ```c
   // Usage demo
   #include <stdio.h>
   #include <stdlib.h>
   #include <unistd.h>
   #include <pwd.h>
   
   int main(void)
   {
           long buf_size = sysconf(_SC_GETPW_R_SIZE_MAX);
           if (buf_size == -1) {
                   exit(EXIT_FAILURE);
           }
           struct passwd buf;
           char string_buf[buf_size];
           struct passwd *result = NULL;
   
           // On success: return 0, result is not NULL
           // Not found: return 0, result is NULL
           // On error: return errno, result is NULL
           int res = getpwnam_r("steve", &buf, string_buf, buf_size, &result);
   
           if (res == 0 && result != NULL) {
                   printf("Found: %s\n", buf.pw_name);
           } else if (res == 0 && result == NULL) {
                   printf("User not found\n");
           } else {
                   fprintf(stderr, "Error occured\n");
           }
   
           exit(EXIT_SUCCESS);
   }
   ```

   ```rust
   // This is the function behind `User::from_name`
   // and `User::from_uid`

   // link: https://github.com/nix-rust/nix/blob/master/src/unistd.rs

   fn from_anything<F>(f: F) -> Result<Option<Self>>
   where
       F: Fn(*mut libc::passwd,
             *mut c_char,
             libc::size_t,
             *mut *mut libc::passwd) -> libc::c_int
   {
       let buflimit = 1048576;

       // retrieve the buffer size
       let bufsize = match sysconf(SysconfVar::GETPW_R_SIZE_MAX) {
           Ok(Some(n)) => n as usize,
           Ok(None) | Err(_) => 16384,
       };

       let mut cbuf = Vec::with_capacity(bufsize);
       let mut pwd = mem::MaybeUninit::<libc::passwd>::uninit();
       let mut res = ptr::null_mut();

       loop {
           let error = f(pwd.as_mut_ptr(), cbuf.as_mut_ptr(), cbuf.capacity(), &mut res);
           if error == 0 {
               if res.is_null() {
                   return Ok(None);
               } else {
                   let pwd = unsafe { pwd.assume_init() };
                   return Ok(Some(User::from(&pwd)));
               }
           } else if Errno::last() == Errno::ERANGE {
               // Trigger the internal buffer resizing logic.
               reserve_double_buffer_size(&mut cbuf, buflimit)?;
           } else {
               return Err(Errno::last());
           }
       }
   }
   ```

8. sequentially scan `/etc/passwd` and `/etc/group`

   ```c
   #include <sys/types.h>
   #include <pwd.h>
   
   // continueously return a pointer pointing to a user entry
   // from `/etc/passwd`, NIS and LDAP
   // returns NULL when there are no more records (or an error occurs)
   struct passwd *getpwent(void);

   // rewind the pointer to the start 
   void setpwent(void);
   
   // close file when `getpwent` is finished
   void endpwent(void);

   The return value may point to a static area, and may be **overwritten** by 
   subsequent calls to getpwent(), getpwnam(3), or getpwuid(3).  (Do not  
   pass  the  returned pointer to free(3).)

   And endpwent() is necessary so that any subsequent getpwent() (in other parts
   of our program or in the library we used in current program) will reopen the 
   file and read from the beginning.
   ```

   Note: If you wanna read `/etc/passwd` twice, do not do this:

   ```c
   getpwent(); // first read
   setpwent();

   // update `/etc/pwsswd`

   getpwent(); // second read
   endpwent();
   ```

   The second call  of `getpwent()` won't touch `/etc/passwd` again as it already
   did that. So if you update `/etc/passwd`, the changes will not be reflected on
   you second `getpwent()`. [More info](https://stackoverflow.com/q/60058907/14092446)

   You should use `endpwent()` instead of `setpwent()`

   ```c
   getpwent(); // first read
   endpwent();

   // update `/etc/pwsswd`

   getpwent(); // second read
   endpwent();
   ```

   ```c
   // print all user names

   #include <stdio.h>
   #include <stdlib.h>
   #include <pwd.h>
   
   int main(void)
   {
           struct passwd *buf;
           while ((buf = getpwent()) != NULL) {
                   printf("%s\n", buf->pw_name);
           }
           endpwent();
           exit(EXIT_SUCCESS);
   }
   ```


   ```c
   // These functions are analogous except they are for group file

   #include <sys/types.h>
   #include <grp.h>
   
   struct group *getgrent(void);
   void setgrent(void);
   void endgrent(void);
   ``` 


   ```c
   #include <pwd.h>

   int getpwent_r(struct passwd *pwbuf, char *buf, size_t buflen, 
                  struct passwd **pwbufp);
   ```

9. retrieve records from the shadow password file

   ```c
   #include <shadow.h>

   // Returns pointer on success, or NULL on not found or error
   struct spwd *getspnam(const char* name);

   strcut spwd *getspent(void);
   void setspent(void);
   void endspent(void);
   ```

   ```c
   #[repr(C)]
   pub struct spwd {
       pub sp_namp: *mut c_char, // Login name (username)
       pub sp_pwdp: *mut c_char, // Encrypted password

       // Remaining fields support `password aging`, an optional
       // feature that forces users to regularly change their passwords
       pub sp_lstchg: c_long,
       pub sp_min: c_long,
       pub sp_max: c_long,
       pub sp_warn: c_long,
       pub sp_inact: c_long,
       pub sp_expire: c_long,
       pub sp_flag: c_ulong,
   }
   ```

10. Password encryption

    UNIX uses a `one-way` encryption algorithm, which means there is no way of
    recreating the original password from its encrypted foam. The only way of
    validating a candidate password is to encrypt it and see if the encrypted
    result matches the value stored in `/etc/shadow`.

    ```c
    // Functions used to encrypt passwords

    #include <crypt.h>
    
    char * crypt(const char *phrase, const char *setting);
    char * crypt_r(const char *phrase, const char *setting, struct crypt_data *data);
    char * crypt_rn(const char *phrase, const char *setting, struct crypt_data *data, int size);
    char * crypt_ra(const char *phrase, const char *setting, void **data, int *size);
    ```

    > On other OSes, these functions may be marked as deprecated.

11. use `getpass` to prompt user to input a password (obsolete)
   
    ```c
    #include <unistd.h>

    char *getpass(const char *prompt);
    ```

    > The  getpass()  function opens /dev/tty (the controlling terminal of
    > the process), outputs the string prompt, turns off echoing, ignore the
    > signal of `SIGINT`, reads one line (the "password"), restores the 
    > terminal state and closes /dev/tty again.

    > This function is **obsolete**.  Do not use it. Consider writing your own
    > one.

12. retrieve maximum login username length

    ```c
    long max_len = sysconf(_SC_LOGIN_NAME_MAX);	
    ```

13. Most Linux distros use `yescrypt` as their default shadow hasing method.

    ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-08-23%2008-45-20.png)
