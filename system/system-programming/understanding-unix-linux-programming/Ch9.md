1. `realloc(void * ptr, size_t size)`可以把`ptr`指向的堆内存改为`size`的大小，具
   体实现是将原来的`dealloc`，再重新`malloc`下，然后把旧的拷贝到新的里面去

   > 需要注意`ptr`指向的内存必须是堆内存，否则就是UB

2. `strcpy`和`strncpy`中的`src`貌似都不能是`NULL`，否则就是segfault，奇怪的是在手
   册中并没有写明这一点。`strlen`中的参数也是不能为NULL。


3. `strtok(char *, char *)`会将字符串给分割掉，需要注意的是这个函数是直接在你的
   字符串中将delimiter替换为NUL，所以没有任何的内存分配。

4. shell中的if语句如果有多个命令的话，会以最后一个命令的退出值作为if判断的条件，换言之，
   只有最后一个命令`exit(0)`，if才为真，然后执行。

5. shell中if语句的工作流程:  
   1. shell运行if后面的命令
   2. shell检查命令的退出状态
   3. exit的状态为0，则表示成功；非0，表示失败
   4. 如果成功，shell执行then部分的代码
   5. 如果失败，shell执行else部分的代码
   6. 关键字fi标志着if块的结束

6. rust中的`read_line`:

   * std::io::Stdin::read_line
   * std::io::BufRead::read_line 

   前者的实现当中，是这样的
   
   ```rust
   pub fn read_line(&self, buf: &mut String) -> io::Result<usize> {
        self.lock().read_line(buf)
   } 
   ```
   
   它调用了自身的`lock`方法拿到了`StdinLock`类型的实例，而`StdinLock`类型刚好实现了`BufRead`的trait
   所以`std::io::Stdin::read_line`就是调用了`std::io::BufRead::read_line`。

7. UNIX变量包含两种，shell变量和环境变量(environment variable)

   `set`命令是shell的内置命令，`set`可以看到shell和环境变量。而`env`命令是一个单独的程序
   仅可以拿到环境变量(env是怎么做的，遍历environ字符串数组罢了)
   
   > 命令的管理，shell变量字母小写而环境变量大写，但`http_proxy/https_proxy`这两个环境变量通常是小写的
   [What's the 'right' format for the HTTP_PROXY environment variable? Caps or no caps?](https://unix.stackexchange.com/questions/212894/whats-the-right-format-for-the-http-proxy-environment-variable-caps-or-no-ca)

   > 这里的名字可能有点歧义，在学shell编程时，统一将所有的变量都称作shell variable，而在这里我们叫它UNIX variable。
   这里的shell variable只的是仅shell自身可见的变量

8. 对变量的操作

   |operation|syntax|
   |---------|-------|
   |赋值      | var=value|
   |引用      | $var|
   |删除      | unset var|
   |输入      | read var |
   |列出变量| set|
   |全局化| export var|
   
    
9. UNIX变量在内部的存储结构

   ```c 
   struct var{
      char * str; // name=val
      int global;
   }
   ```
   
10. rust中数组里批量初始化的写法，需要那个类型满足`Copy`的trait

    ```
    A repeat expression [x; N], which produces an array with N copies of x. The type of x must be Copy.
    ```
    
    如果你要初始化的类型它不copy，可以这样做
    ```rust
    #[derive(Default)]
    struct NonCopy;
    
    fn main() {
       let arr: [NonCopy;200] = [();200].map(|_| NonCopy::default());
    }
    ```
11. 我们目前写的shell的结构是这样的

    ```
    is control command:
      do control command
    is built-in command:
      do built-in command
    do shell command
    ```
    要先将特殊的过滤出去，然后再执行普通的shell命令
    
12. 当使用`sudo command`来运行命令的时候，它会覆盖当前用户的环境变量

    ```shell
    $ sudo bash -c 'echo $http_proxy'

    $ sudo -E bash -c 'echo $http_proxy'
    http://127.0.0.1:8890  
    ```
    
    可以使用`sudo -E`来preserver variable
    > -E, --preserve-env
    Indicates to the security policy that the user wishes to preserve their 
    existing environment variables.  The security policy may return an error if the
    user does not have permission to preserve the environment.
    
    **但是**，对于`bash`来言，`PATH`使用`-E`是救不回来的
    
    ```shell
    $ sudo -E bash -c 'echo $PATH'
    /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin
    $ sudo bash -c 'echo $PATH'
    /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin    
    ```

    但`zsh`是可以救回来的：）
    ```shell
    $ sudo zsh -c 'echo $PATH'
    /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin
    $ sudo -E zsh -c 'echo $PATH'
    /home/steve/.cargo/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin
    ```
    
    注意的是，在检测`sudo`会不会覆盖环境变量时，不要用`sudo echo $PATH`或者`sudo echo $http_proxy`，这样
    环境变量的替换还是由当前自己的shell来完成的。
    
13. 在c中如何遍历所有的环境变量(environment variables)，有一个特殊的变量`extern char ** environ`，这是一个
    字符串数组，遍历它就可以将所有的环境变量打印出来
    
    ```c
    #include <stdio.h>
    extern char ** environ;
    int main() {
       for (int i = 0; environ[i]; i+=1) {
          printf("%s\n", environ[i]);
       }
       return 0;
    }
    ```
    
14. 如何拿到具体的环境变量(environment variable)

    ```c
    #include <stdlib.h>
    #include <stdio.h>

    int main() {
       char * target = getenv("PATH");
       if (target != NULL) {
          printf("PATH=%s\n", target);
       } else {
          printf("PATH is unset");
       }
       return 0;
    }
    ```
    
15. 前面说过，在使用`exec`家族的系统调用时就像是进行了换脑。但有一个例外，则是进程的环境变量(environment variable)

    ```c
    /*
     * changeenv.c: shows how to change the environment
     * note: calls `env` to display environments variables
    */
    #include <unistd.h>

    extern char ** environ;

    int main() {
        char * table[3] = {NULL, NULL, NULL};  // don't forget that this string-array needs to be NULL-terminated 
        table[0] = "TERM=vt100";
        table[1] = "/on/the/range";
        
        environ = table;

        // 在这里进行了患脑，但是环境变量仍然没有换        
        execlp("env", "env", NULL);
    }
    ```

16. 前面我们在自己的shell中定义了如何存储UNIX变量，但是并没有和系统里面真正的环境变量进行交互。而系统内真正的环境变量
    就是`environ`这个字符串数组以及`getenv/setenv/clearenv`的接口，我们仍然需要将UNIX变量中的环境变量放到
    `environ`数组中以及将数组中的变量放到我们的UNIX变量中的2个接口，来提供和真正的环境变量之间的交互

    而且我们应该在shell启动的时候，就将`environ`中的环境变量导入到我们自己的存储结构中
    
17. 父进程给子进程传消息的方式:

    1. 通过参数 fork之后，调用execvp时把参数传给子进程。子进程通过main函数的`ac/av`来接收
    2. 环境变量，fork时，父进程的环境变量直接复制给了子进程。
    
18. 我们自己写的shell，在varlib.c中对tab做的修改，都是对父进程中tab做的，并没有改子进程中tab这个变量，毕竟改了也
    没有用，子进程调用exec后tab变量就消失了，子进程只剩下environ这个东西了。
    
    用户在使用smsh的时候，与UNIX变量做的交互都是使用的是自己定义的tab及其api，并没有直接修改envrion。这就满足了在使用shell时所有
    和UNIX变量交互的需求了。但当fork子进程时，我们就不能单纯地和tab进行交互了，而是需要将tab中的环境变量放到父进的environ中去，由
    OS完成父进程environ到子进程environ的拷贝
    
    ```
    // 何时对tab进行修改，何时对environ进行修改

    shell启动: 在父进程中，将environ中的环境变量拷贝到tab中
    用户使用shell:
      1. 创建UNIX变量
      2. 更新UNIX变量
      3. 打印UNIX变量
      ...
      都是在和tab交互，而不是和environ交互
      
    用户要执行别的程序，创建子进程了，在父进程中将tab中的环境变量拷贝到environ中去(毕竟用户可能对环境变量做了修改，需要交给子进程)
    而tab中的shell变量则不需要动，毕竟这是shell独属的
    ```