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


   > [uulp ch4 note 4](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch2.md)

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
