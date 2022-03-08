1. `#include <arpa/inet.h>`中的`htonl(unsigned long n)`函数是转换字节顺序的，从
   host的字节序转换为net的字节序。

2. `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);`函数中
   的第二个参数的类型是`struct sockaddr *`，但其实真正在用的时候，其address的类型
   取决于具体的address family。在这里统一使用`sockaddr`类型是为了统一接口吧。


   The actual structure passed for the addr argument will depend on the address 
   family.  The sockaddr structure is defined as something like:

   ```c
   struct sockaddr {
       sa_family_t sa_family;
       char        sa_data[14];
   }
   ```

   The only purpose of this structure is to cast the structure pointer passed 
   in addr in order to avoid compiler warnings.  See EXAMPLE below.

3. `address family`

   不同的`AF`规定了通讯的不同的域，即你在哪里去通讯，在使用`socket`函数创建sock
   时就需要在`domain`参数指定`socket family`

   ```c
   #include <sys/types.h>          /* See NOTES */
   #include <sys/socket.h>

   int socket(int domain, int type, int protocol);
   ```

   比如`AF_UNIX`或者`AF_LOCAL`用来进行本机中进程间通讯，`AF_INET`是IPV4的通讯，
   `AF_INET6`是IPV6的通讯。更多af可以查看`man 7 address_families`
   
   > `man 2 bind`中给的例子就是`AF_UNIX`进行IPC通讯的例子
