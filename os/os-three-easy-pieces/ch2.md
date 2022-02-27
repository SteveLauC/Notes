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

9. 在c中拿到进程的当前进程的PID:

   ```c
   #include <sys/types.h>
   #include <unistd.h>     // 引用这个可能是为了`pid_t`的类型

   pid_t getpid(void);
   pid_t getppid(void);
   ```

   在rust中可以这样:
   ```rust
   use std::process;
   
   println!("{}", process::id());
   ```
   
   > rust的`id`函数返回`u32`，然而c中定义的`pid_t`的类型是`i32`

10. address spcace layout randomization
   中文叫做位置空间配置随机加载，简称为`ASLR`，是一种防范攻击的os机制。
   
   [how to disable ASLR in ubuntu](https://askubuntu.com/questions/318315/how-can-i-temporarily-disable-aslr-address-space-layout-randomization)
   按照上面的教程我们测试下课本中的内存虚拟化的代码

   ```shell
   # 禁用ASLR
   ➜  kernel echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
   0
   ➜  kernel cat randomize_va_space
   0
   ```

   ```c
   // commmon.h
   #ifndef __common_h__
   #define __common_h__

   #include <sys/time.h>
   #include <assert.h>
   #include <stddef.h>

   double get_time() {
       struct timeval t;
       int rc = gettimeofday(&t, NULL);

       assert(rc==0);

       return (double)t.tv_sec + (double)t.tv_usec/1e6;
   }

   void spin(int howlong) {
       double t = get_time();
       while ((get_time() -t) < (double)howlong) {
           // do nothing
       }
   }

   #endif 
   ```

   ```c
   // main.c

   #include <stdio.h>
   #include <stdlib.h>
   #include <assert.h>
   #include "common.h"
   #include <unistd.h>

   int main(int ac, char *av[]) {
       int *p = malloc(sizeof(int));
       assert(p != NULL);

       printf("(%d) addresss of p: %08x\n", getpid(), (unsigned)p);
       *p = 0;

       while(1) {
           spin(1);
           *p = *p + 1;
           printf("(%d)p: %d\n", getpid(), *p);
       }

       return 0;
   }
   ```
   
   ```shell
   ➜  pg ./main& ; ./main&
   [1] 29027
   [2] 29028
   (29027) addresss of p: 555592a0
   (29028) addresss of p: 555592a0
   ```
   
   我们同时跑两个程序，每个程序都申请一块内存，然后打印申请到的内存的地址，惊人  
   的结果是两个程序申请的内存的地址居然是相同的，这就是os对内存的虚拟化，使得每  
   个进程都具有自己的独立的独占的完整内存。
