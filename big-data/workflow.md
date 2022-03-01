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

5. 对`tar.gz`文件进行解压，然后配置`JAVA_HOME`在`hadoop_root_dir/etc/hadoop-env.sh`中
   
   ```shell
   tar -xf your_tar_archive.tar.gz
   echo "export JAVA_HOME="your_java_home_path""
   ```

6. 配置伪分布，编辑`etc/hadoop/core-site.xml`和`etc/hadoop/hdfs-site.xml`
   
   ```
   <configuration>
       <property>
           <name>fs.defaultFS</name>
           <value>hdfs://localhost:9000</value>
       </property>
   </configuration>
   ```
   
   ```
   <configuration>
       <property>
           <name>dfs.replication</name>
           <value>1</value>
       </property>
   </configuration>
   ```

7. 确保本机可以无需密码`ssh localhost`
  
