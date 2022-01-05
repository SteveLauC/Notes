### The Rust Rand book 笔记

#### 1.Getting started

* 使用这个库的比较简便的方式是引用它的 `prelude` 

  ```rust
  use rand::prelude::*;
  ```

* random方法

  当引用了prelude后就可以使用最常见的 `random` 方法，来获取任意类型的布尔值，可以使用 `turbofish` 或指定被赋值变量的类型来
  指明类型。

  ```rust
  use rand::prelude::*;
  
  let rand_i32 = random::<i32>();
  let rand_boolean = random::<bool>();
  let rand_u32: u32 = random();
  
  # 有时候它可以静态分析出类型，此时就不需要我们进行指定
  if random() {
      println!("the random boolean is true");
  }
  ```

* 更精确且有效的方法 ThreadRng [link](https://docs.rs/rand/latest/rand/rngs/struct.ThreadRng.html)

  这个结构体是对 thread local generator的引用，可以通过使用 `thread_rng()` 或者 `ThreadRng::default()` 得到。注意这个结果体
  不可以在线程间传递。

  ```rust
  // 注意generator需要是可变的 因为所以的gen方法第一个参数都是&mut self
  let mut generator = thread_rng();
  let mut genreator2 = ThreadRng::default();
  // 产生随即值
  let random_i32 = generator.gen::<i32>();
  let random_u32: u32 = generator.gen();
  let random_bool = genreator2.gen::<bool>();
  
  // 产生某一个范围的随机值
  let dice_val = generator.gen_range(1..=6);
      
  // 以某个概率产生true
  let true_with_probability_two_thirds = generator.gen_bool(2 as f64/3 as f64);

  // 还是以某个概率产生true，但不是直接给浮点数，而是给分子和分母
  let true_with_probability_fraction = generator.gen_ratio(2_u32, 3_u32);
  ```
