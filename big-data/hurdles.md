1. 使用`systemctl`查看sshd状态失败，因为docker不是用systemd作为init进程的，使用
   `/etc/init.d/ssh start`来启动ssh

   [Docker System has not been booted with systemd as init system](https://stackoverflow.com/questions/59466250/docker-system-has-not-been-booted-with-systemd-as-init-system)

2. SLF4J: Class path contains multiple SLF4J bindings.
   删掉hadoop中的log jar包

3. hvie的cli无法启动，猜测是版本不兼容导致，换乘java8和hadoop3.2.2

4. hive的cli无法启动，guava不兼容
   

5. 装完hive没有初始化，直接去打开了hive的cli
   > Starting from Hive 2.1, we need to run the schematool command below as an initialization step. For example, we can use "derby" as db type. 
   
  [link](https://stackoverflow.com/questions/43947930/unable-to-initialize-hive-with-derby-from-brew-install)
