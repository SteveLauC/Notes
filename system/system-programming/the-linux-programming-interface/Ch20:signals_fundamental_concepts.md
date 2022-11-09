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

1. Various signals

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
