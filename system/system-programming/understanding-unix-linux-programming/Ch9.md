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

7. shell变量包含两种，局部变量和环境变量

   `set`命令是shell的内置命令，`set`可以看到局部和环境变量。而`env`命令是一个单独的程序
   仅可以拿到环境变量
   
   > 就像我们自己写的shell，将所有变量存储在一个数组中，当执行`set`时，会遍历数组将所有的变量打印出来


8. 对变量的操作

   |operation|syntax|
   |---------|-------|
   |赋值      | var=value|
   |引用      | $var|
   |删除      | unset var|
   |输入      | read var |
   |列出变量| set|
   |全局化| export var|
   
    
9. shell变量在内部的存储结构

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
    
13. 在c中如何遍历所有的环境变量，有一个特殊的变量`extern char ** environ`，这是一个字符串数组，遍历它就可以将所有的环境变量打印出
    来
    
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
    
14. 如何拿到具体的环境变量

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