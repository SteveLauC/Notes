#### CH2

##### cat

1. cat命令的全称是concatenate，是用来拼接多个文件一起到stdout的命令.

2. cat可以在输出的时候压缩文件的多个空白行，使用 ```-s``` 参数即可

   ```shell
   root@2b916092bb53:/home/shell# cat multi_blanks 
   line1
   
   
   line2
   
   
   line3
   root@2b916092bb53:/home/shell# cat -s multi_blanks 
   line1
   
   line2
   
   line3
   root@2b916092bb53:/home/shell# 
   ```


##### tr命令
3. tr命令用于对stdin进行修饰再打印到标准输出 ```tr - translate or delete characters```

   比如在上面的测试中，将连续的换行符去掉至只剩一个，或者完全去掉换行符

   ```shell
    -d, --delete
                 delete characters in SET1, do not translate
   
    -s, --squeeze-repeats
                 replace each sequence of a repeated character that is listed in the last specified SET, with a single occurrence of that character
   ```

   ```shell
   root@2b916092bb53:/home/shell# cat multi_blanks |tr -s '\n'
   line1
   line2
   line3
   root@2b916092bb53:/home/shell# cat multi_blanks |tr -d '\n'
   line1line2line3root@2b916092bb53:/home/shell# 
   ```

   如上，使用 ```-s``` 参数将连续的 ```'\n'``` 换成一个，使用 ```-d``` 将所有的换行删除

   从输入中删除line

   ```shell
   line1line2line3root@2b916092bb53:/home/shell# cat multi_blanks |tr -d 'line'
   1
   
   
   2
   
   
   3
   ```

4. 显式地显示制表符，```-T``` 参数可以用于显式地将指标符替换为 ```^I``` 

   ```shell
   root@2b916092bb53:/home/shell# cat tabs 
   	1	2
   root@2b916092bb53:/home/shell# cat -T tabs
   ^I1^I2
   root@2b916092bb53:/home/shell# 
   ```

5. 给cat加上行号 ```-n``` 参数

##### script命令进行录屏

6. script命令，给你的终端进行录屏

   当输入script命令时，记录就开始了，输入exit使其结束。

   默认不给出参数时，会在当前路径下创建一个typescript文件，作为记录

   ```shell
   root@2b916092bb53:/home/shell# script  
   Script started, file is typescript
   # echo "hello to script"
   hello to script
   # exit
   Script done, file is typescript
   root@2b916092bb53:/home/shell# l
   total 16
   drwxr-xr-x 2 1000 1000 4096 Nov  8 08:30 .
   drwxr-xr-x 1 root root 4096 Oct 30 13:31 ..
   -rw-rw-r-- 1 1000 1000  179 Nov  8 07:57 test_shell_file
   -rw-r--r-- 1 root root  218 Nov  8 08:30 typescript
   root@2b916092bb53:/home/shell# cat typescript 
   Script started on 2021-11-08 08:30:40+00:00 [TERM="xterm" TTY="/dev/pts/1" COLUMNS="153" LINES="40"]
   # echo "hello to script"
   hello to script
   # exit
   
   Script done on 2021-11-08 08:30:50+00:00 [COMMAND_EXIT_CODE="0"]
   root@2b916092bb53:/home/shell# 
   ```

   当给出参数时，会在当前记录下创建名为此参数的记录，而不再是默认的typescript。然后上面的文件都只是文字记录，想要让其像录屏那样，则需要使用两个文件，一个记录时间信息，一个记录命令记录。

   ```script -ttime.log -a command.log``` 比如这条命令，使用 ```time.log``` 来记录时间信息，使用 ```command.log``` 来记录命令。当想要回放这个记录时仅需 ```scriptreplay  time.log command.log ``` 便可以对记录进行动态的复现。

   详细解释一下录制动态记录的这个命令：

   ​	-t参数用于指定时序记录的文件，本来这个参数的用法是：如果给出这个参数，则将时序记录输出到stderr，故可以对stderr重定向到文件中，但也可以参数后面直接跟文件名指定。

   ​	-a参数是对命令的记录，将记录追加到文件中。当然，如果不担心内容覆盖，不使用-a参数直接使用文件名```script -ttime.log command.log```也是可以的。

   ​	```scriptreplay``` 命令可以将记录回放，它需要时序记录，所以必须给两个文件作为参数，一个时间记录，一个命令记录。还有就是最好保证输出的终端的录制时的终端是一致的，环境不同不能保证播放的信息和录制时完全一样，比如两种环境对texcape字符的解释不同，那么在遇到escape字符时所输出的肯定就不是录制时的东西了。

##### find命令
7. find命令，列出当前目录及子目录下所有的文件和文件夹

   

   ```shell
   root@2b916092bb53:/home/shell# l
   total 24
   drwxr-xr-x 2 1000 1000 4096 Nov  8 08:49 .
   drwxr-xr-x 1 root root 4096 Oct 30 13:31 ..
   -rw-r--r-- 1 root root  216 Nov  8 08:40 command.log
   -rw-rw-r-- 1 1000 1000  179 Nov  8 07:57 test_shell_file
   -rw-r--r-- 1 root root  188 Nov  8 08:40 time.log
   -rw-r--r-- 1 root root  218 Nov  8 08:30 typescript
   root@2b916092bb53:/home/shell# find
   .
   ./time.log
   ./command.log
   ./test_shell_file
   ./typescript
   root@2b916092bb53:/home/shell# 
   ```

   ```find  path -print``` 对结果进行打印，使用换行符分隔， ```find path -print0``` 对结果进行打印使用 ```\0``` 进行分隔。

   

8. find命令匹配文件名

   ```find path -name "pattern string"```

   想忽略大小写则使用 ```-iname``` 参数

   例如，匹配当前目录及子目录下的pdf文件

   ```shell
   root@2b916092bb53:/home/shell# find . -name "*.pdf"
   ./a.pdf
   ```

   匹配当前目录及子目录的txt文件

   ```shell
   root@2b916092bb53:/home/shell# find . -name "*.txt"
   ./test_dir/inner.txt
   ./a.txt
   ./c.txt
   ```

    匹配当前目录下pdf或txt文件，```-o``` 表示or的意思

   ```
   root@2b916092bb53:/home/shell# find . -name "*.pdf" -o -name "*.txt"
   ./test_dir/inner.txt
   ./a.pdf
   ./a.txt
   ./c.txt
   ```

   ```shell
   # 或者将参数用括号扩起来，注意使用\将其进行转义
   # 一定要在内容和括号之间留空格出来。。。。！！！！！
   root@2b916092bb53:/home/shell# find . \( -name "*.pdf" -o -name "*.txt" \)
   ./test_dir/inner.txt
   ./a.pdf
   ./a.txt
   ./c.txt
   ```

9. -name参数是对文件名进行匹配，如果想对路径进行匹配的话，使用-path参数，同样的，对大小写不敏感的匹配使用 ```-ipath```

   比如，在当前路径及子路径中，生成的路径名中包含test的文件或文件夹

   ```shell
   root@2b916092bb53:/home/shell# find . -path "*test*"
   ./test_dir
   ./test_dir/inner.txt
   ./test_shell_file
   ```

   需要注意的是，它生成的待比较的路径名，是从你给的路径参数开始的，在上面的例子中，我们使用 ```.``` 来标识当前路径，则所待比较项均为.开头的路径。是绝对路径还是相对路径由你的path参数决定。

10. 使用正则表达式匹配路径名 ```-regex``` 参数

    不懂re

    很离谱这个参数名，是regex而不是regexp

11. 使用 ```!```可以将此条件取反，注意！和条件之间要有空格

    例如，列出不为pdf文件的文件

    ```shell
    root@2b916092bb53:/home/shell# find . -name "*.pdf"
    ./a.pdf
    root@2b916092bb53:/home/shell# find . ! -name "*.pdf"
    .
    ./test_dir
    ./test_dir/inner.txt
    ./time.log
    ./a.txt
    ./command.log
    ./test_shell_file
    ./c.txt
    ./typescript
    ```

12. 限制find命令递归的文件层数

    ```-maxdepth``` 和 ```-mindepth``` 参数

    这两个参数在提供的时候，应该在第三个参数的位置出现，使其先限制层数，减小待测试量。

13. find指定文件类型

    | 文件           | f    |
    | -------------- | ---- |
    | 目录           | d    |
    | 字符设备       | c    |
    | 块设备         | b    |
    | 符号连接软连接 | l    |
    | 套接子         | s    |
    | FIfo           | p    |

    > 什么是FIFO？
    >
    > ```
    >        A FIFO special file (a named pipe) is similar to a pipe, except
    >        that it is accessed as part of the filesystem.  It can be opened
    >        by multiple processes for reading or writing.  When processes are
    >        exchanging data via the FIFO, the kernel passes all data
    >        internally without writing it to the filesystem.  Thus, the FIFO
    >        special file has no contents on the filesystem; the filesystem
    >        entry merely serves as a reference point so that processes can
    >        access the pipe using a name in the filesystem.
    > ```
    >
    > FIFO又名命名管道，但它和pipe的区别在于它可以通过文件系统access到。虽然它是文件系统的一部分，但进程间通过它传递的内容并不会被写入文件系统

    例如，查看当前目录下的目录 ```find . -type d```

    查看当前目录下的普通文件 ```find . -type f```

    

14. find按时间进行搜索

    linux的时间有3种：

    ```
    -atime: access time 数据的访问时间，当一个文件被读了，其就是被访问了
    
    -mtime: modified time 数据的内容被修改时间，即被写
    当然如果你写一个文件，那么一定也读了它
    
    -ctime:  changed time 文件的元数据被修改的时间，元数据指的是文件类型，所有者，所属组，其他人的权限，就是14还是16bit代表的数据
    ```

    > UNIX/Linux并没有创建时间这个概念。

    使用ls命令查看这些时间特征：

    ```shell
    ls -l # modified time
    ls -lu # access time
    ls -lc # changed time
    ```

    atime mtime ctime这3个find参数是以天作为单位来使用的，-表示比x天短，+表示比x天长

    ```shell
    # 例如，列出一天内被写过的文件及被读的文件，由于写就一定读了，所以前者是后者的子集
    root@2b916092bb53:/home/shell# find . -mtime -1
    .
    ./test_dir
    ./test_dir/inner.txt
    ./a.pdf
    ./time.log
    ./a.txt
    ./c.txt
    root@2b916092bb53:/home/shell# find . -atime -1
    .
    ./test_dir
    ./test_dir/inner.txt
    ./a.pdf
    ./time.log
    ./a.txt
    ./command.log
    ./test_shell_file
    ./c.txt
    root@2b916092bb53:/home/shell# find . -mtime -1|wc
          7       7      69
    root@2b916092bb53:/home/shell# find . -atime -1|wc
          9       9     101
    root@2b916092bb53:/home/shell# 
    ```

    除了以天单位的搜索参数，find还提供了以分钟进行搜索的参数，amin,mmin, cmin，同样支持-+，用法相同

    ```shell
    # 找出10分钟内被写的文件
    root@2b916092bb53:/home/shell# find . -mmin -10
    .
    ./a.pdf
    ./a.txt
    ```

    

15. find按文件大小进行搜索
    -size参数可以搜索文件大小，且和时间一样，支持-/+表示比其大或小。使用方法是 ```find path -size 大小``` ，其中大小支持单位：

    * b block 块 512字节
    * c 字节？？？？？无语，用c表示字节
    * w 字 2字节
    * k KB
    * M MB
    * G GB

    ```shell
    # 我们可以使用dd命令来创建指定大小的空文件
    # 创建一个1MB的文件和1B的文件
    root@2b916092bb53:/home/shell# dd if=/dev/zero of=one_mb_file bs=1M count=1
    root@2b916092bb53:/home/shell# dd if=/dev/zero of=one_byte_file bs=1c count=1
    # 解释一下上面的命令，if代表input file, /dev/zero是一个特殊的设备文件，可以不断产生0
    # of代表output file，即我们要创建的文件，bs是文件大小，M=MB c=B count不懂
    ```

16. find命令的一些动作
    ```find . -print``` 的这个print其实就是动作，将其打印出来，但是你补给这个参数，它也会将其打印出来。

    find还有其他的一些动作，比如 ```-delete``` 用于将文件删除， ```-ls``` 用于将文件像 ````ls -dils``` 一样列出文件到标准输出，但我使用这个命令和 ```ls -dils``` ，输出不一样

    先给出一些用法

    ```shell
    # 删除所有f型文件
    find . -type f
    
    # 删除所有文件夹不可以使用 find . -type d
    # 因为它会把.这个特殊文件夹删掉，整个当前文件夹就直接空了
    
    # find . -ls 和 ls -dias的输出不同
    root@2b916092bb53:/home/shell# ls -dils
    396222 4 .
    root@2b916092bb53:/home/shell# find . -ls
       396222      4 drwxr-xr-x   2 1000     1000         4096 Nov 12 12:20 .
       397358      0 -rw-r--r--   1 root      root            0 Nov 12 12:20 ./txt
    root@2b916092bb53:/home/shell# 
    
    
    # 在这里解释一下ls -dils的参数
    # -d, --directory list directories themselves, not their contents
    # i就是inode号
    # -s, --size print the allocated size of each file, in blocks
    # -S     sort by file size, largest first
    ```

    

    除了ls和delete，print动作，find还有一个exec，用于执行其他命令

    ```shell
    比如删除当前路径下的txt文件
    find . -type f -name "*.txt" -exec rm {} \; # 要加这个分号
    # {}代表find检索出的文件
    ```

    ok动作和exec效果是一样的，不过它在做之前会问一下，比较安全。

    exec命令只能接受一个参数，也就是说它只能执行一个命令，不过我们可以让它执行shell脚本，然后 ```{ }``` 就会被作为参数传递进去。

    

​		

17. find使用权限和所有权匹配文件

    -perm 参数后来跟权限644就可以匹配权限为644的文件

    -user后跟uid或和username就可以匹配所有人

    ```shell
    # uid to username
    id -nu uid
    # username to uid
    id -u username
    ```

    

18.  删除当前目录下的所有文件夹

    ```shell
    # 假设文件夹均为空，可以轻松使用这条命令删除，但假如不为空，我们仍需使用rm
    find . -maxdepth 1 \( -type d \) -a \( ! -name "." \) -delete
    
    
    find .  -maxdepth 1 \( -type d \) -a \( ! -name "." \) -exec rm -r {} \;
    
    # 如果不给maxdepth参数的话，会报错，不知道原因
    ➜  shell_mount-point find .  \( -type d \) -a \( ! -name "." \) -exec rm -r {} \; 
    find: ‘./d1’: No such file or directory
    ➜  shell_mount-point 
    
    # 但从理论上讲，更深层次的文件夹必定在第一层的文件夹下面的，把第一层的文件夹删掉后，深层的肯定没有了
    ```
##### xargs命令

> 有些命令，只能通过命令行参数给出输入，而不可以通过stdin给出，这就对管道重定向造成了不便。而xargs命令就是将stdin转变为参数的一个命令，它会将stdin重新格式化，然后将其作为参数传递给其他命令。注意此命令必须跟在管道|后面。

19. 将多行输入换为单行输出

    xargs默认的使用命令是echo，所以你如果在xargs后给出任何命令，则其默认使用echo。

    同时，其会格式化，将stdin的换行换为空格，所以它可以将多行文件格式化为单行输出。

    ```shell
      shell_mount-point l
    Permissions Links Size User  Group Date Modified Name
    .rw-r--r--      1   12 steve steve 12 Nov 22:08  a
    ➜  shell_mount-point b a  
    ───────┬─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
           │ File: a
    ───────┼─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
       1   │ hello
       2   │ world
    ───────┴─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
    #  默认使用echo命令
    ➜  shell_mount-point cat a|xargs     
    hello world
    ➜  shell_mount-point cat a|xargs echo
    hello world
    ➜  shell_mount-point 
    ```

20. 将单行输入通过指定 每行的最大参数数量 将其划分为多行.

    注意它是先将换行换为空格，再按每行最多多少个输出。

    ```shell
    ➜  shell_mount-point l
    Permissions Links Size User  Group Date Modified Name
    .rw-r--r--      1   27 steve steve 12 Nov 22:16  a
    ➜  shell_mount-point cat a|xargs        
    1 2 3 4 5 6 7 8 9 10 11 12
    ➜  shell_mount-point cat a|xargs -n 2
    1 2
    3 4
    5 6
    7 8
    9 10
    11 12
    ➜  shell_mount-point b a           
    ───────┬─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
           │ File: a
    ───────┼─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
       1   │ 1 2 3 4 5 6
       2   │ 7 8 9 10 11 12
    ───────┴─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
    ➜  shell_mount-point 
    ```

21. 指定xrgs的分割符而不是使用默认的IFS

    ```shell
    ➜  shell_mount-point echo "oneXtwoXthree"|xargs -d 'X'
    one two three
    ```

    ```-0``` 使用 ```\0``` 作为定界符，在配合find命令删除文件时很好用。


19. 使用xargs将stdin转变为参数

    ```shell
    # 我们手写一个cecho来测试这种请况
    ➜  shell_mount-point b cecho.sh   
    ───────┬─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
           │ File: cecho.sh
    ───────┼─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
       1   │ #! /bin/bash
       2   │ 
       3   │ echo $* '#'
    ───────┴───────────────────────
    
    # 使用文件内容作为stdin，然后作为参数
    ➜  shell_mount-point b args.txt 
    ───────┬─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
           │ File: args.txt
    ───────┼─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
       1   │ arg1
       2   │ arg2
       3   │ arg3
    ───────┴─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
    
    # 将文件内容传递给cecho
    ➜  shell_mount-point cat args.txt|xargs ./cecho.sh                
    arg1 arg2 arg3
    ```

20. 有些命令的参数，需要使用 ```-a xxx``` 这种格式，而上面的命令显然做不到这一点，所以我们需要使用 xargs的-I参数，进行字符串替换

    ```shell
    ➜  shell_mount-point cat args.txt|xargs -I {} ./cecho.sh -p {} 
    -p arg1 #
    -p arg2 #
    -p arg3 #
    ➜  shell_mount-point 
    # -I后面的{} 为从stdin拿到的每一块内容，然后每一块内容都被替换到 ./cecho.sh -p {} 里
    # 但有点奇怪的是，它的每一个串，貌似是使用换行分割的，貌似我的IFS变量就是这个，所以大概是用IFS分割，然后逐个替换的
    ```

21. 和find命令配合删除文件

    在使用find和xargs进行删除文件时，一定要将其定界符均设为 ```\0``` ，因为我们不清楚文件名中有没有什么字符，所以需要指定一个不会在文件名中存在的，确保安全

    ```shell
    # 删除当前目录及子目录的所有txt文件
    find . -type f -name "*.txt" -print0 | xargs -0 rm -f
    
    # 在find命令中，我们使用了 -print0 来指定分割符号为 \0 
    # 在xargs命令中我们使用 -0来指定定界符为 \0
    ```

    > 刚才试了一下，使用find的-ok/-exec来删除过滤出的文件，貌似没有问题，即使文件名中有空格。

22. 统计源代码的行数，这个好玩

    统计文件的行数，使用 ```wc -l``` 命令，这个统计newline的行数，所以我们只需使用find过滤，然后通过xargs将其作为参数交给wc命令即可。

    ````shell
    # 统计c代码的行数
    find . -type f -name "*.c" -print0|xargs -0 wc -l
    
    # 还是一样的，我们使用\0作为分割符号
    
    # 这条命令的输出
    行数 文件名
    行数 总共
    ````

##### tr命令 tr - translate or delete characters

> tr命令的输入参数只能通过stdin输入，而不能通过参数，参数只能是一些规则，不是待处理的内容。
>
> 其默认用法是 tr set1 set2 ，对stdin中的内容按set1 -> set2 进行映射，然后输出。这里的集合不是严格的集合，并且它是有序的。而且两个集合的大小最好相同，然后就会按给定的顺序进行映射。
>
> 如果set1的大小大于set2的大小，set2会被其最后一个字符填充；
>
> 如果set1的大小小于set2的大小，set2多出的字符会被忽略。
>
> set1: a-c set2: d-e 那么set2会变成 d-f
>
> set1: a-c set2: d-g 那么set2会变成 d-f

23. 将一个小写英语单词转变为大写单词

    ```shell
    echo "hello world"|tr "a-z" "A-Z"
    ```

24. 23是tr进行替换的例子，其还可以进行删除

    进行删除时，仅需要给一个set，将stdin中位于set中的字符删掉

    ```shell
    # 删除在集合里的字符，如删除数字
    root@2b916092bb53:/home/shell# echo "1a2b3c4d"|tr -d '0-9'
    abcd
    root@2b916092bb53:/home/shell# 
    
    # 删除不在集合里的字符，使用补集 -c
    root@2b916092bb53:/home/shell# echo "1a2b3c4d"|tr -d -c '0-9'
    1234root@2b916092bb53:/home/shell# 
    # 只留下数字
    ```

25. 使用tr压缩字符，其还有一个常用用法就是对连续出现的字符进行压缩，如过出现大于2次，则压到1次

    ```shell
    # 对空格进行压缩
    1234root@2b916092bb53:/home/shell# echo "a   b   c"|tr -s ' '
    a b c
    root@2b916092bb53:/home/shell# 
    ```

26. 对一串数字进行计算，求和，使用tr将换行换为+，再使用$[]进行计算

    

