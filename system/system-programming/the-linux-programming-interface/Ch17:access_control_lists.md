#### Ch17: Access Control Lists

> 1. Concept: what is ACL
> 2. Accesss ACL and default ACL
> 2. What does ACL look like
> 3. Access minimal and extended ACL
>
>    Minimal access ACL is implemented using the traditional permission set.
>    Extended access ACL is implemented using `system.posix_acl_access` 
>    extended attributes.
>  
> 4. ACL permission checking algorithm.

1. What is ACL
  
   In Ch15 we learned the UNIX permission machanism, this is sufficient for most
   applications. However, some applications need more accurate control over the
   permission. This is where ACL comes to help.

   ACL can be seen as an extension to the traditional UNIX permission mechanism,
   it allows you to specify the permission **per user and per group**, for an arbitrary
   number of users and groups (Actually, there is a limit on how many entries are
   supported, since EA is limited.)

   > ACL on Btrfs/ext4 is enabled by default.

   > ACL is not standarized in POSIX. An attempt was made but failed. 

2. Categories of ACL

   There are two kind of ACL:

   1. Access ACL

      > Access ACL can be used on files and directories.

      Access ACL, as its name infers, is used for extending the traditional 
      permission mechanism.

      > Extended Access ACL is implemented using `sytem.posix_acl_access` EA.

   2. default ACL
      
      > Default ACL can be used only on directories.

      Default ACL does not determine the access of this directory, instead, it
      determines the ACL and permission of the file and direcories created in 
      it.

      * When a directory contains default ACL, then the sub directory created in 
        this directory will also contain default ACL (inheritance). 

      * The file created in this directory will inherit its parent dir's default
        ACL as its access ACL. And the ACL entries that are corresponding to the
        traditional permission bits are masked (ANDed) against the `mode` argument 
        of syscalls like `open(2)/mkdir(2)`. **In this case, argument `mode` is 
        seen as some kind of `mask`.**

        > Permission of Entries &= `mode`

        > ```c
        > int open(const char *pathname, int flags, mode_t mode);
        > int mkdir(const char *pathname, mode_t mode);
        > ```

        The ACL entries that are corresponding to the traditional permission bits are:

        1. `ACL_USER_OBJ`
        2. `ACL_MASK` is present (is extended ACL) ? `ACL_MASK` : `ACL_GROUP_OBJ`.

           > `ACL_MASK` is the new group in extended ACL.

        3. `ACL_OTHER`

      > Extended default ACL is implemented using `system.posix_ac_default` EA.


3. What does ACL look like

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-16_09-11-40.jpg)

   > Note that `group class entries`, these entries are limited by `ACL_MASK`, see
   > permission checking algorithm for more information.

   An ACL is a list of ACL entries (ACEs), each entry consists of two or three parts:
   
   1. `Tag type`: specify the target to which this entry applies.

      * `ACL_USER_OBJ`: file owner. Each ACL contains **exactly one** `ACL_USER_OBJ`
        entry. This corresponds to the traditional `owner` permission. 

      * `ACL_USER`: means that this entry applies to a user whose UID is `Tag 
        qualifier`. An ACL can contain one or more `ACL_USER` entries, but at most
        one `ACL_USER` can be defined for a particular user.

        > This is why ACL can set permission for a specific user.

      * `ACL_GROUP_OBJ`: file group. Each ACL contains **exactly one** `ACL_GROUP_OBJ`
        entry. This corresponds to the traditional `group` permission **if `ACL_MASK`
        is not present**.

        > If `ACL_MASK` is present, then `ACL_MASK` corresponds to the file group.

      * `ACL_GROUP`: Entry for a group whose GID is its `Tag qualifier`.An ACL can 
        contain one or more `ACL_GROUP` entries, but only at most `ACL_GROUP` can
        be defined for a particular group.

        > This is why ACL can set permission for a specific group.

      * `ACL_MASK`: Mask for `group class entries`.

        Specify the maximum permission that can be granted to `ACL_USER`,
        `ACL_GROUP_OBJ` and `ACL_GROUP` entries (group class entries). 

        > This does not mean that the permission of `ACL_MASK` is always bigger
        > than the permission of `group class entries`. We say the permission of
        > `ACL_MASK` is maximum because this is guaranteed by the permission
        > checking algorithm. (See note 6)

        An ACL contains at most one `ACL_MASK` entry (0 or 1). If `ACL_USER` or 
        `ACL_GROUP` is present, then an `ACL_MASK` is mandatory. 

        > Minimal ACL does not have `ACL_MASK` entry, extended ACL has.

      * `ACL_OTHER`: file other. Each ACL contains exactly one `ACL_OTHER` entry.
        This corresponds to the traditional `other` permission. 

   2. Tag qualifier (Optional, required only for `ACL_USER` and `ACL_GROUP` entries)

      This field is simply a UID or GID used for identifying a user or group.
   
   3. Permissions

      Permissions granted to this entry.

4. Minimal and Extended ACL

   > Both Access and Extended ACL have these stuff.

   * Minimal ACL: A minimal ACL is basically the traditional UNIX permission
     set. It has just three entries: `ACL_USER_OBJ/ACL_GROUP_OBJ/ACL_OTHER`.

     |Tag type     |Tag qualifier|Permissions         |
     |-------------|-------------|--------------------|
     |ACL_USER_OBJ |None         |permission for owner|
     |ACL_GROUP_OBJ|None         |permission for group|
     |ACL_OTHER    |None         |permission for other|

     > If you use `getfacl(1)` on a new file, you will find that it has a minimal
     > ACL.
     >
     > ```shell
     > $ touch file
     > $ getfacl file
     > # file: file
     > # owner: steve
     > # group: steve
     > user::rw-
     > group::r--
     > other::r--
     > ```

   * Extended ACL: An extended ACL is one that additional contains entries like 
     `ACL_USER`, `ACL_GROUP`, `ACL_OTHER` and `ACL_MASK`.

     Another difference between `minimal access ACL` and `extended access ACL` is 
     that minimal access ACL is implemented unsing the traditional permission set.
     Extended access ACL is implemented using `system.posix_acl_access` extended 
     attributes (ch16).

   > How to differenate between `minimal ACL` and `extended ACL`?
   >
   > `extended ACL` must have a `ACL_MASK` entry.

5. Group class entries.

   Group class entries consists 3 categories of ACL entries:

   1. `ACL_USER`
   2. `ACL_GROUP`
   3. `ACL_GROUP_OBJ`

   `ACL_MASK` limits the maximum permission that can be granted to `group cleas
   entries`.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-02_20-54-58.jpg)

   When a file has `extended ACL`, then setting (chmod) or getting (stat) `group
   permission` are actually maniulating the permission of `ACL_MASK` entry 
   instead of `ACL_GROUP_OBJ` for the reason that `group permission` is no
   longer storing the permission of `ACL_GROUP_OBJ`, rather storing `ACL_MASk`.

   But when the file group are accessing this file, permission checking are
   performed using `ACL_GROUP_OBJ & ACL_MASK`. See permission checking algorithm.


   ```shell
   $ touch file
   $ setfacl -m g:docker:rw file
   $ l file
   .rw-rw-r--@     1    0 steve steve 18 Oct 12:07  file
   $ getfacl file
   # file: file
   # owner: steve
   # group: steve
   user::rw-
   group::r--
   group:docker:rw-
   mask::rw-
   other::r--
   ```

   ```rust
   use std::{fs::metadata, os::unix::fs::PermissionsExt};
   
   fn main() {
       let md = metadata("file")
       .unwrap();
   
       println!("{:16b}", md.permissions().mode())
   }
   ```
   ```shell
   $ cargo r -q
   1000000110110100 # 110 (rw-) the permission of mask.
   ```


6. ACL permission checking algorithm (from man 5 acl)

   > Checks are performed in the following order, until one of the critera is
   > matched.

   1. If the process is privileged, then `r` + `w` are granted. `x` is granted
      only if `x` is granted in **at least one** of ACL entries.

      > This is similar to the traditional UNIX permission check.


   2. If the effective user ID of the process matches the user ID of
      the file object owner, then

      1. if the ACL_USER_OBJ entry contains the requested permissions, access is 
      granted,

      2. else access is denied.

   3. else if the effective user ID of the process matches the qualifier of any 
      entry of type ACL_USER, then

      1. if the matching ACL_USER entry and the ACL_MASK entry contain the 
         requested permissions, access is granted,

      2. else access is denied.

   4. else if the effective group ID or any of the supplementary group IDs of 
      the process match the file group or the qualifier of any entry of type 
      ACL_GROUP, then

      > Several entries may be matched in step 4, the entry that satisfies the 
      > request will be used. (NOTE that permission of several entries will not
      > be accumulated.)
      >
      > Example:
      >
      > If you are in two groups, the first one has the `r` permission, the second
      > one has the `w` permission, then `access(file, R_OK)` and `access(file, W_OK)`
      > both will return 0, but `access(file, R_OK|W_OK)` will return -1.

      1. if the ACL contains an ACL_MASK entry, then

         1. if the ACL_MASK entry and any of the matching ACL_GROUP_OBJ or 
            ACL_GROUP entries contain the requested permissions, access is 
            granted,

         2. else access is denied.

      2. else (note that there can be no ACL_GROUP entries without an ACL_MASK 
         entry)

         1. if the ACL_GROUP_OBJ entry contains the requested permissions, access 
            is granted,

         2. else access is denied.

   5. else if the ACL_OTHER entry contains the requested permissions, access is 
      granted.

   6.   else access is denied.

6. Long and short text form of ACE

   1. `long text form ACLs` contains one ACL entry per line, each field of 
      `ACL_USER_OBJ`, `ACL_GROUP_OBJ` `ACL_MASK` and `ACL_OTHER` is separated
      by `::`, Otherwise, it is separated by `:`. Each entry May contain comment,
      which is started by `#`. `getfacl(1)` displays ACLs in this form. 

      ```shell
      $ getfacl file
      # file: file
      # owner: steve
      # group: steve
      user::rw-
      group::r--
      group:docker:rw-
      mask::rw-
      other::r--
      ```

   2. `short text form ACL` consists a sequence of ACL entries speparated by 
      commas. Each field of `ACL_USER_OBJ`, `ACL_GROUP_OBJ` `ACL_MASK` and 
      `ACL_OTHER` is separated by `::`, Otherwise, it is separated by `:`. 

      ```
      u::rw,u:paulh:rw,u:annabel:rw,g::r,g:teach:rw,m::rwx,o::-
      ```

7. Why do we need `ACL_MASK` entry?

   TL;DR: to be compatible with the traditional UNIX permission mechanism.

   Say we have a file, and this file has a ACL like:

   ```
   user::rwx
   user:paulh:r-x
   group::r-x
   group:teach:--x
   other::--x
   ```

   And we call `chmod(file, 0700)` on this file, which means deny all acccess
   to everyone who is not the file owner. How can we implement this?

   1. set the permission of `ACL_GROUP_OBJ` and `ACLOTHER` to `000`?
      
      but `user:paulh` and `group:teach` still have their extra permission.

   2. set the permission of `ACL_USER/ACL_GROUP_OBJ/ACL_GROUP/ACL_OTHER` all to
      `000`? This overwrote the setting for `user:pault` and `group:teach`. 
      Calling `chmod(file, 0751)` won't store the original setting.

   We need to "remove" the permissions set for `paulh` and `teach` without
   overwriting those two entries.

   `ACL_MASK` entry is derised to solve this problem. Changing the group permission
   now alter the permission of `ACL_MASK` without overwriting the setting for
   `user:paulh` and `group:teach`. With `ACL_MASK`, we can simply set the permission
   of `ACL_MASK` and `ACL_OTHER` to `000`.

   > `group class entries` are seen as the new group.

8. Using `getfacl(1)` and `setfacl(1)` to retrieve or modify ACLs

   Note that `setfacl(1)` will automatically adjust `ACL_MASK` to be the union
   of `group class entries`, to disable this, use the `-n` option.


9. behavior of GNU `ls` when encountering ACL
  
   `ls(1)` will print a `+` when a directory has `default ACL` or `extended access ACL`
   or when a file has `extended access ACL`.

10. ACL APIs

    ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-18_11-00-53.jpg)

    NOTE that all these path-APIs will dereference symbolic link, since the
    symbolic link has no ACL (on Linux).

    1. Retrieve a file's ACL
      
       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       acl_t acl_get_file(const char *path_p, acl_type_t type);
       ```

       Argument `type` : 

       * To retrieve `Access ACL`, pass `ACL_TYPE_ACCESS` to `type`

       * To retrieve `Default ACL`, set `type` to `ACL_TYPE_DEFAULT`

       This function return `acl_t`

       > Use `acl_free(3)` to deallocate the `acl_t` retunred by this function.

    2. Retrieve an ACE from an ACL

       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       int acl_get_entry(acl_t acl, int entry_id, acl_entry_t *entry_p);
       ```

       To iterate over all the ACL entries, we should set `entry_id` as 
       `ACL_FIRST_ENTRY` first, then set it to `ACL_NEXT_ENTRY` in the following
       calls.

    3. Retrieve or modify `Tag Type` in an entry

       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       int acl_get_tag_type(acl_entry_t entry_d, acl_tag_t *tag_type_p);
       int acl_set_tag_type(acl_entry_t entry_d, acl_tag_t tag_type);
       ```

       `acl_tag_t` is a integer type, it can be:

       1. ACL_USER_OBJ
       2. ACL_USER
       3. ACL_GROUP_OBJ
       4. ACL_GROUP
       5. ACL_MASK
       6. ACL_OTHER

    4. Retrieve or modify `qualifier` in an entry
       
       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       void * acl_get_qualifier(acl_entry_t entry_d);
       int acl_set_qualifier(acl_entry_t entry_d, const void *qualifier_p);
       ```

       This function return a pointer to `uid_t` or `gid_t` (u32 on amd64 Linux).

       And this will only succeed when this entry is of type `ACL_USER` or 
       `ACL_GROUP`.

    5. Retrieve or modify `Permission set` in an entry

       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       int acl_get_permset(acl_entry_t entry_d, acl_permset_t *permset_p);

       // Handy functions to avoid manually `&` or `|`
       // `perm` argument can be: ACL_READ, ACL_WRITE, ACL_EXECUTE
       int acl_add_perm(acl_permset_t permset_d, acl_perm_t perm);
       int acl_delete_perm(acl_permset_t permset_d, acl_perm_t perm);
       int acl_clear_perms(acl_permset_t permset_d);

       int acl_set_permset(acl_entry_t entry_d, acl_permset_t permset_d);
       ```

    6. Create or delete an ACE

       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       // add entry `entry_p` to `acl_p`
       int acl_create_entry(acl_t *acl_p, acl_entry_t *entry_p);

       // delete `entry_d` from `acl`
       int acl_delete_entry(acl_t acl, acl_entry_t entry_d);
       ```

    7. Update a file's ACL

       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       // converse of `acl_get_file(3)`
       int acl_set_file(const char *path_p, acl_type_t type, acl_t acl);
       ```

    8. Create an ACL from text
      
       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       acl_t acl_from_text(const char *buf_p);
       ```

       `buf_p` is a string containing long or short form of ACL.

       > Use `acl_free(3)` to deallocate `acl_t` returned by this function.

    9. Convert an ACL to text

       ```c
       #include <sys/types.h>
       #include <sys/acl.h>

       char * acl_to_text(acl_t acl, ssize_t *len_p);
       ```

       The string returned by this function is allocated in the heap, caller should
       manually deallocate it using `int acl_free(void *obj_p)(3)`.

    10. Recalculate the permission of `ACL_MASK` entry

        ```c
        #include <sys/types.h>
        #include <sys/acl.h>

        int acl_calc_mask(acl_t *acl_p);
        ```

        The permission of `ACL_MASK` will be the union of the permission sets of
        `group class entries`.

        This  function will **create** an `ACL_MASK` entry if it does not exist.

    11. Check if an ACL is valid

        ```c
        #include <sys/types.h>
        #include <sys/acl.h>

        int acl_valid(acl_t acl);

        // A better `acl_valid` but is specific to Linux
        int acl_check(acl_t acl, int *last);
        // Convert the error code returned by `acl_check` to a printable string.
        // Also Linux specific
        const char * acl_error(int code);
        ```

    12. Remove the default ACL
        
        ```c
        #include <sys/types.h>
        #include <sys/acl.h>

        int acl_delete_def_file(const char *path_p);
        ```

    13. Allocate a space that is capable of accommodating `count` entries

        ```c
        #include <sys/types.h>
        #include <sys/acl.h>

        acl_t acl_init(int count);
        ```

        > Use `acl_free(3)` to deallocate the storage.

    14. Duplicate an existing acl
        
        ```c
        #include <sys/types.h>
        #include <sys/acl.h>

        acl_t acl_dup(acl_t acl);
        ```

        This function will allocate memory, use `acl_free(3)` to deallocate it.
