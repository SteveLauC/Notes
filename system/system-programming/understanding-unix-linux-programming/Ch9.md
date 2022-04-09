1. `realloc(void * ptr, size_t size)`可以把`ptr`指向的堆内存改为`size`的大小，具
   体实现是将原来的`dealloc`，再重新`malloc`下，然后把旧的拷贝到新的里面去

   > 需要注意`ptr`指向的内存必须是堆内存，否则就是UB

2. `strcpy`和`strncpy`中的`src`貌似都不能是`NULL`，否则就是segfault，奇怪的是在手
   册中并没有写明这一点。`strlen`中的参数也是不能为NULL。


3. `strtok(char *, char *)`会将字符串给分割掉，需要注意的是这个函数是直接在你的
   字符串中将delimiter替换为NUL，所以没有任何的内存分配。
