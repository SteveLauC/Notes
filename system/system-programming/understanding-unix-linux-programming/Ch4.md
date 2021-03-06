1. UNIX系统fs的3部分: (通用的特性) 
   1. 超级块: 存放文件系统本身的信息
   2. i节点表: 用来存放文件的属性
   3. 数据区: 存放文件内容
 
2. 新建文件的过程:
   内核首先在i节点表中寻找空的i-node，找到后则去数据区寻找空闲的块存放数据，然后
   将数据写入数据块，再将所占用的数据块的编号存储到i-node中，最后在目录中添加i-
   node和文件名的映射。

3. 目录，不同UNIX的目录的内部标识不同(所以读目录需要使用专用的系统调用`readdir`
   而不是`read`)，但目录提供的抽象功能是一样的，即i-node号到文件名的映射。
 

4. 使用`ln orig.file link.file`创建的hard link文件和原文件具有相同的i-node号。
   所以从文件系统的角度来看hard link，原来的文件在磁盘上只是一份，i-node和数据块
   都是那一块，仅仅是在`目录`这个文件中加了一项

   link前
   |i-node|file name|
   |------|---------|
   |99    |orig.file|

   link后
   |i-node|file name|
   |------|---------|
   |99    |orig.file|
   |99    |link.file|

5. root目录的特殊性，在`/`下，使用`ls -ia`才查看当前目录和父目录的inode号，会发现
   两者都是2.

   pwd命令就靠这个特性写

6. 读取文件内容的过程: 
   我们需要文件名作为参数传给`open`函数，然后`open`函数打开目录，寻找此文件的in
   ode号，然后在inode节点中寻找数据块的编号极其顺序，然后去数据块中读取内容。

   > 试想一下读取没有`r`权限的文件，在inode的元数据中`open`函数发现没有读取权限，
   则其直接返回`-1`，并将`errno`设置为相应的值。


7. 目录包含一个文件，就是目录文件中有一个文件项，项里是文件名和inode节点。由于i
   node编号像一个指针一样，目录包含的是对文件的引用。
 
8. 链接数，目录是引用，也就是链接，那么一个文件的链接数就是其在目录树中这个项出现
   的次数。


9. hard link 和 symbolic link

   A hardlink isn't a pointer to a file, it's a directory entry (a file) pointing 
   to the same inode. Even if you change the name of the other file, a hardlink 
   still points to the file. If you replace the other file with a new version (by 
   copying it), a hardlink will not point to the new file. You can only have 
   hardlinks within the same filesystem. With hardlinks you don't have concept of 
   the original files and links, all are equal (think of it as a reference to an 
   object). It's a very low level concept.

   On the other hand, a symlink is actually pointing to another path (a file name); 
   it resolves the name of the file each time you access it through the symlink. 
   If you move the file, the symlink will not follow. If you replace the file with 
   another one, keeping the name, the symlink will point to the new file. Symlinks 
   can span filesystems. With symlinks you have very clear distinction between the 
   actual file and symlink, which stores no info beside the path about the file 
   it points to.
   
   硬链接是目录表项，是inode级别的链接；而symbolic link则只是文件名层面的链接，
   如果原文件的文件名改了，slink也就失效了。
   
   如回答所说，inode是不可以跨文件系统的，不同的文件系统的inode是不同编址的。

   [link](https://askubuntu.com/questions/108771/what-is-the-difference-between-a-hard-link-and-a-symbolic-link/43599#43599)
   这个问题的回答中的讲解的图片蛮好的。

10. 删除文件`rm`使用的系统调用是`unlink`，本身就存在`(1)unlink`用来删除文件

    ```c
    int unlink(const char *pathname);
    ```

11. `mv`命令使用的系统调用是`rename`
    
    ```c
    int rename(const char *oldpath, const char *newpath);
    ```

    它的工作流程是: 
    1. 在新的目录中添加新entry
    2. 在旧的目录中删除entry

    > 在以前是没有`rename`这个系统调用的，有的仅是`unlink`和`link`，先`link`，也
    就是在新的目录中添加一项，然后再`unlink`删去旧目录项。

    ```c
    if (link("old", "new") != -1) {
        unlink("old");
    }
    ```

12. 每一个进程都有一个当前工作路径，但进程里保存的不是文件名，而是inode号

13. 使用`readdir`和`stat`在mount point拿到的inode不一致，这是因为`readdir`是遍历
    目录的，拿到的信息并不全面，而`stat`拿到的信息更加全面。在mount point上前者拿
    到的是总的文件系统上的inode编号，而后者则是mount上去的新文件系统上的inode编号。
    
    使用`ls -ali`可以正确地辨别出mount point，拿到另一个fs的inode号。我们在前面自
    己写的`ls`仅使用了`readdir`，在这里应该是没那么合理的。


14. UNIX的文件系统树:
    每个分区都有自己的文件系统树，有一个是主分区，主分区的文件树的根是真的整个文件
    的根。而其他分区，需要将其根挂到主分区树的叶子上，这就是mount操作。

15. 可以使用`mount`命令查看目前系统中有哪些挂载点

16. 同一个inode编号，可以出现在不同的文件系统上，也就是说inode号无法标识一个具体的
    文件。设备号+inode可以唯一地标识具体的文件。

    正是由于这个特性，`link`命令拒绝用来创建跨越文件系统的硬链接。

    > 在`(2)link`的手册中可以看到这样的错误信息，link是ln命令的内部系统调用
    EXDEV  oldpath and newpath are not on the same mounted filesystem.  
    (Linux permits a filesystem to be mounted at multiple points, but link() does 
    not work across different mount  points, even if the same filesystem is mounted 
    on both.) 

    硬链接是靠inode实现的，不可以跨文件系统，而软链接则是靠文件名链接起来的，是可以跨
    文件系统的。
