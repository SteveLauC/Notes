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
