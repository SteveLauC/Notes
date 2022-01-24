1. delete the char selected by the cursor
x
2. append text after the whole line
A

> Pressing <ESC> will place you in Normal mode or will cancel an unwanted and partially completed command.i

3. move the cursor to the start of the word, type dw to delete the word
dw
> when you type d, you will see it in the last line of vim, and vim is waiting for u to type another command.
  if u wanna cancel this command, type <ESC> instead.

4. type d$ to delete from current cursor to the end of line
d$
> dw means "cut from here to next word".

 before: fo[o]bar baz
        dw
 after:  fo[b]az
 de means "cut from here to the end of the current word".

 before: fo[o]bar baz
        de
 after:  fo[ ]baz


5. move the cursor to the start of line, not the first char
0

6. if u wanna go to the first char, use shift+4 instead

7. use w to mv the cursor to the start of the next word
8. use e to mv the cursor to the end of current word
> u can use a number to modify the motion above


9. use U to undo the changes you just made in this whole line
```
U
```
10. use u to undo, and use ctrl+R to redo
```
ctrl+R
```
11. The format for a change command is: operator   [number]   motion

12. the sentence deleted in vim is put in copy buffer, so u can use p to paste after the cursor

13. replace command: use rx to replace the cursor content with x

14. ```ce``` command: when u type ce, the cursor to the end of this word is deleted and enter you are in  **insert** mode now
    so u can input whatever u want.
    change operator
	it also supports c [number] motion pattern

15. use ctrl G to show your cursor location and files status, in your left-bottom corner

16. use gg to mv the cursor to the first line, G(capital) to mv the last line of file
    :0 and :$ have same functionality with them
	想要移动到指定的行，可以使用:num 或者 num G
17. search after the cursor in normal mode /pattern 
    search bofore the cursor in normal mode ?pattern
    use n to keep searching in same direction, N in a reverse direction

18. use ctrl-O to move the cursor to the last position, ctrl-I to move forward
   
19. 找到匹配的括号，移动光标到某一个括号上，摁下%光标移到和它匹配的括号上

20. substiture替代命令
    :s/old/new/  将会使用new替换old，但仅替换一次
    :s/old/new/g g意味着global，则此行的old都会被new所替代
    这个名令和sed编辑器很像

    To change every occurrence of a character string between two lines,
    type   :#,#s/old/new/g    where #,# are the line numbers of the range
                              of lines where the substitution is to be done.
    Type   :%s/old/new/g      to change every occurrence in the whole file.
    Type   :%s/old/new/gc     to find every occurrence in the whole file,
                              with a prompt whether to substitute or not.

21. 在vim中执行外部的命令，使用:!external_command

22. vim的保存，可以使用```:w``` 来保存，但这种保存是保存到你打开的这个文件的，如果想保存同样的内容到别的文件中，使用 ```:w NAME``` 
    来保存到一个名为NAME的新文件。

23. 保存部分内容到一个新文件中
    使用v进入visual mode, 然后选中你要保存的内容，然后摁:, 注意，此时底下会出现:'<,'> 这个标记，紧接着输入 w FILE_NAME

    S:'<,'> w FILE_NAME, 然后敲击回车，你选中的部分内容就被保存到了这个新文件中。

24. 从外部文件插入内容到当前文件，可以使用 ```:r file_name``` 来将file_name的内容插入到cursor的下一行。
	从下一行的第一个位置开始插入
	除了可以插入文件内容，还可以将外部执行命令的结构插入进来，比如
	```:r !ls``` 不过它插入的形式比较诡异，每行一个文件名，并没有在终端里执行ls的格式了，可能是delimitor的问题吧

25. 在cursor的下一行开新的一行，并进入insert mode， type o
    在cursor的上一行开新的一行，并进入insert mode，type O(captial o)



26. append mode, use a to append text after the cursor, you will be in insert mode.
	```
	a: append
	A: append after the end of this line
	o: open a new line below the cursor
	
	O: open a new line above the cursor
    ```
27. replace mode: 
	> 在vim中，使用rx可以将cursor的char改为x，这是只更改1个
	使用R(captial r)替换多个字符，此时的mode称之为replace mode
	Replace mode is like Insert mode, but every typed character deletes an existing character.

28. set option for search and subtitution
	在搜索和替换时忽略大小写 :set ic  (ic: incasesensitive)
    在搜索时开启语法高亮  :set hls (highlight search)
	在搜索时。。。(我不清楚这个是干吗的)    :set is (is: incremental search)
	
	如果想关闭某个option，比如使大小写再度敏感， ```set noic```，仅需要在option前加no，然后set下就好 
	
29. vim可以打开多个窗口，比如在打开文件的时候使用 ```:help something_you_wanna_know``` 就会打开另一个新的窗口，使用 ```ctrl w``` 可以    在不同的窗口之间跳转cursor

30. vim的命令也是可以自动补全的，```:e``` 然后摁下tab或者ctrl d就可以获得补全。

