#### Ch17: Access Control Lists

> 1. concept: what is ACL
> 2. what does ACL look like
> 3. Minimal and extended ACL
>
>    Minimal ACL is implemented using the traditional permission set. Extended
>    ACL is implemented using `system.posix_acl_access` extended attributes.
>  
> 4. ACL permission checking algorithm.

1. What is ACL
  
   In Ch15 we learned the UNIX permission machanism, this is sufficient for most
   applications. However, some applications need more accurate control over the
   permission. This is where ACL comes to help.

   ACL can be seen as a extension to the traditional UNIX permission mechanism,
   it allows you to specify the permission per user and per group, for an arbitrary
   number of users and groups.

   > ACL on Btrfs/ext4 is enabled by default.

   > ACL is not standarized in POSIX. An attempt was made but failed. 

2. What does ACL look like

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-16_09-11-40.jpg)

   An ACL is a list of ACL entries, each of which specify the file permission for
   an individual user or group of users.

   Each entry consists of two or three parts:
   
   1. `Tag type`: specify the target to which this entry applies.

      * `ACL_USER_OBJ`: file owner. Each ACL contains **exactly one** `ACL_USER_OBJ`
        entry. This corresponds to the traditional `owner` permission. 

      * `ACL_USER`: means that this entry applies to a user whose UID is `Tag 
        qualifier`. An ACL can contain one or more `ACL_USER` entries, but only
        one `ACL_USER` can be defined for a particular user.

        > This is why ACL can set permission per user.

      * `ACL_GROUP_OBJ`: file group. Each ACL contains **exactly one** `ACL_GROUP_OBJ`
        entry. This corresponds to the traditional `group` permission **if `ACL_MASK`
        is not present**.

      * `ACL_GROUP`: Entry for a group whose GID is `Tag qualifier`.An ACL can 
        contain one or more `ACL_GROUP` entries, but only one `ACL_GROUP` can be 
        defined for a particular group.

        > This is why ACL can set permission per group.


      * `ACL_MASK`: Specify the maximum permission can be granted to `ACL_USER`,
        `ACL_GROUP_OBJ` and `ACL_GROUP` entries. An ACL contains at most one
        `ACL_MASK` entry (0 or 1). If `ACL_USER` or `ACL_GROUP` is present, then 
        an `ACL_MASK` is mandatory. 

        > Minimal ACL does not have `ACL_MASK` entry, extended ACL has.

      * `ACL_OTHER`: file other. Each ACL contains exactly one `ACL_OTHER` entry.
        This corresponds to the traditional `other` permission. 

      > If an ACL contains `ACL_USER` or `ACL_GROUP` entry, then it must contain
      > `ACL_MASK`, else, `ACL_MASK` is optional.


   2. Tag qualitifier (Required only for `ACL_USER` and `ACL_GROUP` entries)

      This field is simply UID or GID used for identifying a user or group.
   
   3. Permissions

      Permissions granted to this entry.

3. Minimal and Extended ACL

   * Minimal ACL: A minimal ACL is basically the traditional UNIX permission
     set. It has just three entries: `ACL_USER_OBJ/ACL_GROUP_OBJ/ACL_OTHER`.

     |Tag type     |Permissions         |
     |-------------|--------------------|
     |ACL_USER_OBJ |permission for owner|
     |ACL_GROUP_OBJ|permission for group|
     |ACL_OTHER    |permission for other|

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

   * Extended ACL: An extended ACL is one that additional contains `ACL_USER`,
     `ACL_GROUP` and `ACL_OTHER`.

     Another difference between `minimal ACL` and `extended ACL` is that minimal
     ACL is implemented unsing the traditional permission set. Extended ACL
     is implemented using `system.posix_acl_access` extended attributes (ch16).


4. ACL permission checking algorithm (from man 5 acl)

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


5. `ACL_MASK` will always be the upper limit of `ACL_USER` and `ACL_GROUP` entries.
   And possibily is the upper limit of `ACL_GEOUP_OBJ` entry.

