##### 9.1
1. 世界上的两种时间：
   * TAI: 让每一秒的时长都是固定的，这对于计算机来说是准确的，但对于现实生活来说不是，因为潮汐阿，地球自传等各种因素，时间是动态的。
   * UTC: 每年都会随真实的物理情况对时间进行调整，这对人很友好，但对机器来说不友好。
   > 还有一种时间是GMT，以天文的现象规定时间，没有UTC准确。

2. 计算机当中一般有2种时钟：
   * one is a battery powered device, called the real-time clock.
   * the other one is known as system time.
   > raspberry pi中并没有第一种时钟。

3. crate level的lint要放在文件的头部
   ```rust
   line0: #[alow(unused_variables)]
   ```
4. 计算机中如何表示时间？
   一般是记录UTC时间，然后还有一个整数记录用户的时区，在UTC的基础上加上时区数的偏移量，就是用户的本地时间。

5. NTP(network time protocal)是一个应用层的协议，用来在计算机间同步时间。 
