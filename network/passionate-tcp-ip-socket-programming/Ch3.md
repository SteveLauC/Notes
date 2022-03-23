1. IPV4地址的大小是4个字节32bits，而IPV6的地址的大小是16个字节128bits。

2. 虽然端口号不允许重复，但TCP和UDP不会使用相同的端口号。比如，某个TCP
   使用了9190端口号，UDP也可以使用这个端口号。

3. 以前学计网时，书上说IP地址是用来在网络中标识计算机的。而端口号则是用
   来标识应用程序的。端口号={IP:PORT}，那假如一个应用程序创建多个socket，
   有多个端口号，那不就标志不了了吗。

3. 发现我们在写`socket`函数时第一个参数有使用`PF_INET`，又有时使用`AF_INET`.
   在Linux中，他们两个是等价的，但建议使用`AF_INET` 

4. 在使用IPV4时我们给sock绑定的地址:  

   ```c
   /* Structure describing an Internet socket address.  */
   struct sockaddr_in
   {
       __SOCKADDR_COMMON (sin_);
       in_port_t sin_port;         /* Port number.  */
       struct in_addr sin_addr;        /* Internet address.  */

       /* Pad to size of `struct sockaddr'.  */
       unsigned char sin_zero[sizeof (struct sockaddr)
                  - __SOCKADDR_COMMON_SIZE
                  - sizeof (in_port_t)
                  - sizeof (struct in_addr)];
   };
   ``` 

   其中的`in_port_t`是u16的类型，最后的一个字段`sin_zero`是用来填充到`struct
   sockaddr`的大小。

   其中的地址部分，则是使用了另一个结构体:  
   
   ```c
   /* Internet address.  */
   typedef uint32_t in_addr_t;
   struct in_addr
   {
       in_addr_t s_addr;
     
   };
   ``` 
   可以看到其中，也只是一个u32，所以IPV4的IP地址在计算机内部就是一个4字节的数。

   还有一个字段，也就是`address family`被藏起来了:  
   
   ```c
   /* This macro is used to declare the initial common members
   of the data types used for socket addresses, ```struct sockaddr',
   `struct sockaddr_in', `struct sockaddr_un', etc.  */

   #define __SOCKADDR_COMMON(sa_prefix) \
   sa_family_t sa_prefix##family
   ```

   ```c
   typedef unsigned short int sa_family_t;
   ```
    
   由于各种各样的`sockaddr`变体都有这个字段，所以他们使用宏来复用下。目前看不
   懂这个宏，但大致意思是有一个`sin_family`的字段，其类型是`sa_family_t`，也就
   是`u32`.

5. `sin_port`以及`sin_addr`其中存储字节的顺序都须是网络的端序，所以需要使用`htonx`
   函数来转换以下。

6. 关于`sockaddr`与其各种变体  
   bind函数的第二个参数使用的是`struct sockaddr *`，但由于地址的类型众多，所以统
   一使用了`sockaddr`，然后在`sockaddr_in`等其变体中填完信息，在强制类型转换过去。
  
   由于不同的协议传给bind函数的都是`sockaddr`类型的指针，那么则需要使用`sin_family`
   来指定不同的协议。

7. 网络中使用的byte order称为`network byte order`，规定为大端序。

8. 在网络和主机间转换字节序所使用的函数:  

   ```c
   #include <arpa/inet.h>

   uint32_t htonl(uint32_t hostlong);
   uint16_t htons(uint16_t hostshort);
   uint32_t ntohl(uint32_t netlong);
   uint16_t ntohs(uint16_t netshort);
   ```
   
   函数名称中的`h`代表`host`，而`n`代表着`network`，最后的`l`代表着`long`，`s`代表着
   `short`。
  
   比较诡异的一点，其中的两个名字中带有`l`的函数，其进行转化的类型却是`unsigned int`.
   可能是历史兼容问题，又或者`long`不是代表`long`类型，而是转换的函数中处理数据较长的
   那一个。
  
   不管怎么说，这两个函数目前来看，l的用来转换32bit的地址，s的用来转换16bit的端口号，
   蛮ok的。

9. 即使host是大端序的，也应该使用字节序转换函数，来写出可移植性更好的代码。

10. rust中的`{:x}/{:X}`也是有trait的，是`std::fmt::LowerHex/std::fmt::UpperHex`

11. 如果你想做的类型是类似c中`mode_t`那种用bit来标识的类型，可以使用一个库
    [bitflags](https://crates.io/crates/bitflags)
