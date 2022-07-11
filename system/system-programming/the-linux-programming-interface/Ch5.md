1. open的`O_CREAT`和`O_EXCL`正常情况下需要同时使用，如果`path`
   参数指向一个早就存在的文件，那么则会报错(EEXIST)。

   在正常情况下，`O_EXCL`都需要和`O_CREAT`配合使用。但有一个例
   外，在linux 2.6及以后，如果`pathname`指向的是block device，
   则可以去掉`O_CREAT`来使用

   在使用`O_EXCL`和`O_CREAT`调用`open(2)`的过程中，检查`path`
   是否存在以及创建文件是原子性的。

   > create a file exclusively指的是确保在创建这个文件时，此文
   件之前不存在

2. 在rust中exclusively创建一个文件

   `std::fs::OpenOptions`中有

   ```rust
   pub fn create_new(&mut self, create_new: bool) -> &mut Self
   ```
