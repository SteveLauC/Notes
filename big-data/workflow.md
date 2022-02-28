1. 安装java，使用的是ubuntu默认的java，是最新的lts
   
   ```shell
   apt install default-jre
   ```
   
2. 安装ssh，并开启服务 

   ```shell
   apt install openssh-server
   /etc/init.d/ssh start
   ```

3. 创建一个非root用户，并为其创建密码，方便ssh登陆

   ```shell
   adduser steve   
   passwd steve
   ```

4. 下载hadoop，由于选择java11，所以我们的hadoop只能选3.3及以上
