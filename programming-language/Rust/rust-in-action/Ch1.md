##### 1.5

1. `str` 有一个  `lines()` 方法，是按 `\n` split开，可以得到 `Struct std::str::Lines`  结构体，这个结构体实现了 `Iterator` 的 `trait`。

   ```rust
   let str = "a\nb\nc";
   for item in str.lines(){
   	println!("{}", item);
   }
   ```

   > 由于String也可以Deref成str，所以String也可以使用&str的方法。

2. cfg宏，这个宏主要是检测在编译时的一些配置，和c语言里ifdef和ifndef有点像。

   ```rust
   # 检测操作系统是Linux
   if cfg!(target_os="linux"){
   	println!("linux os")
   }
   ```

   ```rust
   # 看下编译是debug还是release
   fn main() {
       if cfg!(debug_assertions){
           println!("debug-enabled");
       }
       if cfg!(not(debug_assertions)){
           println!("debug-disabled");
       }
   }
   ```

3. 在使用collect拼成Vec时，常常需要指定类型，告诉rust我们collect的结果是Vec，并且需要告知Vec里面存放的类型，当Vec中类型使用 `_` 时表示，我们命令rust去自己进行类型推倒。

   ```rust
   fn main() {
       let hello = "hello world";
       // instructs Rust to infer the type of elements
       let v:Vec<_> =  hello.split_whitespace().collect();
       println!("{:?}", v);
   }
   ```

4. cargo run的 `-q` 参数 使输出变得安静

   ```shell
   ➜  leetcode git:(master) ✗ cargo run -q       
   ["hello", "world"]
   ➜  leetcode git:(master) ✗ cargo run   
       Finished dev [unoptimized + debuginfo] target(s) in 0.00s
        Running `target/debug/leetcode`
   ["hello", "world"]
   ```



