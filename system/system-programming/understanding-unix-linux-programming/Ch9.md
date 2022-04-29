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