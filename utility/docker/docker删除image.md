docker删除image

```sh
docker rmi image的名字/id  #rmi是remove image
docker rmi id/repository1 id/repository2 id/repository3 #连续删除多个
docker rmi $() #括号里的参数我们传id 
docker rmi $(docker image ls -aq)  #括号里是所有image的id 也就是删除所有的image
```

Docker run命令参数

```shell
docker run centos #这个命令只是会创建并启动一个容器
-d #在后台运行 
-p #指定特定的端口
-P #随机指定端口
-it #以交互的方式进行 
--name name #将容器的名字命名为name
```

docker ps命令参数

```shell
-n=? #可以查看最近新创建的几个容器
-q   #显示容器的时候只给出容器的id号Only display container IDs
```

退出容器但不令容器停止

```shell
ctrl+p+q #摁这3个键可以退出容器但不令容器停止
```

删除container

```shell
docker rm container_id #删除某一个没有在运行的容器
docker rm -f container_id #强制删除 故可以删除在运行中的容器
docker rm $(docker ps -aq) #删除所有的容器
```

后台启动命令

```shell
docker run -d ubuntu #后台启动ubuntu
#但你docker ps又看不到这个容器在跑 因为docker认为我们没有前台的任务直接启动完就stop了
#想要有前台任务的话
docker run -it -d ubuntu 
```

查看容器中的进程信息

```shell
docker top container_id
```

查看一个object的具体信息

```shell
docker inspect id #无论这个object是image还是container
```

进入当前正在运行的容器

```shell
docker exec -it container_id /bin/bash
docker attach -it container_id 
#两者的区别就在于 
#1. 当你在容器内部输入exit时容器会不会被直接stop
#前者不会被stop 而后者不会
#需要注意的是 docker exec命令至少需要两个参数 所以除了container_id 还要显示地给出/bin/bash

#2.进入容器后会不会打开一个新的终端
#exec是打开一个新的终端 attach是进入容器内正在执行任务的终端
```

从容器内拷贝文件到主机上

```shell
docker cp container_id:path des_path
#将容器内path路径上的文件拷贝到宿主机的des_path路径

#举例
docker cp 1c1e2d738779:/tmp /Users/steve/Desktop 
#就将容器1c1e2d738779根目录下的tmp文件夹拷贝到了我们电脑上的桌面上
```

docker run镜像但是用完就删掉

```shell
docker run -it --rm tomcat #以tomcat为例 
#一般用来测试 容器停掉就会被删掉
```

查看docker的容器所占用的cpu资源

```shell
docker stats #使用ctrl+c退出
```

将自己的容器打包成镜像

```shell
docker commit -m "描述信息" -a "author" container_id 想要成为的镜像名：tag版本号
```



docker挂载目录，将容器内的一个目录与本地目录联系起来，呈现一种**双向实时映射**的状态

即使容器停止了，也可以进行映射

```shell
docker run -v host_path: container_path
#需要注意的是container_path必须是绝对路径，而host_path不必。当host_path是相对路径的话，则会
#在/var/lib/docker/volumes/下创建你输入的名的目录。
#当只有container_path时(不用：了)，docker会在host上的/var/lib/docker/volumes/随机生成一个目录名
```

对于目录挂载的情况，可以通过inspect命令去查看，mounts下有详细信息



具名挂载和匿名挂载

 在-v的时候只写了容器内的路径，此时就是一个匿名挂载

在-v的时候不写host的路径，并在容器内路径前加上name:，那就创建了一个名为name的数据卷



如何区分是具名挂载 匿名挂载 还是指定路径挂载

```shell
-v container_path #匿名挂载
-v name:container_path #具名挂载
-v host_path:container_path #指定路径挂载
```



-v的权限参数

```shell
docker run -it -d  ..container_path:ro ubuntu #readonly 只得从host中更改文件，container内部不行
docker run -it -d  ..container_path:rw ubuntu #默认你不写这个参数它就是read write均可的
```



DokcerFile