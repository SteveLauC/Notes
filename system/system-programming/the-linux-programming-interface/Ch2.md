1. Linux的内核可执行文件采用`/boot/vmlinuz`或与之类似的文件名.

   > 在早期的UNIX中，内核可执行文件被称为`vmunix`，而在Linux中，将`unix`换为
   `linux`并将`x`改为`z`，表示其是已经被压缩的可执行文件

    ```shell
    $ cd /boot
    $ ls -l
    Permissions Size User Group Date Modified Name
    .rw-r--r--  264k root root  17 Jun 22:39  config-5.17.5-76051705-generic
    drwx------     - root root   1 Jan  1970  efi
    lrwxrwxrwx    34 root root  10 May 01:08  initrd.img -> initrd.img-5.17.5-76051705-generic
    .rw-r--r--  116M root root  23 Jun 16:27  initrd.img-5.17.5-76051705-generic
    lrwxrwxrwx    34 root root  10 May 01:08  initrd.img.old -> initrd.img-5.17.5-76051705-generic
    .rw-------  6.3M root root  17 Jun 22:39  System.map-5.17.5-76051705-generic
    lrwxrwxrwx    31 root root  10 May 01:08  vmlinuz -> vmlinuz-5.17.5-76051705-generic
    .rw-------   11M root root  17 Jun 22:39  vmlinuz-5.17.5-76051705-generic
    lrwxrwxrwx    31 root root  10 May 01:08  vmlinuz.old -> vmlinuz-5.17.5-76051705-generic
    ```

2. 在Linux上，`sh`是用`bash`仿真实现的
