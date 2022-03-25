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

   > execvp就像换脑

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
