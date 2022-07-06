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

   > 其实就是3种时间 [TLPI Ch2 18](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch2.md)

8. c中的restrict关键字，修饰指针时，告诉编译器，这个指针是唯一可以访问这个变量
   的入口

9. 和interval timer相关的系统调用
   
   ```c
   int getitimer(int which, struct itimerval *curr_value);  
   int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
   ``` 
   
   `getitimer`用来取出其timer的值，setitimer用来设置。在取和设置的时候，都需要
   指明你要取和设置的是哪一个timer，在which参数中指明(传入ITIMER_REAL等宏).

   `setitimer`在使用时需要传入1，2个参数，第3个参数是用来获取以前的设置的。如果
   不需要拿到以前的设置，传入NULL就好。如果需要的话，则传入你准备好的buffer的指
   针。

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

10. 虽然每个进程都有自己的时钟，但是os却只维护一个真正的时钟，进程只维护一个计数
    时间。每当os的时种过一个时间片后，内核收到一个时种脉冲，os就会对所有进程的计
    数时间做递减。当进程中的这个时间到0了，再对进程发送`SIGALRM`信号。

11. `man 2 signal`中写道这个函数是在不同的UNIX中的行为是不同的，并不能跨平台。
  
    > The only portable use of signal() is to set a signal's disposition to 
      SIG_DFL or SIG_IGN.  The semantics when using signal() to establish a 
      signal handler vary across systems (and POSIX.1 explicitly permits this 
      variation); do not use it for this purpose.这个函数可以跨平台的就只有设
      置默认处理或者忽略的时候，当设置`interrupt handler`时，在不同的系统中是
      不一样的，POSIX也允许这一点。
 
12. POSIX推荐的设置信号处理的函数，`sigaction()`  
    其中的action结构体
     
    ```c
    struct sigaction
    {
        /* Signal handler.  */
    #if defined __USE_POSIX199309 || defined __USE_XOPEN_EXTENDED
        union
        {
        /* Used if SA_SIGINFO is not set.  */  // 这里有些新旧的处理函数的模式
        __sighandler_t sa_handler;
        /* Used if SA_SIGINFO is set.  */
        void (*sa_sigaction) (int, siginfo_t *, void *);
          
        }
        __sigaction_handler;
    # define sa_handler __sigaction_handler.sa_handler
    # define sa_sigaction   __sigaction_handler.sa_sigaction
    #else
        __sighandler_t sa_handler;
    #endif

        /* Additional set of signals to be blocked.  */
        __sigset_t sa_mask;

        /* Special flags.  */
        int sa_flags;

        /* Restore handler.  */
        void (*sa_restorer) (void);
    }
    ```
   
    ```c
    // 来自GNU文档的更加简洁的结构体定义
    struct sigaction {
        void     (*sa_handler)(int);                        // 处理函数
        void     (*sa_sigaction)(int, siginfo_t *, void *);
        sigset_t   sa_mask;    // unsigned long
        int        sa_flags;
        void     (*sa_restorer)(void);
    };
    ```
    
    `man 2 sigaction`中写道，在一些平台上，使用了union，所以`sa_handler`和
    `sa_sigaction`不能同时被赋值。如果你只需要和`man 2 signal`相同的功能，设置处
    理行为为`SIG_IGN/SIG_DFL/自定义函数`，传给`sa_handler`就好。如果要使用新的信
    号处理模式的话，则需要使用`sa_sigaction`并将`SA_SIGINFO`赋给`sa_flags`。最后
    一个字段`sa_restorer`并非应用程序可以用的，且POSIX并没有指定这个字段。`sa_mask`
    和`sa_flags`这两个字段在使用新旧信号处理时均可以使用。

    关于新旧信号处理函数，旧的信号处理函数只能拿到被处理的信号的编号，在处理函数
    `sa_handler(int)`中的参数中可以看到。而若使用新的信号处理，则除了拿到被处理
    信号的编号，还可以获取被调用的原因以及产生问题的上下文的相关信息。

    新信号处理拿到的信息在`siginfo_t`的结构体:  

    ```c
    siginfo_t {
        int      si_signo;     /* Signal number */
        int      si_errno;     /* An errno value */
        int      si_code;      /* Signal code */
        int      si_trapno;    /* Trap number that caused
                                 hardware-generated signal
                                 (unused on most architectures) */
        pid_t    si_pid;       /* Sending process ID */
        uid_t    si_uid;       /* Real user ID of sending process */
        int      si_status;    /* Exit value or signal */
        clock_t  si_utime;     /* User time consumed */
        clock_t  si_stime;     /* System time consumed */
        sigval_t si_value;     /* Signal value */
        int      si_int;       /* POSIX.1b signal */
        void    *si_ptr;       /* POSIX.1b signal */
        int      si_overrun;   /* Timer overrun count;
                                 POSIX.1b timers */
        int      si_timerid;   /* Timer ID; POSIX.1b timers */
        void    *si_addr;      /* Memory location which caused fault */
        long     si_band;      /* Band event (was int in
                                  glibc 2.3.2 and earlier) */
        int      si_fd;        /* File descriptor */
        short    si_addr_lsb;  /* Least significant bit of address
                                  (since Linux 2.6.32) */
        void    *si_lower;     /* Lower bound when address violation
                                  occurred (since Linux 3.19) */
        void    *si_upper;     /* Upper bound when address violation
                                  occurred (since Linux 3.19) */
        int      si_pkey;      /* Protection key on PTE that caused
                                  fault (since Linux 4.6) */
        void    *si_call_addr; /* Address of system call instruction
                                  (since Linux 3.5) */
        int      si_syscall;   /* Number of attempted system call
                                  (since Linux 3.5) */
        unsigned int si_arch;  /* Architecture of attempted system call
                                 (since Linux 3.5) */
    }
    ```

    `sa_mask`是用来设置信号阻塞的，`sa_flags`是用来定义信号的行为的。

    
13. UNIX中的软件中断被称为信号(signal)

14. 对于`SIGALRM`信号，其默认唤醒进程。

15. 通过`sigdemo3.c`的测试，发现:  
    1. 对于blocking的系统调用，信号会中断其系统调用，并且使之前buf中的东西清空。
    2. 系统对于连续的相同的信号，后面的信号会被阻塞，直到前面的信号被处理结束。
    3. 对于不同的信号的连续调用，后者会中断前者，后者执行完再去执行前者，有点像
    递归的函数调用。

16. 关于`sa_mask`的一些api:  
    
    ```c
    #include <signal.h>
    int sigemptyset(sigset_t *set);                  // 将set置空
    int sigfillset(sigset_t *set);                   // 将set置满
    int sigaddset(sigset_t *set, int signum);        // 添加signum到set
    int sigdelset(sigset_t *set, int signum);        // 删除signum到set
    int sigismember(const sigset_t *set, int signum);// 检查signum是否在set中
    ```
    

    ```c
    /* Prototype for the glibc wrapper function */
    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
    ``` 
    
    sigprocmask()  is  used  to fetch and/or change the signal mask of the 
    calling thread.  The signal mask is the set of signals whose delivery is 
    currently blocked for the caller (see also signal(7) for more details).

    The behavior of the call is dependent on the value of how, as follows.  

       * SIG_BLOCK The set of blocked signals is the union of the current set 
       and the set argument.

       * SIG_UNBLOCK The signals in set are removed from the current set of blocked signals.  It is 
       permissible to attempt to  unblock  a  signal which is not blocked.

       * SIG_SETMASK The set of blocked signals is set to the argument set.

    If oldset is non-NULL, the previous value of the signal mask is stored in oldset.

    If  set is NULL, then the signal mask is unchanged (i.e., how is ignored), 
    but the current value of the signal mask is nevertheless returned in oldset 
    (if it is not NULL).(fetch的话就这样做 sigprocmask(SIG_SETMASK, NULL, buf))

    A set of functions for modifying and inspecting variables of type sigset_t 
    ("signal sets") is described in sigsetops(3).(就是这点笔记最开始提到的一堆api)

    The use of sigprocmask() is unspecified in a multithreaded process; see 
    pthread_sigmask(3).
  
    第一组api和`sigprocmask`是用来结合着使用的，使用第一个来创建sigset，然后使用
    `sigprocmask`进行设置。比如下面的代码片段进行设置:   
    
    ```c
    sigset_t sigs, prevsigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);               // 添加SIGINT
    sigaddset(&sigs, SIGQUIT);              // 添加SIGQUIT
    sigprocmask(SIG_SET, &sigs, &prevsigs); // 进行阻塞 
    sigprocmask(SIG_SET, &prevsigs, NULL);  // 将阻塞set还原
    ```

17. `sa_flags`中的`SA_NODEFER`标记，正常情况下，在一个信号的handler被调用的时候，
    如果再来一个这个信号，则后来的信号会被阻塞直到上一个handler执行完毕。但当这
    个`SA_NODEFER`标记被设置的话，则允许同一信号处理函数的递归调用，后来的新的相
    同信号则不会被阻塞。


18. UNIX中给用于留下的，可以由用户自定义支配的两个信号，`SIGUSR1/SIGUSR2`

19. 可以使用`kill`函数来让进程给另一个进程发送任意信号
  
    ```c
    #include <sys/types.h>
    #include <signal.h>

    int kill(pid_t pid, int sig);
    ```
20. 记录一下时间的单位
 
    1. 秒   second      s
    2. 毫秒 millisecond ms
    3. 微妙 microsecond us
    4. 纳秒 nanosecond  ns

21. `curses.h`中的`crmode()`设置终端进入char by char的模式，就和我们之前使用termios.h
    中的`tcgetattr`以及`tcsetattr`是一样的。

22. `curses.h`中的`noecho()`用来关闭tty的回显。

23. 究竟什么是异步?
    和concurrency有点混乱，这个stackoverflow的答案说的很好。


    Concurrent and parallel are effectively the same principle as you correctly 
    surmise, both are related to tasks being executed simultaneously although I 
    would say that parallel tasks should be truly multitasking, executed "at the 
    same time" whereas concurrent could mean that the tasks are sharing the 
    execution thread while still appearing to be executing in parallel.

    Asynchronous methods aren't directly related to the previous two concepts, 
    asynchrony is used to present the impression of concurrent or parallel 
    tasking but effectively an asynchronous method call is normally used for 
    a process that needs to do work away from the current application and we 
    don't want to wait and block our application awaiting the response.

    > 异步指的就是从你的主任务分出一个子任务来，但不想这个子任务将主任务block掉。
    主任务不需要去等子任务完成才继续自己的执行，而是继续执行自己，当子任务完成，
    回调函数会告知主任务子任务已经完成了

    For example, getting data from a database could take time but we don't 
    want to block our UI waiting for the data. The async call takes a call-back 
    reference and returns execution back to your code as soon as the request has 
    been placed with the remote system. Your UI can continue to respond to the 
    user while the remote system does whatever processing is required, once it 
    returns the data to your call-back method then that method can update the
    UI (or handoff that update) as appropriate.

    From the User perspective, it appears like multitasking but it may not be.

    EDIT

    It's probably worth adding that in many implementations an asynchronous 
    method call will cause a thread to be spun up(created) but it's not essential, 
    it really depends on the operation being executed and how the response can 
    be notified back to the system.

    > 异步依赖于实现
  
    最常见的异步操作是异步的IO了

24. UNIX的asychronous input有两种：  
    1. 当输入就绪时发送信号，在使用`open`时给`O_ASYNC`flag即可或者使用`fcntl`手
    动修改
    2. 当输入被读入时发送信号，POSIX标准，调用`aio_read`函数。可以使用`man 7 aio`
    来查看更多信息。

    > 来自`man 7 aio`  
    The POSIX asynchronous I/O (AIO) interface allows applications to initiate 
    one or more I/O operations that are performed asynchronously (i.e., in the 
    background).  The application  can  elect to be notified of completion of 
    the I/O operation in a variety of ways: by delivery of a signal, by 
    instantiation of a thread, or no notification at all.

    当使用第一个种方式时，I/O完成的时候会给我们的进程发送`SIGIO`的信号，所以我们
    的对IO的操作必须放在这个信号的handler里面。
    
    ```c
    /*前提*/

    // 确保SIGIO信号是发给此进程的。
    // Set the process ID or process group ID that will receive SIGIO  and  
    // SIGURG  signals  for events on the file descriptor fd. 
    fcntl(0, F_SETOWN, getpid());
    // 打开O_ASYNC属性
    fcntl(0, F_SETFL, fcntl(0, F_GETFL)|O_ASYNC);
    ``` 

    ```c
    // 设置SIGIO的handler
    signal(SIGIO, handler);
    ```

    ```c
    void handler(int signum) {
        // scanf(); some IO function
    }
    ```

    
    当使用第二种方式时，必须先准备一个`struct aiocb`的buf，来配置想要的异步IO请
    求:

    ```c
    struct aiocb
    {
      nt aio_fildes;       /* File desriptor.  */ // 从哪里读
      int aio_lio_opcode;       /* Operation to be performed.  */
      int aio_reqprio;      /* Request priority offset.  */
      volatile void *aio_buf;   /* Location of buffer.  */  // 读到哪里去
      size_t aio_nbytes;        /* Length of transfer.  */  // 读多少个字节
      struct sigevent aio_sigevent; /* Signal number and value.  */ // 如果读到了，应该做什么

      /* Internal members.  */
      struct aiocb *__next_prio;
      int __abs_prio;
      int __policy;
      int __error_code;
      __ssize_t __return_value;

       #ifndef __USE_FILE_OFFSET64
         __off_t aio_offset;       /* File offset.  */
         char __pad[sizeof (__off64_t) - sizeof (__off_t)];
       #else
         __off64_t aio_offset;     /* File offset.  */
       #endif
         char __glibc_reserved[32];

    };
    ```

    然后再使用`aio_read(ptr_to_aiocb_struct)`来生成一个请求。然后等键盘输入字符
    后就会发送你设置的信号(如果aiocb的aio_sigevent中的signotify设置为SIGEV_SIGNAL)
    。就可以在对应信号的处理函数中使用`aio_return`来拿到读到的东西。
