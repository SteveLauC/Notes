
0. 配置好docker

   ```shell
   # host
   $ docker pull ubuntu
   $ docker run -it -d ubuntu
   CONTAINER ID   IMAGE     COMMAND   CREATED       STATUS       PORTS     NAMES
   16d1ab8df529   ubuntu    "bash"    2 hours ago   Up 2 hours             nervous_einstein
   $ docker exec -it 16 /bin/bash

   # container
   # apt install zsh
   ```

1. hadoop依赖
   ```shell
   # apt install ssh
   # apt install openjdk-8-jdk
   ```
2. 创建新用户，避免使用root
   ```shell
   # useradd steve
   ```

3. 使用steve账号登陆
   ```shell
   # su steve
   $
   ```

3. 配置ssh和java

   ```shell
   # 启动ssh进程
   $ /etc/init.d/ssh start
   # 确保ssh localhost可以免密登陆
   $ ssh-keygen -t rsa
   $ cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
   $ chmod og-wx ~/.ssh/authorized_keys 
   ```

   ```
   # ~/.zshrc
   # java环境变量
   export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
   export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH
   ```

4. 解压hadoop

   ```shell
   $ cd ~
   $ mkdir hadoop-root
   $ mv hadoop-3.2.2.tar.gz hadoop-root
   $ cd hadoop-root
   $ tar -xzvf hadoop-3.2.2.tar.gz
   ```
5. 配置hadoop环境变量

   ```
   # ~/.zshrc
   export HADOOP_HOME=/home/steve/hadoop-root/hadoop-3.2.2
   export PATH=/home/steve/hadoop-root/hadoop-3.2.2/bin:$PATH
   ```

5. 给hadoop配置java

   ```shell
   # $HADOOP_HOME/etc/hadoop/hadoop-env.sh
   export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
   ```

6. 配置伪分布

   ```
   # $HADOOP_HOME/etc/hadoop/core-site.xml
   <configuration>
      <property>
         <name>fs.defaultFS</name>
         <value>hdfs://localhost:9000</value>
      </property>
   </configuration>
   ```

   ```
   # $HADOOP_HOME/etc/hadoop/hdfs-site.xml
   <configuration>
      <property>
         <name>dfs.replication</name>
         <value>1</value>
      </property>
   </configuration>
   ```
   
7. 格式化文件系统并启动所有的节点

   ```shell
   $ hdfs namenode -format
   $ bash $HADOOP_HOME/sbin/start-all.sh 
   ```
8. 解压hive

   ```shell
   $ cd ~
   $ mkdir hive-root
   $ mv apache-hive-2.3.9-bin.tar.gz hive-root
   $ cd hive-root
   $ tar -xzvf apache-hive-2.3.9-bin.tar.gz
   ```

9. 配置hive环境变量

   ```
   # ~/.zshrc
   export HIVE_HOME=/home/steve/hive-root/apache-hive-2.3.9-bin
   export PATH=/home/steve/hive-root/apache-hive-2.3.9-bin/bin:$PATH
   ```

10. 为了运行hive的cli，需要在hdfs中创建以下文件

   ```shell
   $ hadoop fs -mkdir       /tmp
   $ hadoop fs -mkdir -p      /user/hive/warehouse
   $ hadoop fs -chmod g+w   /tmp
   $ hadoop fs -chmod g+w   /user/hive/warehouse
   ```

11. 由于我们使用的是的 embeded derby，每次执行hive命令都会在当前目录下创建
metastore_db和derby文件，所以单独设置一个文件夹用来执行hive命令

   ```shell
   $ mkdir $HIVE_HOME/hql

   # ~/.zshrc
   export HQL_PATH=$HIVE_HOME/hql
   ```
12. 对hive初始化

   ```shell
   schematool -dbType derby -initSchema
   ```

   错误: `SLF4J: Class path contains multiple SLF4J bindings.`问题

   删除掉hadoop中的log jar包

   ```shell
   $ rm -r  /home/steve/hadoop-root/hadoop-3.2.2/share/hadoop/common/lib/slf4j-log4j12-1.7.25.jar
   ```

   再尝试

   错误: Exception in thread "main" java.lang.NoSuchMethodError: com.google.common.base.Preconditions.checkArgument(ZLjava/lang/String;Ljava/lang/Object;)

   是由于`guava`冲突导致的

   删掉hive的这个jar包，然后用hadoop里的替换掉

   ```shell
   $ rm $HIVE_HOME/lib/guava-14.0.1.jar
   $ cp $HADOOP_HOME/share/hadoop/hdfs/lib/guava-27.0-jre.jar $HIVE_HOME/lib 
   ```

13. 上传数据到hdfs

   ```shell
   hdfs dfs -mkdir -p /data/good
   hdfs dfs -mkdir -p /data/sales_detail
   hdfs dfs -mkdir -p /data/sales_head
   hdfs dfs -put /home/steve/data/good/good.csv  /data/good
   hdfs dfs -put /home/steve/data/sales_detail/sales_detail.csv /data/sales_detail
   hdfs dfs -put /home/steve/data/sales_head/sales_head.csv /data/sales_head
   ```

14. 数据库的创建以及查询

   ```sql
   # 数据库创建脚本
   # db_init.hql
   create external table if not exists retail_good (
        GoodId string, Category1Name string, Category2Name string, Category3Name string, Category4Name string, BrandName string, GoodName string)
        row format delimited
        fields terminated by '\t'
        stored as textfile
        location '/data/good';


   create external table if not exists retail_sales_head (
         BillId string, CustId string, Quantity int, Transtime string, OrigTotalPrice double, Pay double, Change double, ActualTotalPrice double)
         row format delimited
         fields terminated by '\t'
         stored as textfile
         location '/data/sales_head';

   create external table if not exists retail_sales_detail (
         BillId string, RowNo int, TransTime string, GoodId string, BarCode string, GoodName string, Unit string, Quantity int, OrigUnitPrice double, OrigTotalPrice double, ActualUnitPrice double, ActualTotalPrice double)
         row format delimited
         fields terminated by '\t'
         stored as textfile
         location '/data/sales_detail';
   ```

   ```sql
   # 数据库查询脚本

   # query-a.sql
   select substr(TransTime, 1, 13) as trans_time, sum(ActualTotalPrice) as trans_sales, count(*) as trans_num
   from retail_sales_head
   group by substr(TransTime, 1, 13);


   # query-b.sql
   select substr(TransTime, 1, 10) as trans_time, sum(ActualTotalPrice) as trans_sales,  count(*) as trans_num
   from retail_sales_head
   group by substr(TransTime, 1, 10);

   # query-c.sql
   select substr(TransTime, 1, 10) as trans_time, Category1Name as trans_categ, sum(ActualTotalPrice) as trans_sales,  count(*) as trans_num
   from retail_sales_detail, retail_good
   where retail_sales_detail.GoodId = retail_good.GoodId
   group by substr(TransTime, 1, 10), retail_good.Category1Name;

   # query-d.sql
   select CustId, min(round((unix_timestamp('2013-08-01 00:00:00') - unix_timestamp(TransTime)) / 3600 / 24)) as trans_interval, count(CustId) as trans_freq, sum(ActualTotalPrice) as trans_sales
   from retail_sales_head
   where CustId is not null
   group by CustId;
   ```


   开始创建数据库并进行查询
   ```shell
   $ cd $HQL_PATH
   $ hive -f db_init.hql
   ```
   ```shell
   hive -f query-a.hql 2>/dev/null > a.output
   hive -f query-b.hql 2>/dev/null > b.output
   hive -f query-c.hql 2>/dev/null > c.output
   hive -f query-d.hql 2>/dev/null > d.output
   ```

   ```shell 
   -rw-rw-r--  1 steve steve 169931 Apr 26 21:42 a.output
   -rw-rw-r--  1 steve steve  12535 Apr 26 21:40 b.output
   -rw-rw-r--  1 steve steve 282598 Apr 26 21:43 c.output
   -rw-rw-r--  1 steve steve 615901 Apr 26 21:43 d.output