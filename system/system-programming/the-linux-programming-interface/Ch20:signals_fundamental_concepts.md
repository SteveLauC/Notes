#### Ch20: Signals: Foundamental Concepts

> This chapter covers the following topics:
>
> 1. the various different signals and their purposes
> 2. the circumstances in which the kernel may generate a signal for a process,
>    and the syscall that one process may use to send a signal to another process.
> 3. how a process responds to a signal by default, and the means by which a 
>    process can change its response to a signal, in particular, through the use
>    of a signal handler, a programmer defined function that is automatically 
>    invoked on receipt of a signal.
> 4. the use of the process signal mask to block signals, and the associated 
>    notion of pending signals
> 5. how a process can suspend execution and wait for the delivery of a signal.

> ##### 20.1 Concepts and Overview
>
> ##### 20.2 Signal Types and Default Actions
>
> ##### 20.3 Changing Signal Dispositions: `signal(2)`
> 
> ##### 20.4 Introduction to Signal Handlers
> 
> ##### 20.5 Sending Signals: `kill(2)`
> 
> ##### 20.6 Checking for the Existence of a Process
> 
> ##### 20.7 Other Ways of Sending Signals: `raise(3)` and `killpg(3)`
> 
> ##### 20.8 Displaying Signal Descriptions
> 
> ##### 20.9 Signal Sets
> 
> ##### 20.10 The Signal Mask (Blocking Signal Delivery)
> 
> ##### 20.11 Pending Signals
> 
> ##### 20.12 Signals Are Not Queued
> 
> ##### 20.13 Changing Signal Dispositions: `sigaction(2)`
> 
> ##### 20.14 Waiting for a Signal: `pause(2)`

##### 20.1 Concepts and Overview

1. Signal is a notification to a process that an event has occurred. Signals
   are sometimes called `software interrupts`.

2. Signal can be send from one process to another one, in this case, signal
   can be seen as a technique of `synchronization` or a form of `interprocess 
   communication(IPC)`.

   > Though the usual source of most signals sent to the process is the 
   > kernel.

3. Signals can be devided into two categories:
   
   1. The `traditoinal` or `standard` signals (POSIX reliable signal)

      This  category of signals is used by the kernel to notify processes of
      events.
     
      > On Linux, the `standard signals` are numbered from 1 to 31.

   2. `realtime` signals (described in section 22.8) (POSIX real-time signal)

   > What the difference: 
   > [What is the difference between POSIX reliable signals and POSIX real-time signals in Linux?](https://stackoverflow.com/q/39530694/14092446)

4. what is pending or what is the status of `pending`?

   Between the time a signal is generated and the time a signal is delivered
   to the process, a signal is to be said to be `pending`.

   Normally, a pending signal is delivered to a process as soon as it is scheduled
   to run, or immedidately if the process is already running.

   If the pending signal is in the signal mask, then it is blocked and remaines
   pending, until it is unblocked (removed from the mask).

5. Actions to a signal

   When a signal is delivered, a process can carry out one of the following 
   **default** actions, depending on the signal:

   1. ignore it: the signal is discarded by the kernel and **the process is not
      aware of it**.
   
   2. The process is terminated (killed)

   3. A `core dump file` is generated, and the process is terminated.

   4. The process is stopped (suspended)

   5. Execution of the process is resumed.

   Instead of accepting the default action, a process can change the action that
   occurs when the signal is deliverrd, this is called `disposition` and can be 
   done using `signal(2)` or `sigaction(2)`. The action can be set to:

   1. Default Action should occur (This is typically used to **undo the previous
      disposition**)

      ```c
      signal(signum, SIG_DFL);
      ```
   
   2. Ignore the signal

      ```c
      signal(signum, SIG_IGN);
      ```

   3. Use the user-defined `signal handler`

      > This is typically called `installing` or `establishing` a signal handler.

      ```c
      void * my_customized_signal_handler(int signum) {
              printf("Caught signal %d\n", signum);
              return NULL;
      }

      signal(signum, my_customized_signal_handler);
      ```

##### 20.2 Signal Types and **Default** Actions

1. Various signals and their meanings

   * SIGABRT: This signal is sent when the process calls `abort(3)`, this signal 
     terminates the process with a core dump file.

   * SIGALRM: The kernel generation this signal upon the expiration of a **real time**
     timer set by a call to `alarm(2)` or `setitimer(2)`

     > `real time` means that time counts accroding to the wall clock.
     >
     > `man 2 setitimer`: ITIMER_REAL

     > `SIGVTALRM` is used for **virtual timer**

   * SIGBUS: This signal (bus error) is sent to indicate kinds of memory error.
     If we use `mmap(2)` to access the memory that does not belongs to us, we will
     get this.

   * SIGCHLD: When one of the child processes terminates (either by calling `exit(3)`
     or killed by a signal), this signal is sent to the parent process.

     It may also be sent to the parent if one of its children is stopped or resumed.

   * SIGCLD: Synonym for `SIGCHLD`.

   * SIGCONT: When sent to a **stopped process**, this signal causes the process to 
     **resume**.

     When sent to a non-stopped process, this signal is ignored.

   * SIGEMT: In UNIX systems generally, this signal is used to indicate an 
     implementaion dependent hardware error. On Linux, this signal is used only
     in the **Sun SPARC** (An architecture) implementation. The suffix `EMT` 
     means `emulator trap`, an assembler mnemonic on the Digital PDP-11.

   * SIGFPE: This signal is generated for certain types of arithmetic errors, 
     such as **divide-by-zero**. The suffix FPE is an abbreviation for floating-point 
     exception, *although this signal can also be generated for integer arithmetic 
     errors*. The precise details of when this signal is generated depend on the 
     hardware architecture and the settings of CPU control registers. 

     For example, on x86-32, integer divide-by-zero always yields a SIGFPE, but
     the handling of floating-point divide-by-zero depends on whether the 
     FE_DIVBYZERO exception has been enabled. If this exception is enabled 
     (using feenableexcept()), then a floating-point divide-by-zero generates 
     SIGFPE; otherwise, it yields the IEEE-standard result for the operands 
     (a floating-point representation of infinity). See the fenv(3) manual page
     and `<fenv.h>` for further information.

   * SIGHUP: When a terminal disconnect (hangup) occurs, this signal is sent to
     the controlling process of the terminal. We describe the concept of a 
     controlling process and the various circumstances in which SIGHUP is sent 
     in Section 34.6. 

     A second use of SIGHUP is with daemons (e.g., init, httpd,
     and inetd). Many daemons are designed to respond to the receipt of SIGHUP 
     by reinitializing themselves and rereading their configuration files. The 
     system administra- tor triggers these actions by manually sending SIGHUP 
     to the daemon, either by using an explicit kill command or by executing 
     a program or script that does the same.

   * SIGILL: This signal is sent to the process who tries to execute an illegal
     instument.

     > ILL stands for `illegal`.

   * SIGINFO: On Linux, this is equivalent to `SIGPWR` (power failure). 

     On BSD systems, this signal is generated by typing `Control-T`, is used 
     to obtain status information about the foreground process group.

   * SIGINT: This signal is sent to the process when the user type the `interrupt`
     character (Usually by `Contr-C`). The default action for this signal is 
     terminating the process.

   * SIGIO: Using the `fcntl(2)` system call, it is possible to arrange for this 
     signal to be generated when an I/O event (e.g., input becoming available) 
     occurs on certain types of open file descriptors, such as those for terminals 
     and sockets. This feature is described further in Section 63.3.

   * SIGIOT: On Linux, this is a synonym for `SIGABRT`. On some traditional UNIX 
     implementations, this signal indicates an implementation-defined hardware
     fault. 

   * SGIKILL: This is the `sure kill` signal. **It can't be blocked, ignore, or 
     caught by a handler**. And thus always terminates a process.

   * SIGLOST: This signal exists on Linux, but is unused. 
     
     On some other UNIX implementations, the NFS client sends this signal to 
     local processes holding locks if the NFS client fails to regain locks 
     held by the those processes following the recovery of a remote NFS server 
     that crashed. (This feature is NOT standardized in NFS specifications.)

   * SIGPIPE: THis signal is generated when a process tries to write to a pipe,
     a FIFO, or a socket **for which there is NO corresponding reader process**.

     This normally occurs because the reading process has closed its file 
     descriptor for the IPC channel. See Section 44.2 for further details.

   * SIGPOLL: This signal is derived from System V, and is a synonym for `SIGIO`
     on Linux. 

   * SIGPROF: The kernel generates this signal upon the expiration of a profiling 
     timer set by a call to `setitimer(2)` (Section 23.1). A profiling timer is one 
     that counts the CPU time used by a process. Unlike a virtual timer (see 
     `SIGVTALRM` below), a profiling timer counts CPU time used in both user mode 
     and kernel mode.

   * SIGPWR: This is the `power failure signal`. On systems that have an 
     uninterruptible power supply (UPS), it is possible to set up a daemon 
     process that monitors the backup battery level in the event of a power failure. 
     If the battery power is about to run out (after an extended power outage),
     then the monitoring process sends `SIGPWR` to the `init process`, which 
     interprets this signal as a request to shut down the system in a quick 
     and orderly fashion.

     > On Linux, `SIGINFO` is equivalent to this.

   * SIGQUIT: When the user types the quit character (`usually Control-\`) on the 
     keyboard, this signal is sent to the foreground process group. By default, 
     this signal terminates a process and causes it to produce a core dump, 
     which can then be used for debugging. Using `SIGQUIT` in this manner is 
     useful with a program that is stuck in an infinite loop or is otherwise 
     not responding. By typing Control-\ and then loading the resulting core 
     dump with the `gdb` debugger and using the backtrace command to obtain a 
     stack trace, we can find out which part of the program code was executing. 

   * SIGSEGV:  segfault

     This very popular signal is generated when a program makes an 
     invalid memory reference. A memory reference may be invalid because the 
     referenced page doesn’t exist (e.g., it lies in an unmapped area somewhere
     between the heap and the stack), the process tried to update a location in 
     read-only memory (e.g., the program text segment or a region of mapped memory
     marked read-only), or the process tried to access a part of kernel memory 
     while running in user mode (Section 2.1). 

     In C, these events often result from dereferencing a pointer containing a 
     bad address (e.g., an uninitialized pointer) or passing an invalid argument
     in a function call. The name of this signal derives from the term 
     `segmentation violation`.

   * SIGSTKFLT: Documented in `signal(7)` as `stack fault on coprocessor`, this
     signal is defined, but is unused on Linux.

   * SIGSTOP: This is the sure `stop` signal. **It can't be blocked, ignored or 
     caught by a handler;** Thus, it always stops a process.

   * SIGSYS: This signal is sent when a process executes a syscall trap (e.g., 
     0x80 on x86 arch), but the syscall number placed in the specific register
     is invalid.

     > bad syscall

     > Refers to 
     > [Ch3: 1](https://github.com/SteveLauC/Notes/blob/main/system/system-programming/the-linux-programming-interface/Ch3:system_programming_concepts.md)
     > to see more information about the precedures of syscall.

   * SIGTERM: This is the **standard signal used for terminating a process** and is 
     the **default signal sent by the kill and killall commands**. 

     Users sometimes explicitly send the `SIGKILL` signal to a process using 
     `kill –KILL` or `kill –9`. However, *this is generally a mistake*. 

     A well-designed application will have a handler for `SIGTERM` that causes
     the application to exit *gracefully*, cleaning up tem- porary files and 
     releasing other resources beforehand. Killing a process with SIGKILL 
     bypasses the SIGTERM handler. Thus, we should always first attempt to 
     terminate a process using `SIGTERM`, and reserve `SIGKILL` as a last 
     resort for killing runaway processes that don’t respond to `SIGTERM`.

   * SIGTRAP: This signal is used to implement debugger breakpoints and system 
     call tracing, as performed by `strace(1)` (Appendix A). See the `ptrace(2)` 
     manual page for further information.

   * SIGTTIN: When running under a job-control shell, the terminal driver sends
     this signal to a background process group when it attempts to `read(2)` from 
     the terminal. This signal stops a process by default

   * SIGTTOU: This signal serves an analogous purpose to `SIGTTIN`, but for **terminal
     output** by background jobs. When running under a job-control shell, if the 
     `TOSTOP (terminal output stop)` option has been enabled for the terminal 
     (perhaps via the command stty tostop), the terminal driver sends `SIGTTOU` to 
     a background process group when it attempts to `write(2)` to the terminal (see 
     Section 34.7.1). This signal stops a process by default.

   * SIGUNUSED: As the name implies, this signal is **unused**. On Linux 2.4 and 
     later, this signal name is synonymous with SIGSYS on many architectures. 
     In other words, this signal number is no longer unused on those architectures, 
     although the signal name remains for backward compatibility.

   * SIGURG: This signal is sent to a process to indicate the presence of 
     out-of-band (also known as *urgent*) data **on a socket** (Section 61.13.1).

   * SIGUSR1/SIGUSR2: 

     > `USR` stands for `user`.

     This signal and `SIGUSR2` are available for programmer-defined purposes. 
     The kernel never generates these signals for a process. Processes may use 
     these signals to notify one another of events or to synchronize with each 
     other. 

     In early UNIX implementations, these were the only two signals that could 
     be freely used in applications. (In fact, processes can send one another 
     any signal, but this has the potential for confusion if the kernel also 
     generates one of the signals for a process.) Modern UNIX implementations 
     provide a large set of realtime signals that are also available for 
     programmer-defined purposes (Section 22.8). 

   * SIGVTALRM: The kernel generates this signal upon expiration of a **virtual 
     timer** set by a call to `setitimer(2)` (Section 23.1). 

     A virtual timer is one that counts the **usermode CPU** time used by a process.

     > `man 2 setitimer`: `ITIMER_VIRTUAL`

     > `SIGALRM` is used for `realtime` timer.

   * SIGWINCH: In a windowing environment, this signal is sent to the foreground
     process group **when the terminal window size changes** (as a consequence either
     of the user manually resizing it, or of a program resizing it via a call 
     to ioctl(), as described in Section 62.9). 

     By installing a handler for this signal, programs such as **vi** and **less** can 
     know to redraw their output after a change in window size.

   * SIGXCPU: This signal is sent to a process when it exceeds its CPU time 
     resource limit (`RLIMIT_CPU`, described in Section 36.3).

   * SIGXFSZ: This signal is sent to a process if it attempts (using `write(2)`
     or `truncate(2)`) to increase the size of a file beyond the process’s file 
     size resource limit (`RLIMIT_FSIZE`, described in Section 36.3).

##### 20.3 Changing Signal Dispositions: `signal(2)`
##### 20.4 Introduction to Signal Handlers
##### 20.5 Sending Signals: `kill(2)`
##### 20.6 Checking for the Existence of a Process
##### 20.7 Other Ways of Sending Signals: `raise(3)` and `killpg(3)`
##### 20.8 Displaying Signal Descriptions
##### 20.9 Signal Sets
##### 20.10 The Signal Mask (Blocking Signal Delivery)
##### 20.11 Pending Signals
##### 20.12 Signals Are Not Queued
##### 20.13 Changing Signal Dispositions: `sigaction(2)`
##### 20.14 Waiting for a Signal: `pause(2)`
