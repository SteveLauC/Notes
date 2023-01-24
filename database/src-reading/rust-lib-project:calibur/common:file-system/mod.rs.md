1. 这个模块里面有蛮多的`trait`，首先就是一些关于`File`的`triat`

   ```rust
   #[async_trait]
   /// 支持随机访问的文件
   pub trait RandomAccessFile: 'static + Send + Sync {
       async fn read(&self, offset: usize, data: &mut [u8]) -> Result<usize> {
           self.read_exact(offset, data.len(), data).await
       }
       async fn read_exact(&self, offset: usize, n: usize, data: &mut [u8]) -> Result<usize>;
       fn file_size(&self) -> usize;
       fn use_direct_io(&self) -> bool {
           false
       }
   }

   #[async_trait]
   /// 支持顺序访问的文件
   pub trait SequentialFile: 'static + Send + Sync {
       async fn read_sequential(&mut self, data: &mut [u8]) -> Result<usize>;
       fn get_file_size(&self) -> usize;
   }

   #[async_trait]
   /// 可以写的文件
   pub trait WritableFile: Send {
       async fn append(&mut self, data: &[u8]) -> Result<()>;
       async fn truncate(&mut self, offset: u64) -> Result<()>;
       fn allocate(&mut self, offset: u64, len: u64) -> Result<()>;
       async fn sync(&mut self) -> Result<()>;
       async fn fsync(&mut self) -> Result<()>;
       fn use_direct_io(&mut self) -> bool {
           false
       }
       fn get_file_size(&self) -> usize {
           0
       }
   }
   ```

   为什么对于文件的写没有区分顺序写和随即写呢？


2. 定义了一个 `FileSystem` 的 `triat` 来表示文件系统支持的操作

   ```rust
   pub trait FileSystem: Send + Sync {
       fn open_writable_file_in(
           &self,
           path: &Path,
           file_name: String,
       ) -> Result<Box<WritableFileWriter>> {
           let f = path.join(file_name);
           self.open_writable_file_writer(&f)
       }
    
       fn open_writable_file_writer(&self, file_name: &Path) -> Result<Box<WritableFileWriter>>;
       fn open_writable_file_writer_opt(
           &self,
           file_name: &Path,
           _opts: &IOOption,
       ) -> Result<Box<WritableFileWriter>> {
           self.open_writable_file_writer(file_name)
       }
    
       fn open_random_access_file(&self, p: &Path) -> Result<Box<RandomAccessFileReader>>;
    
       fn open_sequential_file(&self, path: &Path) -> Result<Box<SequentialFileReader>>;
    
       async fn read_file_content(&self, path: &Path) -> Result<Vec<u8>> {
           let mut reader = self.open_sequential_file(path)?;
           let sz = reader.file_size();
           let mut data = vec![0u8; sz];
           const BUFFER_SIZE: usize = 8192;
           let mut offset = 0;
           while offset < data.len() {
               let block_size = std::cmp::min(data.len() - offset, BUFFER_SIZE);
               let read_size = reader
                   .read(&mut data[offset..(offset + block_size)])
                   .await?;
               offset += read_size;
               if read_size < block_size {
                   data.resize(offset, 0);
                   break;
               }
           }
           Ok(data)
       }
    
       fn remove(&self, path: &Path) -> Result<()>;
       fn rename(&self, origin: &Path, target: &Path) -> Result<()>;
    
       fn list_files(&self, path: &Path) -> Result<Vec<PathBuf>>;
    
       fn file_exist(&self, path: &Path) -> Result<bool>;
       }
   ```

   然后代码里面有3个`*FileSystem*`的结构体实现了这个`trait`

   ```rust
   file_system/mod.rs::InMemFileSystem  
   file_system/async_file_system::AsyncFileSystem
   file_system/posix_file_system::SyncPosixFileSystem
   ```
