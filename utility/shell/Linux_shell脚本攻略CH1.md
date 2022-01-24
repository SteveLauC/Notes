#### CH1

1. 想要关闭bash的history功能，可以使用```unset history``` 

2. 在bash中，每个命令的分隔是靠;或换行符实现的

3. shell脚本的解释器，```#!``` 被称之为shebang，she指代sharp - #， bang指代!

4. echo的打印后会自动添加换行

5. ```!``` 对于bash来说是一个特殊符号，想要```echo "hello!"``` 应该把双引号换为单引号，或者使用 ```\```来对它进行转义

   > 但在我的docker容器里，貌似history是默认关闭的，所以双引号也可以打印。
   >
   > 而在我的host里，shell是zsh，不可以打印
   >
   > ```shell
   > ➜  shell_mount-point echo "hello!" 
   > dquote> 
   > ```

6. echo时使用 ```' '``` 将环境变量扩起来，shell不会对其解释，仅仅是原样输出

   ```shell
   steve@64ac17e28136:~$ echo '$PATH'
   $PATH
   echo "$PATH"
   /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
   ```

7. printf也可以用来进行打印，和c语言的很像。和echo不同的是，并不会在打印后添加换行

   好处就是它可以用来格式化打印，比echo更归整

   ```shell
   steve@64ac17e28136:~$ printf "hello %d %c\n" 5 c
   hello 5 c
   ```

8. echo命令的转义功能默认是关闭的，想要打开需要使用```-e``` 参数

9. 使用env命令可以查看与此终端相关的环境变量

   ```shell
   steve@64ac17e28136:~$ env
   SHELL=/bin/bash
   HOSTNAME=64ac17e28136
   PWD=/home/steve
   LOGNAME=steve
   HOME=/home/steve
   LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=00:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.zst=01;31:*.tzst=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.wim=01;31:*.swm=01;31:*.dwm=01;31:*.esd=01;31:*.jpg=01;35:*.jpeg=01;35:*.mjpg=01;35:*.mjpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=00;36:*.au=00;36:*.flac=00;36:*.m4a=00;36:*.mid=00;36:*.midi=00;36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.oga=00;36:*.opus=00;36:*.spx=00;36:*.xspf=00;36:
   TERM=xterm
   USER=steve
   SHLVL=2
   PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin
   MAIL=/var/mail/steve
   OLDPWD=/home/steve/shell_par
   _=/usr/bin/env
   ```

10. 创建一个变量，并声明其为一个环境变量

    ``````shell
    var=value
    export var
    
    # export var=value
    # 环境变量就是可以给自己以及任何当前进程的子进程用的变量
    # 但这种环境变量是暂时的，想要将其永久化，需要写入配置文件，如bashrc或zshrc中
    ``````

11. 查看一个变量的长度

    ````shell
    var=value
    echo ${#var} # 5
    ````

12. 查看当前的shell

    ```shell
    echo $SHELL
    echo $0
    ```

13.  在shell脚本中检查执行此脚本的用户是否是root，通过UID实现，root的UID为0

    ```shell
    #! /bin/bash
    if [$UID -ne 0 ]; then
    	echo "you are not a root user"
    else
    	echo "you are root"
    fi	
    ```

    ```shell
    #! /bin/bash
    if [ $UID = 0 ]; then # []左右要有空格
    	echo "root"
    else
    	echo "not root"
    fi	
    ```

14. 修改shell的prompt，可以通过修改PS1变量可以做到。但想要永久修改，应该在bashrc中改这个

    ```shell
    $PS1='\e[1;31m[\u@\h \W]\$\e[0m' # 可以提示符改为红色的
    # \u会被扩展为用户名
    # \h会被扩展为主机名
    # \w会被扩展为当前工作目录
    ```

15. let 命令可以执行基本的算术操作，使用let时不需要添加$

    ```shell
    n1=1
    n2=2
    let res=n1+n2
    echo $res
    ```

16. shell可以进行 ```++/--```操作，和C语言一样

    ```shell
    n1=1
    let res=n1--
    
    echo $res # 1
    echo $n1  # 0
    ```

17. 如果不使用let，也可以使用$[]将表达式进行计算，[]里面的变量即可以加$，也可以不加

    ```shell
    n1=1
    res=$[n1+1]  # []中的变量不加$
    res=$[$n1+1] # []中的变量加$
    ```

18. 也可以使用$(())进行运算

    ```shell
    n1=1
    n2=2
    res=$((n1--))
    
    echo $res
    echo $n1
    ```

19. expr也可以用于算术计算

20. let $[] $(()) expr仅可以进行整数运算

    可以使用bc这个计算器，来计算浮点数

    ```shell
    echo "4*0.56"|bc
    
    no=54
    result=`echo "$no*1.5" | bc`   # 在shell脚本中执行其他的命令，使用``包裹
    echo $result
    ```

21. 给bc计算器传递些参数

    * 制定输出的位数 scale参数

      ```shell
      [steve@64ac17e28136 shell_par]$echo "scale=2;3/8"|bc
      .37
      ```

    * 指定输出的数的进制 obase参数

       ```shell
       [steve@64ac17e28136 shell_par]$echo "obase=2;100"|bc
       1100100
       ```

    * 指定输入的数的进制 ibase 也就是如何看待输入数的进制

      ```shell
      [steve@64ac17e28136 shell_par]$echo "ibase=2;111"|bc
      7  # 所以可以看到默认的输出进制为10
      ```

    * 可以看到传给bc的参数是通过 ```;``` 和待计算表达式分割开来，而且待计算表达式需要在参数后面

22. 重定向符号，重定向默认是将stdout重定向到别的地方，stdout的文件描述符是1，所以当使用 > / >> 时，其相当于

    ```shell
    echo "new line" > temp.txt # => echo "new line" 1> temp.txt
    cat temp.txt # new line
    ```

23. 重定向stderr到文件里 

    > 应该叫将错误信息输出到error_file里，stdout没有被重定向，还是打印到屏幕
    
    ```shell
    [steve@64ac17e28136 shell_par]$touch error_file
    [steve@64ac17e28136 shell_par]$echo "error" 2> error_file 
    error
    [steve@64ac17e28136 shell_par]$cat error_file  # 什么也没有
    ```
    
24. 当一个进程成功退出时，它会返会0；未成功退出，则会返回非0值，我们可以通过 ```echo $?``` 来查看上一个进程的退出状态

    ```shell
    steve@64ac17e28136:~/shell_par$ ls not_a_file
    ls: cannot access 'not_a_file': No such file or directory
    steve@64ac17e28136:~/shell_par$ echo $?
    2
    steve@64ac17e28136:~/shell_par$ ls
    bc.sh  error_file  hello.sh  is_root.sh  math.sh  temp.txt  variables.sh
    steve@64ac17e28136:~/shell_par$ echo $?
    0
    ```

25. 如果有时不想被错误信息干扰，可以将stderr重定向到/dev/null这个文件里，这文件是一个无底洞哈哈哈

26. 如果有一个命令的stdout，既想打印到终端，又想重定向到文件里，可以使用tee命令

    > 试了一下，对于stderr不行，仅可以使用stdout

    ```shell
    steve@64ac17e28136:~/test_file$ echo "will appear in two places" | tee copy_of_stdout
    will appear in two places
    steve@64ac17e28136:~/test_file$ cat copy_of_stdout 
    will appear in two places
    steve@64ac17e28136:~/test_file$ echo "only in the file" | cat > copy_of_stdout 
    steve@64ac17e28136:~/test_file$ cat copy_of_stdout 
    only in the file
    steve@64ac17e28136:~/test_file$ 
    ```

    通常情况下，tee命令会将文件覆盖，它提供了 ```-a``` 的选项，可以让我们对文件进行追加

    ```shell
    steve@64ac17e28136:~/test_file$ echo "first line" > file
    steve@64ac17e28136:~/test_file$ echo "second line" | tee -a file
    second line
    steve@64ac17e28136:~/test_file$ cat file
    first line
    second line
    steve@64ac17e28136:~/test_file$ 
    ```

27. stdin stdout也是/dev下面的文件，所以我们也可以使用他们来显式地重定向

28. bash中有两种数组，一种是普通数组，一种是关联数组，也就是map

29. 普通数组

    ```shell
    #! /bin/bash
    
    # shell中的数组是用()包裹 空字符隔开的
    array=(1 2 3)
    
    # 打印数组的第1个元素
    echo ${array[0]}
    
    # 遍历打印数组
    echo ${array[*]}
    
    # 看下数组越界会怎么样 什么也不会打印 只有echo自带的换行
    echo ${array[3]}
    
    
    # 可以自由地向数组里插入数据 而且没有越界的问题 想插哪里插哪里
    array[9]=9 # 明明只有3个元素，却可以插入到9那里
    echo ${array[*]}
    
    # 数组中的元素类型 也可以不同
    array[0]="steve"
    echo ${array[0]}
    
    # 打印数组长度
    # 有几个元素打印几个 无关元素插入的index
    echo ${array[*]}
    echo ${#array[*]} 
    
    # 删除元素
    unset array[0] # 删除0号索引的元素
    
    # 删除整个数组
    unset array
    ```

30. 关联数组

    > bash中的关联数组是从4.0版本开始引入的

    ```shell
    #! /bin/bash
    
    # 使用declare -A参数可以将一个变量声明为动态数组 也就是创建动态数组
    declare -A dyn_array
    # 也可以一下子初始化 这样子或将之前的存在kv覆盖掉，而不是将其追加
    dyn_array=(["key"]="value" ["another_key"]="another_value")
    # 可以一个一个地插入 只有插入才可以追加
    dyn_array["steve"]="lau"
    
    
    # 查看数组中所有的value
    echo  ${dyn_array[*]}
    # 查看数组中的所有key
    echo ${!dyn_array[*]}
    # 查看某一个k的value
    echo ${dyn_array["key"]}
    
    # 可以使用unset删除元素
    unset dyn_array["steve"] # 删除key为 "steve" 的value
    
    # 删除整个数组
    unset dyn_array
    ```

31. 在使用alias创建别名时，如果此别名已经给别人了，那么将会被覆盖调

    ```shell
    oot@2b916092bb53:/home/shell# alias me="steve"
    root@2b916092bb53:/home/shell# type me
    me is aliased to `steve'
    root@2b916092bb53:/home/shell# alias me="lau"
    root@2b916092bb53:/home/shell# type me
    me is aliased to `lau'
    root@2b916092bb53:/home/shell# 
    ```

32. 有时使用别名是很方便的，但有时我们不想使用别名，而是使用此命令原本的意思

    ```shell
    # ll is aliased to `ls -alF'
    # 所以使用 ll 可以列出文件
    # 但我们也可以不使用这个命令 而仅仅使用原本的ll
    oot@2b916092bb53:/home/shell# ll
    total 16
    drwxr-xr-x 2 1000 1000 4096 Oct 31 02:51 ./
    drwxr-xr-x 1 root root 4096 Oct 30 13:31 ../
    -rw-r--r-- 1 root root   16 Oct 31 02:51 file
    -rw-rw-r-- 1 1000 1000  530 Oct 31 02:48 test_shell_file
    root@2b916092bb53:/home/shell# \ll
    bash: ll: command not found
    root@2b916092bb53:/home/shell# 
    ```

    > 这个功能在什么时候有用呢？假设我们的ls命令被别人恶搞改为了rm，那么再使用ls就很危险了，这时需要使用\ls来使用其原本的功能。

33. tput命令，可以用来或许一些终端信息

    ```
    #! /bin/bash
    # 这些东西在bash里是可以的，但在zsh里试了一下 不行
    
    # 打印列数
    tput cols
    # echo $COLUMNS 可以做到同样的事
    
    # 打印行数
    tput lines
    # echo $LINES   可以做到同样的事
    
    # 打印当前的终端名
    tput longname 
    
    # 移动光标到 (100, 100)
    tput cup 100 100 # 可以用来快速移动光标
    
    # 改变终端的背景颜色
    tput setb no
    # no [0,7] 
    # 0 黑灰色
    # 1 蓝色
    # 2 绿色
    # 3 浅蓝色
    # 4 大红色
    # 5 浅紫色
    # 6 暗黄色
    # 7 水泥灰色
    
    # 设置终端字体为粗体
    tput bold
    ```

34. 计算机内的时间被存储为一个整数，其大小是从标准时间1970年1月1日0时0分0秒起流逝的秒数，这个时刻被称为纪元时或UNIX时间或POSIX时间

    ```shell
    # 在shell中我们可以使用date民林拿到当前的时间
    root@2b916092bb53:/home/shell# date
    Sun Oct 31 03:22:05 UTC 2021
    root@2b916092bb53:/home/shell# 
    ```

    其中date命令输出的格式可以给 ```+%特殊字符``` 进行设置，比如 ```date +%s``` 打印出当前时间距离纪元时的秒数.

    其他的一些格式控制符号可以通过 ```man date``` 查看一下

    ```shell
    root@2b916092bb53:/home/shell# date +%s
    1635650757
    root@2b916092bb53:/home/shell# 
    ```

35. 计算脚本执行所花费的时间

    ```
    #! /bin/bash
    
    # 计算脚本执行所花费的时间
    start=$(date +%s)
    
    echo "running..."
    sleep 2s
    
    
    end=$(date +%s)
    
    dif=$((end-start))
    echo "the process takes $dif seconds"
    ```

36. tput的一些操作

    ```shell
    tput clear # 清屏
    tput sc # 保存当前光标位置
    tput cup 10 13 # 将光标移动到 row col
    tput civis # 光标不可见
    tput cnorm # 光标可见
    tput rc # 显示输出
    exit 0
    ```

37. clear/tput clear和ctrl+l的区别，前两者会清空buffer，也就是上滚查看输入命令的buffer，而ctrl+l不会

38. ```bash -x``` 可以进行  ```Print commands and their arguments as they are executed.``` ，可以进行debug

39. bash的for in循环

    ````shell
    for i in {1..6}
    do
    echo hello $i
    done
    echo "script executed"
    
    hello 1
    hello 2
    hello 3
    hello 4
    hello 5
    hello 6
    script executed
    ````

    > 注意它的 1..6 是6次循环，也就是和Rust不同的，右边也是闭区间

40. bash的函数

    ```shell
    #! /bin/bash
    
    function foo(){
        echo hello world
    }
    
    # 或者定义为下面这种形式
    foo(){
    	echo hello world
    }
    
    foo
    
    
    # hello world
    ```

    > 注意它的调用不需要加括号，这个许多高级语言不同

    ```shell
    sum(){
        i=0
        i=$((i+$1))
        i=$((i+$2))
        echo $i
    }
    
    sum 1 2
    
    # 可以发现虽然传入了参数，但并没有形参
    # $1指的是第一个参数 $2指的是第二个参数 $n指第n个参数
    ```

    ```shell
    # 可以用$@或$#来指代所有参数
    echo $@  # 将这些参数分别打印出来，打印$#个东西
    ！echo $#  # 将这些参数拼成一个字符串，打印出来，打印一个东西 貌似写错了 是打印参数个数
    ```

41. shell的脚本的参数和函数的参数类似

42. shell的函数一样可以导出，这样就可以将其扩展到子进程中

43. ```shell
    root@2b916092bb53:/home/shell# l
    total 12
    drwxr-xr-x 2 1000 1000 4096 Nov  2 02:59 .
    drwxr-xr-x 1 root root 4096 Oct 30 13:31 ..
    -rw-rw-r-- 1 1000 1000   74 Nov  2 02:56 test_shell_file
    
    # 很诡异的点 在执行这条命令时并没有out.txt文件
    # 所以它是先创建了out.txt
    root@2b916092bb53:/home/shell# ls|cat -n > out.txt
    # 还有一个很诡异的点，ls的stdout不同的文件名被加了换行
    # 试了试 exa也会这样
    
    root@2b916092bb53:/home/shell# cat out.txt 
         1	out.txt
         2	test_shell_file
    root@2b916092bb53:/home/shell# l
    total 16
    drwxr-xr-x 2 1000 1000 4096 Nov  2 02:59 .
    drwxr-xr-x 1 root root 4096 Oct 30 13:31 ..
    -rw-r--r-- 1 root root   38 Nov  2 02:59 out.txt
    -rw-rw-r-- 1 1000 1000   74 Nov  2 02:56 test_shell_file
    root@2b916092bb53:/home/shell# 
    ```

44. 用一个变量接受另一个命令的stdout

    ```shell
    out=$(COMMAND) # 这个方法被称为**子shell**
    out=`COMMAND`  # 这个方法被成为反引用
    echo $out
    
    # 但是这种方法会自动被去掉换行和空格
    echo -e "1\n2 2\n3" > file
    echo "先来看一个文件的内容"
    cat file
    out=`cat file`
    echo "再来看下将stdout交给一个变量后，其输出"
    echo $out
    echo "可以发现shell自动帮我们去掉了换行和空格"
    
    # 想要不被去掉换行和空格，需要使用""将$(COMMAND)/`COMMAND`包裹起来
    # 然后我试了试，不行
    # 紧接着我又拿zsh试了一下，发现zsh加不加"都可以保留空格和换行
    ```

45. 子shell是建了一个独立的子进程，这个子进程在不对其进行修改时和父进程的内容相同，和父进程不是一个东西

    ```shell
    (
        echo "我们在子进程中"
        cd /;pwd
    )
    echo "我们在父进程中"
    pwd
    
    echo "可以看到两个进程的当前工作路径不是一个"
    ```

46. read命令，这是一个shell内建的命令

    ```shell
    # 读入3个字符到var中 不需要回车来确认
    read -n 3 var
    
    # 读取时关闭回显 尤其在读密码的时候
    read -s pw
    
    # 显示提示信息
    read -p "please input:" var
    
    # 在特定的时间内读取输入
    read -t timeout var
    # 例如在2s内完成输入
    read -t 2 var
    
    # 确定定界符，当输入到定界符，结束输入，也不需要回车确认
    read -d "." var
    # 假定input为 "a." 那么var为a
    ```

47. IFS(internal field separator)

    > 这是一个种分割符，用来分割字符串

    ```shell
    data="name:sex:rollno:location"
    oldIFS=$IFS
    IFS=:
    for item in $data;
    do
    echo Item: $item
    done
    
    IFS=$oldIFS
    ```

    > 比如上面这个代码，想要分割data字符串，发现它是靠":"分割的，所以我们先把IFS设置为:，然后就可以对data使用for  in循环来遍历它。如果在data中并没有找到分割符，比如我们设置IFS为"."，那么整个字符串会被当作一个东西被打印出来。

48. shell中的循环

    shell提供了for in循环，也提供了c语言样式的 ```for((i=1;i < 3;i++){command}``` ，这种for循环不需要do done。然后还有while循环和特殊的until循环

    ```shell
    #! /bin/bash
    
    for i in {0..3};
    do
        echo $i;
    done;
    
    for ((i=0;i<=3;i++)){
        echo $i;
    }
    
    
    # -le 是lower than or equal
    i=0
    while [ $i  -le 3 ]; do
        echo $i;
        let i++;
    done
    
    
    # 当i==4时才会停下来
    i=0
    until [ $i -eq 4 ]; do
        echo $i;
        let i++;
    done
    ```

49. shell的复杂一点的逻辑表达式

    ```shell
    #! /bin/bash
    
    i=1
    j=1
    if [ $i -eq 1 -a $j -eq 1 ]; then
        echo "command"
    fi
    
    
    if [ $i -eq 1 ] && [ $j -eq 1 ];then
        echo 'command'
    fi
    ```

    > 两个逻辑表达式要想通过 与 或 连接起来，要不然就写在两个[]里；如果要写在一个[]中，那么and 就要写成-a，or就要写成-o

    ```shell
    -gt # 大于
    -lt # 小于
    -ge # 大于等于
    -le # 小于等于
    ```

    ```
    # 与文件类型有关的条件判断
    -f $var # 如果var是一个文件而非文件夹，则返回真
    -d $var # 如果var是一个文件夹，则返回真
    -c $var # 判断是char型设备文件
    -b $var # 判断是block设备文件
    -L $var # 判断是软链接 符号连接
    
    # 与文件权限有关的条件判断
    -r $var # 可读
    -w $var # 可写
    -x $var # 可执行
    
    # 文件存在
    -e $var 
    ```

    ```shell
    # 字符串比较
    #! /bin/bash
    
    str1="str"
    str2="str"
    
    # 判断两个字符串相同
    if [ $str = $str ]; then
        echo 'eq'
    fi
    
    
    if [ $str1 == $str2 ];then
        echo 'eq'
    fi
    
    # 判断两个字符串不同
    if [ $str1 != $str2 ];then
        echo "not eq"
    fi
    
    # 两个字符串还可以比大小 就像c语言的strcmp函数一样
    str1='sA' # A: 65
    str2='sa' # a: 97
    
    if [[ $str1 < $str2 ]];then
        echo "smaller than"
    fi
    
    
    # 判断字符串为空或非空
    str=''
    if [[ -z $str ]];then
        echo "str is empty"
    elif [[ -n $str ]];then
        echo "str is not empty"
    fi
    ```

    ```shell
    # if的无括号写法
    if [condition]; then
    	|
    if test condition; then	
    ```

    
