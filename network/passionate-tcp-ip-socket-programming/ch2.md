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
