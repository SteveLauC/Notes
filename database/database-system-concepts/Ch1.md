1. 使用文件进行存储的一大弊端就是，针对你的需求，你会创建不同的文件，并且需要编
   写程序来解析处理操作这些文件。后续如果有新的需求，你又需要创建新的文件，并且
   可能要编写新程序来对新文件进行解析。随着需求不断增多，维护成本越来越高

2. DBMS

    ```
    DBMS = interrated data + software used to access and modify the data
    flat file storage = files + programs 
    ```

3. data abstraction的3层:
    * phycial level
    * logical level
    * view level

4. 数据是有内在联系的，其内在联系就存在于data abstraction中的logical level。view 
   level在简化系统的同时，也提供了一定的安全性

5. DBMS相比于FS的有改进的地方，在其DDL中应该都有所体现。比如，可能限制有一个值非
   负，可以用`CHECK(attr >= 0)`来做。

6. 数据定义语言的输出结果被称为数据字典(data dictionary)，包含元数据。数据字典可
   以被看作某种形式的表，但其只能被数据库本身所修改
   
7. DML有2种:
    1. 过程式的
    2. 声明式的

8. the query processor会将DML翻译为物理层操作数据的指令
    
9. database system可以被大致分为:
    * storage manager
    * query processor
    * transaction manager


