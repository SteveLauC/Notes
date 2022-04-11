1. Plug 'chun-yang/auto-pairs'

   这个插件提供括号的自动补全

2. Plug 'neoclide/coc.nvim', {'branch': 'release'}
   
   1. 需要`node(>12.2)`的依赖:
   
   ```shell
   # for macOS
   brew install node
   
   # for Ubuntu
   sudo apt update
   sudo apt install nodejs
   ```
   
   2. 然后就可以直接使用`vim-plug`来安装

   ```vimscript
   Plug 'neoclide/coc.nvim', {'branch': 'release'}
   ``` 

3. Plug 'airblade/vim-rooter'
   
   提供vim的自动保存，需要在`~/.vimrc`中加入
   
   ```vimscript
   let g:auto_save = 1 " enable default auto-save
   ```
   来激活自动保存，默认是不开启的


4. Plug 'dense-analysis/ale'
   
   提供代码的错误提示和函数跳转，rust的使用依赖`cargo/rustc`等官方工具链，并不使用
   `rust-analyzer`，需要注意的是，官方工具链中`rls`默认是没有安装的
   
   ```shell
   rustup component add rls
   ```

   > 错误提示，rust和c都开箱即用，目前不清楚它c依赖的是什么，估计是`clang`。函数跳转
   在rust中开箱即用，只需将光标放到标识符上，`:ALEGoToDefinition`就可以了；在c中函数
   跳转不能用。
    
5. Plug 'junegunn/fzf', { 'do': { -> fzf#install()  }  }
   
   提供文件的模糊查找，`fzf`本来是一个cli工具

   ```shell
   # for macOS
   brew install fzf

   # for Ubuntu
   sudo apt update
   sudo apt install fzf
   ```
   
   安装好cli的fzf后，想要在vim中使用fzf则需要再安装一些依赖(正常步骤)，但这些依赖提供的
   功能我目前用不到，所以不管它，我只使用在vim中查找当前工作目录下的文件的功能。则仅仅
   需要在`~/.vimrc`中加上这条配置run time path的语句
   
   ```vimscript
   # 后面的路径是macOS中使用homebrew安装的路径，请使用`which fzf`查看自己电脑中fzf的路径
   set rtp+=/opt/homebrew/bin/fzf
   ```

6. rust与c代码的自动补全
   
   1. rust中我使用`coc-rust-analyzer`，在安装好`coc.nvim`后，打开vim，输入`:CocInstall coc-rust-analyzer`
   即可
   
   2. c中我使用`coc-clangd`，需要事先安装好`clangd`，然后打开vim，输入`:CocInstal coc-clangd`
   即可

   关于clangd的安装

   macOS其实是自带的，其路径在`/Library/Developer/CommandLineTools/usr/bin/clangd`，二进制
   文件就在这里，但是这个`bin`目录并没有在`PATH`中，而且这个`bin`下面有很多的常用cli的二进
   制文件，为了不引起冲突，我决定不将其加入`PATH`中。使用`homebrew`安装一套新的
   ```shell
   brew install llvm
   ```
   安装完成之后，你输入`clangd`仍然是`command not found`，这是由于`homebrew`知道macOS自带这
   家伙，它也怕引起冲突，所以并没有在`/opt/homebrew/bin`中创建软连接。但macOS自带的`clangd`
   并没有直接暴露给用户，所以我就去`homebrew`的`bin`下面创建软连接
   ```shell
   cd /opt/homebrew/bin
   ln -s /opt/homebrew/Cellar/llvm/13.0.1_1/bin/clangd  clangd
   ```
