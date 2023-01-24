这个 `os.rs` 是个抽象层，负责在各 OS 上 Direct I/O 的功能抽象出来，每个模块基本
都返回一个 `open` 函数
