##### 8.2
1. 使用Rust发送http请求，我们使用`reqwest`这个库，但是这个库的最新版本已经是基于tokio的异步请求了，但其仍保留着早期版本的同步请求的
   api
   ```rust
   [dependencies]
   reqwest = {version="0.11.8",features = ["blocking"]} // 需要将blocking这个feature打开
   ```
   ```rust
   use std::error::Error;

   use reqwest;

   fn main() -> Result<(), Box<dyn Error>> {        
       let url = "target_url" 
	   let mut response = reqwest::blocking::get(url)?;

       let content = response.text()?; // 比较无语的是这个text函数会夺去response结构体的所有权。。。什么傻逼设计
       print!("{}", content);

       Ok(())
   }
   ```


