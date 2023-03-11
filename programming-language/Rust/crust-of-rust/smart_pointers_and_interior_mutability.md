1. `std::cell::Cell`

   `Cell` provides single-threaded interior mutability, for the updaing operation
   (write) it is ONLY safe to do this if there is no other readers or writers exist.

   Rust's type system guarantees that there is no other `&mut self`(`!Sync`) as 
   the current thread is using `&self` to update it. And the the mothods that 
   take `&self` all return the value of type `T` instead of any reference type, 
   which means no other references exist, so updaing the inner value is pretty safe.
   (we will NOT invalidate anything, no dangling pointer bc there is no ptr at all).

2. A code snippet tries to explain why returning `T` is needed for `Cell::get(&self)`

   Let's assume that our `get()` method returns a shared reference:

   ```rust
   pub fn get(&self) -> &T;
   ```

   ```rust
   let c = Cell::new(vec![42]);

   // we have a reference to the inner value
   let first_element: &i32 = &c.get()[0];
   println!("{}", first_element);

   // the old vec `vec![42]` is deallocated
   c.set(vec![]);

   // first_element becomes a dangling ptr
   println!("{}", first_element);
   ```

   ```shell
   $ cargo r -q
   42
   1443937366
   ```

3. To execute `cargo test` with its output shown

   ```shell
   $ carog t -- --nocapture
   ```

   And you may wanna use feature with a specific test:

   ```shell
   $ cargo t --tests <TEST CASE> -- --nocapture
   ```

4. An example to demo why Cell is not `Sync`

   ```rust
   struct Cell<T> {
       inner: UnsafeCell<T>,
   }

   unsafe impl<T> Sync for Cell<T> {}

   impl<T> Cell<T> {
       pub fn new(value: T) -> Self {
           Self {
               inner: UnsafeCell::new(value),
           }
       }

       pub fn set(&self, value: T) {
           unsafe {
               *self.inner.get() = value;
           }
       }

       pub fn get(&self) -> T
       where
           T: Copy,
       {
           unsafe { *self.inner.get() }
       }
   }
   ```

   We have our own `Cell` type, which is basically same with the `Cell` type
   from the standard libray, And we unsafely impl `Sync` for it so that our
   code won't be rejected by the compiler.

   ```rust
   fn main() {
       let c = Arc::new(Cell::new(42));

       let jh1 = spawn({
           let c = Arc::clone(&c);
           move || {
               c.set(1);
           }
       });

       let jh2 = spawn({
           let c = Arc::clone(&c);

           move || {
               c.set(2);
           }
       });

       jh1.join().unwrap();
       jh2.join().unwrap();

       // this will print 1 or 2, depending the execution order of those two threads.
       println!("{}", c.get()); 
   }
   ```

   With `Sync` implemented, we can use our `Cell` type like in the above code 
   snippet. This is generally a bad case for demonstrating our unsafe `Sync`
   impl, as it will be either 1 or 2.

   However, we can change the inner type from a number to an array, and assume 
   the array is big enough or the memory copy operation is slow enough, we will
   possibly get a corrupted array (i.e., an array that is partially 1 or 2):

   ```rust
   fn main() {
       let c = Arc::new(Cell::new([0; 1024]));

       let jh1 = spawn({
           let c = Arc::clone(&c);
           move || {
               // memcpy
               c.set([1; 1024]);
           }
       });

       let jh2 = spawn({
           let c = Arc::clone(&c);

           move || {
               // memcpy
               c.set([2; 1024]);
           }
       });

       jh1.join().unwrap();
       jh2.join().unwrap();

       // You possibly will get a corrupted array, like [1, ....2, ....2].
       // Though you may not bc memory copying is so fast nowadays.
       println!("{:?}", c.get());
   }
   ```

   Another demo case for it, which is easier to reproduce pratically:

   ```rust
   fn main() {
       let c = Arc::new(Cell::new(0));

       let jh1 = spawn({
           let c = Arc::clone(&c);
           move || {
               for _ in 0..10000 {
                   let old = c.get();
                   c.set(old + 1);
               }
           }
       });

       let jh2 = spawn({
           let c = Arc::clone(&c);
           move || {
               for _ in 0..10000 {
                   let old = c.get();
                   c.set(old + 1);
               }
           }
       });

       jh1.join().unwrap();
       jh2.join().unwrap();

       assert_ne!(c.get(), 20000);
   }
   ```

5. To allocate some memory for type T, you can use `Box`.

   ```rust
   let _b = Box::new(T);
   let ptr: *mut T = Box::into_raw(_b);
   ```

   instead of 

   ```rust
   let ptr: *mut T = unsafe {alloc(Layout::new::<T>()).cast() };
   ```
