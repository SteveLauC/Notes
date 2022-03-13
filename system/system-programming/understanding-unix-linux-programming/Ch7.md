1. libc中的`unsigned int sleep(unsigned int seconds);`函数有2种可能的情况:
   1. 线程就真的sleep了`seconds`秒，此种情况，函数返回0
   2. 函数调用被不可忽略的信号中断了，返回从调用`sleep`函数到收到信号所跨过的秒数
   。

2. Linux中的`sleep`函数是使用`nanosleep`来实现的，而在有的系统中则是使用`alarm`
   这个syscall实现的。

3. `man 2 alarm`系统调用

   alarm() arranges for a SIGALRM signal to be delivered to the calling process 
   in seconds seconds.
   
   进程对这个信号`SIGALRM`的默认处理是杀死自己，所以如果想要用`alarm`来实现`sleep`，
   需要给这个信号设置一个特殊的处理函数handler。

   ```c
   signal(SIGALRM, handler)
   ```
   所以当`sleep`是由`alarm`实现，在调用`sleep`前，进程会这样做:
   1. 对`SIGALRM`信号设置特殊的handler
   2. 调用`alarm(seconds)`
   3. `pause()`(2) 将自己休眠

4. `man 2 nanosleep`是更高精度的系统调用，支持纳秒。

5. 忽略rust函数返回的Result/Option的小技巧
   可以用`let _ = foo()`来避免warning

6. 如果不满足于`sleep`的精度，可以使用`man 3 usleep`。可以达到`microsecond(us)`
   微秒的精度。

7. Linux中，对于每一个进程，都有3种`interval timer`:
   > Linux是一个分时系统，进程们会共享cpu的时间片。所以在进程A执行的过程中，会
   被挂起，从而将cpu让给别的进程，所以一个进程执行的时间，分为3类，用户态时间，
   内核态时间以及被挂起的睡眠时间。

   1. ITIMER_REAL: 是真实的时间消耗，即从进程开始被执行，到它执行结束，在钟表上
   所消耗的时间。
   2. ITIMER_VIRTUAL: 指的是进程在用户态执行的时间。
   3. ITIMER_PROF: 用户态+内核态执行的时间
  
   > interval timer其实是为了提高精度才引入的，在之前精度只到秒的时候，进程就已
   经有一个计时器了，所以算上3种interval timer，每个进程有4种计时器。

8. c中的restrict关键字，修饰指针时，告诉编译器，这个指针是唯一可以访问这个变量
   的入口

9. 和interval timer相关的系统调用
   
   ```c
   int getitimer(int which, struct itimerval *curr_value);  
   int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
   ``` 
   
   `getitimer`用来取出其timer的值，setitimer用来设置。在取和设置的时候，都需要
   指明你要取和设置的是哪一个timer，在which参数中指明(传入ITIMER_REAL等宏).

   另外留意下结构体的定义:

   ```c
   struct itimerval {
        // 第一次发送信号的时间距离现在的时间间隔
        struct timeval it_interval; /* Interval for periodic timer */
        // 在第一次响后，以后的发送信号的间隔
        struct timeval it_value;    /* Time until next expiration */ 
   };

   struct timeval {
        // 64位机 time_t 和 suseconds_t 都是64bit i.e. long
        time_t      tv_sec;         /* seconds */
        suseconds_t tv_usec;        /* microseconds */
   };
   ``` 
