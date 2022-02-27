1. os为了让它更加易用提供了虚拟化，但本书的重点是os如何提供的虚拟化，这是本书的  
   curx.

2. 正是由于os提供了虚拟化的机制，将各种各样的硬件变成了更加易用且更强大的虚拟的  
   东西，所以我们有时称os为虚拟机。

   > 这种虚拟机和在host os上跑一个guest os不是一个东西。

3. 操作系统是为程序服务的，记住`程序-api-硬件`这个模型，为了让程序告诉os，程序  
   要做什么，os提供了很多的api，也叫做syscall系统调用。所以我们有时也把os看作是  
   一个api库。

4. 操作系统是资源的管理者。

5. `sys/time.h`中的`int gettimeofday(struct timeval *tv, struct timezone *tz);`  
   可以用来拿到距离`EPOCH`的时间秒数及微秒数。在c中，要将它转变为人类可以读的形  
   式可以用`time.h`中的`char *ctime(const time_t *timep);`得到，这个函数处理了时  
   区巴拉巴拉一堆和人有关的复杂的东西。

   > 好像是在`rust in action`这本书中用过这个函数

   > `gettimeofday`中的第二个参数，是时区的东西，已经过时了，应该传入NULL，应该  
   是为了系统的兼容性没有改API。

6. `command &`这种写法，在command后面跟一个`contrl operator`，是将这个command运  
   行在background.

7. 如何在一行shell 命令中执行多个shell 命令: 
   1. `cmd1; cmd2; cmd3`: 运行多条命令，无论前面的命令成功或失败。
   2. `cmd1 && cmd2 && cmd3`: 只有当前面的命令成功了，才执行后一条命令，比如用于  
      `sudo apt update && sudo apt upgrade`检索源成功了才去更新。
   3. `cmd1 || cmd2 || cmd3`: 只有当前面的命令失败了，才执行后一条命令

8. 如何在shell中并发地执行多条命令:

   ```shell
   $ cmd1& ; cmd2& ; cmd3&
   ```
