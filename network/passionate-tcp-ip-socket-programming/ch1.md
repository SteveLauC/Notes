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
   `AF_INET6`是IPV6的通讯。更多af可以查看`man 2 socket/man 7 address_families`
   
   > `man 2 bind`中给的例子就是`AF_UNIX`进行IPC通讯的例子

4. `socket type`
   `socket`函数的第2个参数是类型，有很多，下面摘抄2个: 

   ```
   // TCP
   SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.

   // UDP
   SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
   ```

5. protocol
   
   The  protocol specifies a particular protocol to be used with the socket. *Normally
   only a single protocol exists to support a particular socket type within a given 
   protocol family, in which case protocol can be specified as 0. *However, it is 
   possible that many protocols may exist, in which case a particular protocol must
   be specified in this manner.  The protocol number to use  is  specific  to the 
   “communication domain” in which communication is to take place; see protocols(5).
   See getprotoent(3) on how to map protocol name strings to protocol numbers.

   在书中的`hello_server`中我们使用`IPV4/TCP`，然后最后一个`protocol`参数给0.

6. `INADDR_ANY`

   当在`bind`时，我们准备`bind`的地址是`INADDR_ANY`的时候，它代表着所有可以用的interface
   这个量的值为0，翻译为IPV4的地址即是`0.0.0.0`.
   
   > A way to specify "any IPv4 address at all". It is used in this way when configuring 
   servers (i.e. when binding listening sockets). This is known to TCP programmers as 
   INADDR_ANY. (bind(2) binds to addresses, not interfaces.)
  
   > In the context of servers, 0.0.0.0 can mean "all IPv4 addresses on the local machine". 
   If a host has two IP addresses, 192.168.1.1 and 10.1.2.1, and a server running on the 
   host is configured to listen on 0.0.0.0, it will be reachable at both of those IP addresses. 

7. `int listen(int sockfd, int backlog)`函数

   ```c
   #include <sys/types.h>          /* See NOTES */
   #include <sys/socket.h>

   int listen(int sockfd, int backlog);
   ```

   listen() marks the socket referred to by sockfd as a passive socket, that is,
   as a socket that will be used to accept incoming connection requests using accept(2).

   The sockfd argument is a file descriptor that refers to a socket of type 
   SOCK_STREAM or SOCK_SEQPACKET.
   注意下这个socket fd其中的类型一定要是这两种，UDP没有这个资格了

   The  backlog argument defines the maximum length to which the queue of pending
   connections for sockfd may grow.  If a connection request arrives when the 
   queue is full, the client may receive an error with an indication of ECONNREFUSED 
   or, if the underlying protocol supports retransmission, the request may be 
   ignored so that a later reattempt at connection succeeds.

8. server side的流程就是:
   1. 使用`socket`函数来创建socket，拿到fd
   2. 准备好socket address
   3. 将socket address bind到fd上
   4. 对fd调用listen，使其进入准备状态
   5. 准备好client的socket address和socket fd
   6. 调用accept函数，初始化client side的sock fd和sock address
   7. 对client fd 进行write之类的行为
   
   (2个fd，2个socket address)

9. 在client这边，比较重要的函数是`int connect(int sockfd, const struct sockaddr 
   *addr, socklen_t addrlen);`，我们需要准备待访问的server的address，然后将client
   的fd，连接到serer address上
   
   client side的流程: 
   1. 调用socket创建client fd
   2. 初始化server address
   3. 调用connect连接client fd到server address
   (1个client fd，1个server address)
   4. 然后从client fd读东西

   > 这个client fd代表着一个socket文件，只有当client这边调用了connect，accept才
   会拿到client的fd。就好像钓鱼一样，鱼上钩了，一抬竿，才能抓到鱼

10. 将server和client的流程合起来看的话: 
    1. server拿到socket
    2. server bind
    3. server listen
    4. server accept
    5. client connect 
    6. server向client fd写
    7. client从client fd读
