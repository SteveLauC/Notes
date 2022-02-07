1. rustc用来编译代码，rustdoc用来编译文档，但是我们通过让cargo来调用他们。

2. unlike c and cpp, in which assertions can be skipped, Rust always checks
   assertions regardless of the how the programm was compiled. There is also
   a `debug_assert!` macro, whose assertions are skipped when the programm is
   compiled for speed(release mode)
