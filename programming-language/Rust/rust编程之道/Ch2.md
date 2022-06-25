1. CTFE机制(compilation-time function execution)，指的是在编译时对函数进行执行的能力

    ```rust
    const fn n() -> usize {
        1 + 2
    }

    fn main() {
        let array: [i32; n()] = [0; n()];
        println!("{:?}", array);
    }
    ```

    比如上面的代码，`n()`就是在编译时进行执行确定返回值的。我们使用`const fn`来
    强制其在编译期进行计算。

    > Rust的CTFE是由`miri`来执行的，它已经被合并到Rustc中去了。在Rust2018时，其
    支持的工作还很有限。

2. closure是可以对参数和返回值显式地标示类型的
    
    ```rust
    fn main() {
        let v: Vec<i32> = vec![1];
        // map(|number| number + 1)
        let v_plus_one: Vec<i32> = v.iter().map(|number: &i32| number + 1).collect();
        println!("{:?}", v_plus_one);
    }
    ```

    > 感觉在这里显示地标记类型，是对抗match ergonomics暴政的好方法
