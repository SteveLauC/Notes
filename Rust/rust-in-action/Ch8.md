##### 8.2
1. 使用Rust发送http请求，我们使用`reqwest`这个库，但是这个库的最新版本已经是基于tokio的异步请求了，但其仍保留着早期版本的同步请求的
   api
   ```rust
   [dependencies]
   reqwest = {version="0.11.8",features = ["blocking"]} // 需要将blocking这个feature打开
   ```
   ```rust
   use std::error::Error;

   use reqwest;

   fn main() -> Result<(), Box<dyn Error>> {        
       let url = "target_url" 
	   let mut response = reqwest::blocking::get(url)?;

       let content = response.text()?; // 比较无语的是这个text函数会夺去response结构体的所有权。。。什么傻逼设计
       print!("{}", content);

       Ok(())
   }
   ```

2. 一个好的HTTP框架都为用户做了哪些对用户透明的事
   1. 何时断开连接
   2. 将传输过来的字节流转换为合适的类型
   3. 如果访问的是默认的80端口，自动帮我们隐藏端口号
   4. 我们发送请求时肯定给的是url，而不是ip，所以它需要帮我们解析为ip


3. rust的dynamic dispatch和static dispatch
   trait obj是动态分发，而范型是静态分发
   * 静态分发更加浪费磁盘，比如范型，在编译后其实是进行了展开
   * 动态分发更加费内存，会给runtime造成一点性能影响
   > 这是动态分发和静态分发的trade off
 
4. trait obj是DST，所以才需要包一层指针

5. trait obj有以下3种形式：
   1. &dyn Trait
   2. &mut dyn Trait
   3. Box<dyn Trait>
   > 其中一个比较显然的区别就是最后的box指针具有所有权
   > 在rust2018更新了trait obj的语法，要求其加上`&`，这是因为一个不加的话`&Trait`和`&Struct`太像了...

6. 使用rust的TCP协议来发送http请求
   ```rust
   use std::io::{prelude, Write};
   use std::net::TcpStream;



   fn main() -> std::io::Result<()> {
       let host = "www.rustinaction.com:80";
	   // let host = "35.185.44.232:443";
	   let mut conn = TcpStream::connect(host)?;
	   conn.write_all(b"GET / HTTPS/1.0")?;
	   conn.write_all(b"\r\n")?;
	   conn.write_all(b"Host: www.rustinaction.com")?;
	   conn.write_all(b"\r\n\r\n")?;

       std::io::copy(&mut conn, &mut std::io::stdout())?;
       Ok(())
   }
   ```
   几点需要注意的地方：
   1. TCP连接的双方是Socket组，也就是{IP:PORT}和{IP:PORT}，它并不知道域名这种东西，但在rust的`std::net::TcpStream`中，我们可以使用域
   名加端口号的方式。
   ```rust
   pub fn connect<A: ToSocketAddrs>(addr: A) -> Result<TcpStream>) // 函数的参数是范型+trait bound
   // 任何实现了`std::net::ToSocketAddrs`trait的类型均可以
   ```
   ```rust
   impl ToSocketAddrs for SocketAddr
   impl ToSocketAddrs for str                           // 这里可以传域名
   impl ToSocketAddrs for (&str, u16)
   impl ToSocketAddrs for (IpAddr, u16)
   impl ToSocketAddrs for (String, u16)
   impl ToSocketAddrs for (Ipv4Addr, u16)
   impl ToSocketAddrs for (Ipv6Addr, u16)
   impl ToSocketAddrs for String                        // 这里也是
   impl ToSocketAddrs for SocketAddrV4
   impl ToSocketAddrs for SocketAddrV6
   impl<'a> ToSocketAddrs for &'a [SocketAddr]
   impl<T: ToSocketAddrs + ?Sized> ToSocketAddrs for &T]
   // u16代表的是port number
   ```
   2. 在http报文格式中的`CRLF`是`\r\n`，两个`CRLF`代表着报文结束
   3. `std::io::copy()`的功能是Copies the entire contents of a reader into a writer.
   4. 然后就是write trait中的函数要写的一般都是slice，代表字节流，如果要将&str的字面量直接转为slice，直接在前面加b变为u8的数组，可以
   隐式转为slice.
   > 为什么array能变成slice
   ```rust
   impl<'a, T, const N: usize> TryFrom<&'a [T]> for &'a [T; N]
   impl<'a, T, const N: usize> TryFrom<&'a mut [T]> for &'a mut [T; N]
   ```
   注意这个usize的trait，这是关键
   rust中有4中类型转换，我该去看死灵书了


##### 8.4.1
1. 端口号是虚拟的，仅仅是u16的一个数字

2. DNS服务在进行查询域名时使用UDP协议，而在区域传输时使用TCP协议，端口号均为53.

// 错的，明明bind时就有端口号
// 3. 在rust中发送UDP报文，同样需要用到`std::net::SocketAddr`这个enum，但是与TCP源/目的主机都是完整的IP:PORT不同，UDP只需要目的主机
// 是完整的Socket，源主机只需要IP
   ```rust
   // NOTE: udp socket does NOT contain port number
   let localhost = UdpSocket::bind("0.0.0.0:0").expect("cannot bind to local socket");
   ```
   > 0.0.0.0:0 意味着监听所有的地址，端口由OS选择一个随机端口

3. `vec![0;512]`意味着生成一个vec，len和cap均为512，值全为0
   > 以前只知道数组可以这样做，才发现vecy也可以
   ```rust
   fn main() {
       let v = vec![0;3];
	   println!("{:?}", v);
   }
   // 打印[0,0,0]
   ```
