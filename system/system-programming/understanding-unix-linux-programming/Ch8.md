1. `ps`命令的输出:
  
   ```shell
   F S UID          PID    PPID  C PRI  NI ADDR SZ WCHAN  STIME TTY          TIME CMD
   0 S steve     170912  163567  0  80   0 -  7051 wait_w Mar03 pts/8    00:00:26 zsh
   0 S steve     345269  311683  0  80   0 -  3662 do_wai Mar06 pts/4    00:00:00 bash
   0 S steve     345281  345269  0  80   0 -  6777 wait_w Mar06 pts/4    00:00:15 zsh
   0 S steve     345323   28145  0  80   0 -  3662 do_wai Mar06 pts/6    00:00:00 bash
   0 S steve     345341  345323  0  80   0 -  4788 sigsus Mar06 pts/6    00:00:00 zsh
   0 S steve     345408  345341  0  80   0 -  3662 do_wai Mar06 pts/6    00:00:00 bash
   0 S steve     345415  345408  0  80   0 -  4788 sigsus Mar06 pts/6    00:00:00 zsh
   0 S steve     345560  345415  0  80   0 -  3662 do_wai Mar06 pts/6    00:00:00 bash
   0 S steve     345593  345560  0  80   0 -  5566 sigsus Mar06 pts/6    00:00:00 zsh
   0 S steve     404043    7728  0  80   0 -  3662 do_wai Mar07 pts/2    00:00:00 bash
   0 S steve     404054  404043  0  80   0 -  6577 wait_w Mar07 pts/2    00:00:15 zsh
   0 S steve     404086  345593  0  80   0 -  3662 do_wai Mar07 pts/6    00:00:00 bash
   0 S steve     404238  404086  0  80   0 -  5589 sigsus Mar07 pts/6    00:00:01 zsh
   0 S steve     500100  404238  0  80   0 -  5557 sigsus Mar08 pts/6    00:00:00 zsh
   0 S steve     500733  500100  0  80   0 -  3662 do_wai Mar08 pts/6    00:00:00 bash
   0 S steve     500895  500733  0  80   0 -  7065 wait_w Mar08 pts/6    00:00:19 zsh
   4 S steve     867274  867172  0  80   0 -  5179 wait_w Mar15 tty4     00:00:00 -zsh
   4 S steve     868140  868138  0  80   0 - 163395 ep_pol Mar15 tty2    01:22:18 /usr/lib/xorg/Xorg vt2 -displayfd 3 -auth /run/user/1000/gdm/Xauthority -background none -noreset -keeptty -verbose 3
   0 S steve     868149  868138  0  80   0 - 48419 poll_s Mar15 tty2     00:00:00 /usr/libexec/gnome-session-binary --systemd --systemd --session=ubuntu
   0 T steve    1100269  981662  0  80   0 -   788 do_sig Mar19 pts/7    00:00:00 target/debug/sigdemo3-rs
   0 T steve    1100410  981662  0  80   0 -   788 do_sig Mar19 pts/7    00:00:00 target/debug/sigdemo3-rs
   0 T steve    1100541  981662  0  80   0 -   788 do_sig Mar19 pts/7    00:00:00 target/debug/sigdemo3-rs
   0 T steve    1101526  981662  0  80   0 -   788 do_sig Mar19 pts/7    00:00:00 target/debug/sigdemo3-rs
   0 T steve    1101661  981662  0  80   0 -   788 do_sig Mar19 pts/7    00:00:00 target/debug/sigdemo3-rs
   0 S steve    1356688  406007  0  80   0 -  5748 sigsus Mar23 pts/3    00:00:04 zsh
   0 S steve    1575156 1273575  0  80   0 -  3206 poll_s 14:46 pts/0    00:00:00 tmux attach-session -t container
   0 S steve    1576123 1356688  0  80   0 -  7690 hrtime 14:58 pts/3    00:00:00 vim Ch8.md
   4 R steve    1576137 1576123  0  80   0 -  4135 -      14:59 pts/3    00:00:00 ps -alf
   ```
    
   `S`字段代码进程的状态，`PRI`是优先级，`NI`代表`niceness`，`SZ`是占用的内存大
   小，`WCHAN`表示进程处理sleep状态的原因(如果进程的`S`为`S`的话)。

2. 单纯使用`ps`输出的进程是和本tty有关的。

3. shell如何运行一个新的程序，使用`int execvp(const char *file, char *const
   argv[])`，这个是c标准里对`int execve(const char *pathname, char *const argv[],
   char *const envp[]);`系统调用的封装

   如果我们要运行`ls -l`命令，需要这样做:

   
   ```c
   // test.c
   #include <unistd.h>
   
   int main() {
       char *cmd = "ls";
       char * arg[3] = {
           "ls",         // 注意`ls`仍然需要在第二个参数中给出来
           "-l",
           NULL,
       };

      execvp(cmd, arg);
      return 0; 
   }
   ```

   但这个函数的特点是，它调用的命令会干掉此进程，比如我们运行`test.c`编译完的程
   序，得到一个进程，然后进程`execvp("ls", arg)`，那么此时`test.c`的进程就已经不
   复存在了，其后其执行的指令，都是`ls`的。

   换句话说，假如我们在execvp后面print一下，是不会得到输出的，因为进程已经被替换
   为`ls`了。

   > execvp就像换脑，The  exec()  family  of  functions replaces the current 
   process image with a new process image.

4. 在rust中要做这个的话，需要这样

   ```rust
   // exec1.rs

   use std::os::unix::prelude::CommandExt;
   use std::process::Command;

   fn main() {
       println!("About to exec ls -l");   

       let mut output: Command = Command::new("ls");
       output.arg("-l");
       output.exec();

       println!("ls is done. bye");
   }
   ```

   为了获得`execvp`的换脑体验，需要使用`std::os::unix::prelude::CommandExt`这个
   trait中的`exec`函数。

5. 使用`strncpy()`来进行拷贝时，需要这样

   ```c
   strncpy(des, src, strlen(src)+1);
   ```

   第三个参数`n`，src的前n个字节需要包含`NUL`，所以是`strlen(src)+1`

   > The strncpy() function is similar, except that at most n bytes of src are 
   copied.  Warning: If there is no null byte among the first n bytes of src, 
   the string placed in dest will not  be  null-terminated.

6. 在使用`exec()`家族函数前申请的内存，在函数调用后，都会被自动释放，无需自行释
   放

   [question_link](https://stackoverflow.com/questions/14492971/how-to-free-memory-created-by-malloc-after-using-execvp)

   > You don't need to. Specifically, if you allocate memory in a process 
   before an exec()-type routine (e.g., execvp() in your case) is called, all 
   of the memory associated with the original executable is released. It's a 
   similar situation to a process exiting (and having all its resources released), 
   and a new process being started with a clean slate.

7. 在rust中`trim`字符串使用的是`str`的`trim`函数，其返回值也是`&str`。所以如果我
   们想要trim的是`String`类型，则比较浪费资源，如果你清楚此值的最后边有一个new
   line的话，可以原地trim(trim in place)

   ```rust
   // str is of type `mut String`
   str.truncate(str.len()-1);
   ```

8. `fork()`进程，来得到和自己一模一样的进程。

   ```c
   #include <stdio.h>
   #inlcude <unistd.h>
   
   int main() {
       printf("Going to fork myself\n");        
       pid_t pid = fork();
       printf("my pid is %d, fork() says %d\n", getpid(), pid); // NOTE: 被两次打印
   }
   ```

   ```shell
   $ gcc main.c && ./a.out
   Going to fork myself
   my pid is 1639022, fork() says 1639023
   my pid is 1639023, fork() says 0
   ```
   
   之所以被两次打印，是因为我们在`fork()`调用后有两个一模一样的进程，它们具有相
   同的PC指针，都指向`fork()`返回后的下一条指令。所以printf语句被打印了两次。那
   怎么判断我们是在父进程还是在子进程里面？通过fork()函数的返回值，在父进程中其
   返回新得到的子进程的pid；而在新创建的子进程中，该函数返回0.

   ```c
   #include <stdio.h>
   #include <unistd.h>
   
   int main() {
       pid_t pid = fork();
       
       if (0 == pid) {
           printf("we are in the child process\n");
       } else {
           printf("we are in the parent process\n");
           printf("And the pid of new child process is %d\n", pid);
       }

       return 0;
   }
   ```

   ```c
   #include <stdio.h>
   #include <unistd.h>

   int main() {
       fork();
       fork();
       fork();
       printf("1\n");
       return 0;
   }
   ```

   上面这段代码会有多少行`1`被打印，答案是8行，因为2^3=8，有点像二分裂


9. rust `nix`中对`fork()`的api的封装

   ```rust
   pub unsafe fn fork() -> Result<ForkResult>
   ```

   函数的返回值中的类型，`ForkResult`是一个`enum`

   ```rust
   pub enum ForkResult {
       Parent {
        child: Pid,
       },
    Child,
   }
   ```

   其实就是将返回值`0/child_pid`给封装了一下，刚好可以使用rust的pattern matching
   来判断是在父进程还是子进程中。

   ```rust
   use nix::unistd::{fork, ForkResult};

   fn main() {
       match unsafe{fork()} {
           Ok(ForkResult::Child) => {
               println!("we are in the child process");
           },
           Ok(ForkResult::Parent{id}) => {
               println!("we are in the parent process");
               // Pid is a wrapper type for pid_t(aka. i32)
               println!("and the new child process is {}", id.as_row());
           },
           Err(_) => (),
       }
   }
   ```

10. 学习到这里，我们已经知道了写shell所需的三项技术中的两项了，如何执行一个命令，
    如何创建一个子进程避免自己本身被替换调。还有一个技术需要学习，即父进程如何等
    待子进程的执行，并读取其返回状态。

    答案是使用`pid_t wait(int *wstatus);`，当函数在父进程中调用，os会将父进程挂起
    ，直到子进程结束，os唤醒父进程。一个父进程可以有很多个子进程，`wait`函数返回
    结束的子进程的`pid`可以来或知是哪一个子进程终结了。子进程退出时存在`状态`，即
    它是怎么退出的，正常退出，非正常退出或者被信号杀死，会在其参数`wstatus`中说明。
    创建一个int的buffer并传过去即可。
    

    传入去的是32bit的数字，但被用到的只有低16位，前8位用来表示退出的状态，再7位记
    录信号的编码(如是被信号杀死的)，最后1位标记是否产生了core dump

    32位数字的编码如下，从Most significant bit到least significant bit
    
    ![note](https://github.com/SteveLauC/pic/blob/main/photo_2022-03-30_15-46-47.jpg) 

    `sys/wait.h`中有一些宏可以帮我们处理这个wait status值:
  
    ```c
    WIFEXITED();      // 是否子进程是自己exit的
    WIFSIGNALED();    // 是否子进程是被信号终止调的
    WIFSTOPPED();     // 被信号暂停了

    WEXITSTATUS();    // 如果是自己exit，则拿到退出的值
    WTERMSIG();       // 如果是被信号杀死，则信号是
    WSTOPSIG();       // 如果是被信号暂停，信号是 

    WCOREDUMP();      // 如果被信号终止，有没有发生core dump
    ```

    这些结果和nix中`WaitStatus`的封装是对应的。

    ```rust
   	pub enum WaitStatus {
		Exited(Pid, i32),
		Signaled(Pid, Signal, bool),
		Stopped(Pid, Signal),
		PtraceEvent(Pid, Signal, c_int),
		PtraceSyscall(Pid),
		Continued(Pid),
		StillAlive,
	} 
	```
