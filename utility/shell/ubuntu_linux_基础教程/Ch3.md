##### 3.2
1. 关机
   ```shell
   sudo halt
   shutdown
   shutdown -h 10 # 指定10分钟后关闭 可使用shutdown -c取消
   poweroff
   ```

2. 重启
   ```shell
   shutdown -r 10 # 指定10分钟后重启 可使用shutdown -c取消
   reboot
   init 6
   ```

##### 3.3
1. 查看文件的mtime atime ctime
   默认地，使用`ls -l`或`exa -l`给的时间戳就是mtime。当想要查看atime时，可以使用`ls -lu`或者`exa -lu`。而想要ctime，使用`ls -lc`或者
   `exa -l --changed`。

   值地注意的是，ext4支持了查看文件的创建时间，`exa -lU`就可以查看创建时间。


2. 修改文件的atime和mtime
   可以使用touch命令来做到这个
   ```shell
   touch -at 201901010000.00 file # 修改atime
   touch -mt 201901010000.00 file # 修改mtime
   ```
   解释一下上面的命令，a指的是atime，m指的是mtime，必须跟t来指定时间，时间的格式可以通过man来查看。



3. ls -m 以逗号分隔列出文件文件夹

4. ls -R 递归地列出所有的文件

5. file命令的-b选项，指简洁地打印出文件的类型，指不在文件类型前prepend文件名。


##### 3.4
1. head命令在没有使用`-n`选项指定前几行时默认是显示前10行
   tail命令也是这样的



##### 3.5
1. grep命令在查找匹配的行时，可以使用`-c`的参数直接打印出匹配成功的行数的总数，这样就没必要用管道再接`wc -l`了



##### 3.6
1. find命令的`-empty`用于过滤出文件大小为0或者为空的文件夹
   `-user`可以过滤指定的文件的所有人，后跟用户名
   `-uid`可以跟user的id

2. locate命令是默认依赖于数据库的，如果一个文件刚被删除，数据库又没有被更新的话，那么locate命令会按*老旧的*数据库来输出。
   如果想要确保从数据库中检索的东西存在其指向的文件*存在*的话，可以使用`-e``--existing`选项.

3. 更新locate命令使用的数据库请使用`updatedb`命令，这个命令默认是每天跑一次也就是每天locate的数据库都会更新。

4. whereis命令，可以用来搜索二进制 源码文件 manual pages



##### 3.7
1. du这个命令默认是用来显示文件占用磁盘的信息，而不是其文件大小，磁盘是以block为最小单位进行存储的，故而du的默认显示精度是block
   也就是4KB(在64位机器上)，所以你看到的某个文件或者文件夹所占的磁盘空间是以4KB为单位进行显示的，想要看到精确地和`ls -l`的size一样
   大小的话，我们需要设置显示`--apparent-size`参数来取得其文件的大小
   > google what is the difference between size and size on disk
 

2. 可以使用du统计某一个文件夹的总大小
   `du -sh dir` 以对人类友好的形式打印size on disk
   `du -sh --apparent-size` 以对人类友好的形式打印size
















