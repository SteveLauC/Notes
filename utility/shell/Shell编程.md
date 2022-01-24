### Shell编程

#### 用户自定义变量

* 用户自己定义变量，像python一样不用写类型，直接 ```a=1``` ，但需注意，等号周围不能加空格。

* 定义的变量的默认类型都是字符串

  `````shell
  a=1+1
  echo $a # 1+1
  `````

* 撤销定义的变量，使用 ```unset``` 关键字

  ```shell
  a=1
  echo $a # 1
  unset a
  echo $a # 空
  ```

  但是 ```readonly``` 的变量不可以撤销：

  ````shell
  readonly a=1
  unset a
  # zsh: a variable: read_only
  ````

* 使某一变量成为全局变量

  ````shell
  a=1
  export a # a就成为了一个全局变量了，在当前shell解释器这个进程及其子进程中均可以使用。
  
  # 但想要真的，每打开一个shell，都有这个变量可以使用，需将其写入shell配置文件
  # 因为每次打开shell配置文件都会执行其配置文件。
  
  # 所以配置路径都是在shell配置文件中写 export home=""
  ````



#### 系统提供的特殊变量

