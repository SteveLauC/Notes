1. the ownership of rust can be seen as a tree, where the root is a variable in
   the stack, when root goes out of scope, the whole tree will be dropped.

   The owner is the parent node, and the contents is the child node. And each 
   resource can only be `dropped` by the owner.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-04_10-06-45.jpg)

   > `move` and `refernce count` are two ways to relax the rigidity of the
   > ownership tree.

2. if you move a value to an initialized value, Rust drops the prior one:
  
   ```rust
   struct Foo;
   
   impl Drop for Foo {
       fn drop(&mut self) {
           println!("foo is dropped");
       }
   }
   fn main() {
       let mut s = Foo;
       s = Foo;
       println!("--separator--");
   }
   ```

   ```shell
   $ cargo r -q
   foo is dropped
   --separator--
   foo is dropped
   ```

   But shadow won't drop the original value:

   ```rust
   struct Foo;
   
   impl Drop for Foo {
       fn drop(&mut self) {
           println!("foo is dropped");
       }
   }
   fn main() {
       let s = Foo;
       let s = Foo;
       println!("--separator--");
   }
   ```

   ```shell
   $ cargo r -q
   --separator--
   foo is dropped
   foo is dropped
   ```

3. Most types have the `move` semantics for the reason that such types uses a
   lot of memory and thus is expensible to copy. For example, `string/vec`.

   > `move` is cheap, copy the contents in the stack frame without involving
   > the heap allocation.

   `Mutable Reference` is different, this doesn't implement `Copy` to adhere to
   the memory safety rule, i.e, there is only one writer at any time.

   > To improve the ergonomics, Rust uses 
   > [`implicit reborrow`](https://stackoverflow.com/q/62960584/14092446).

4. Should my type be `Copy`:

   1. If a type can be easily copied with `memcpy` and such an operation will not
      violate the safety rule, it should be `Copy`.

   2. Any type needs to do something at the time when it's dropped should not be 
      `Copy`.

   Here gives an example, `std::fs::File`. Under the hood, `File` is just a 
   wrapper for `file descriptor` on UNIX platforms.

   ```rust
   // std::fs::File

   pub struct File {
       inner: fs_imp::File,
   }
   ```

   ```rust
   // fs_imp::File

   pub struct File(FileDesc);
   ```

   ```rust
   // FileDesc

   pub struct FileDesc(OwnedFd);
   ```

   ```rust
   // OwnedFd

   pub struct OwnedFd {
       fd: RawFd,
   }
   ```

   `file descriptor` is just a integer, which can be `Copy`. But if `File` implements
   `Copy` (say the detailed impl is to just copy the file descriptor), then we would
   have two owners of that file, each of which would call `close(fd)` when it goes out
   of scope, resulting an error `EBADF`.

   `File` does not implement `Clone`, which surprises me. But it does has a method,
   which **duplicates** the underlying file descriptor.

   ```rust
   use std::fs::File;
   use std::os::unix::io::AsRawFd;
   
   fn main() {
       let f1 = File::open("src/main.rs").unwrap();
       println!("fd of f1: {}", f1.as_raw_fd());
       let f2 = f1.try_clone().unwrap();
       println!("fd of f2: {}", f2.as_raw_fd());
   }
   ```
   ```shell
   $ cargo r -q
   fd of f1: 3
   fd of f2: 4
   ```

5. `std::rc::Rc`

   an `Rc<T>` value is a pointer to a heap-allocated T that has a reference count
   affixed to it.

   ```rust
   use std::rc::Rc;

   let r1 = Rc::new(String::from("shirataki"))
   let r2 = Rc::clone(&r1);
   let r3 = Rc::clone(&r1);
   ```

   Such a code will produce the following memory situation:

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-10-04_15-23-47.jpg)
