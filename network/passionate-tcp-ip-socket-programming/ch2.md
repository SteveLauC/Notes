1. 在使用`socket`创建socket时第二个参数type:

   TCP almost always uses SOCK_STREAM and UDP uses SOCK_DGRAM.

   TCP (SOCK_STREAM) is a connection-based protocol. The connection is established
   and the two parties have a conversation until the connection is terminated by
   one of the parties or by a network error.

   UDP (SOCK_DGRAM) is a datagram-based protocol. You send one datagram and get 
   one reply and then the connection terminates.

       * If you send multiple packets, TCP promises to deliver them in order. UDP does not, so the receiver needs to check them, if the order matters.

       * If a TCP packet is lost, the sender can tell. Not so for UDP.

       * UDP datagrams are limited in size, from memory I think it is 512 bytes. TCP can send much bigger lumps than that.

       * TCP is a bit more robust and makes more checks. UDP is a shade lighter weight (less computer and network stress).

   Choose the protocol appropriate for how you want to interact with the other computer.

2. 套接字的使用必须是配套的，server和client使用的套接字必须是相同类型的。

3. 在使用`socket`函数的时候，第一个参数选择`PF_INET`来使用IPV4，第二个参数选择
   `SOCK_STREAM`来选择面向连接的协议，第三个参数选择`IPPROTO_TCP`来使用TCP协议。
   但由于前面两个参数如果这样选择的话，第三个参数只有TCP一个可以选择。所以就像
   文档所说的`Normally only a single protocol exists to support a particular 
   socket type within a given protocol family, in which case protocol can be 
   specified as 0.`，我们可以使用0来选择唯一的那一个协议。
  
   同样地，在使用`PF_INET`以及`SOCK_GRGAM`时，只有`IPPROTO_UDP`可以选，第三个
   参数同样可以只传入0
