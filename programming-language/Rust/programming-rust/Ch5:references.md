#### Ch5: References

> ##### Reference to values
> ##### Working with references
> ##### Reference Safety
> ##### Reference to values
> ##### Taking Arms Against a Sea of objects

1. Pointers can be divided into two categories:
   
   1. owning pointer: such a pointer has the ownership of the underlying
      buffer, e.g., `Box<T>`, `String` and `Vec`.

   2. non-owning pointer: mutable or immutable references.

2. If type `Foo` implements `Partial<RHS=Self>`, then references, or references
   to references and so on all implement this trait due to:

   ```rust
   impl<A: ?Sized, B: ?Sized> const PartialEq<&B> for &A
   where
       A: ~const PartialEq<B>,
   {
       #[inline]
       fn eq(&self, other: &&B) -> bool {
           PartialEq::eq(*self, *other)
       }
       #[inline]
       fn ne(&self, other: &&B) -> bool {
           PartialEq::ne(*self, *other)
       }
   }
   ```

   > This is the mechanism behind the behavior that Rust can see through any
   > number of references.

   > What is that `impl const Trait` and `A: ~const PartialEq<B>`,
   > see [What is ~const?](https://stackoverflow.com/q/70815748/14092446)
   > 
   > [related pre RFC: const_trait_impl](https://internals.rust-lang.org/t/pre-rfc-revamped-const-trait-impl-aka-rfc-2632/15192)

   > `std::ops::Add` does not have something like this, it even does not have 
   > generic implementation.

3. If you wanna know whether two references have the same address, use `std::ptr::eq()`
  
   ```rust
   use std::ptr::eq;
   
   fn main() {
       let x = 0;
   
       // This works because `&T` can be converted to `*const T`
       assert!(eq(&x, &x));
   }
   ``` 

   ```shell
   $ cargo r -q
   
   $
   ```

4. two kinds of `fat ptr`
   
   1. ref to slice

   2. trait object
      
      ```rust
      use std::ops::Add;
        
      fn main() {
          println!("{}", std::mem::size_of::<Box<dyn Add<i32, Output = i32>>>());
      }
      ```

      ```shell
      $ cargo r -q
      16
      ```

      Contains a pointer to the value and a pointer to the trait implementation.

#### Reference to values
#### Working with references
#### Reference Safety

1. Whenever a reference type appears inside another type's definition, you must
   write out its lifetime.

#### Sharing Versus Mutation

1. A slice is a fat pointer, where the the first field of that ptr points to the
   data directly.

   ```rust
   fn main() {
       let v = vec![1, 2, 3];
       println!("stack p: {:p}", &v);
       println!("heap p: {:p}", v.as_ptr());
   
       let slice = &v as &[i32];
       println!("ptr in slice: {:p}", slice);
   }
   ```

   ```shell
   stack p: 0x7ffd941c7ed0
   heap p: 0x55c6b5811ba0
   ptr in slice: 0x55c6b5811ba0
   ```

2. Why can't we have mutable and immuable references at the same time

   ```rust
   pub fn extend_from_slice(&mut self, other: &[T])
   ```

   ```rust
   fn main() {
       let mut v = vec![1, 2, 3];
       v.extend_from_slice(&v);
   }
   ```

   ```shell
   error[E0502]: cannot borrow `v` as mutable because it is also borrowed as immutable
    --> src/main.rs:8:5
     |
   8 |     v.extend_from_slice(&v);
     |     ^^-----------------^--^
     |     | |                 |
     |     | |                 immutable borrow occurs here
     |     | immutable borrow later used by call
     |     mutable borrow occurs here
   
   For more information about this error, try `rustc --explain E0502`.
   error: could not compile `tt` due to previous error
   ```

   If the above code is allowed, slice `&v` (argument passed to `extend_from_slice()`
   ) may become a dangling pointer. `v` is of type `Vec`, once its heap buffer is full, 
   it will reallocate to get a bigger buffer, and deallocate the previous heap 
   memory. **Remeber that slice is a pointer pointing directly to the data**, slice 
   `&v` is referring the previous heap memory, when deallocated, `&v` becomes a 
   dangling pointer.

   ![memory layout](https://github.com/SteveLauC/pic/blob/main/photo_2022-11-08_19-09-41.jpg)

3. Shared access is read only access
   
   Across the lifetime of a shared reference, neither its referent, **nor anything
   reachable from that referent**, can be changed by anything.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-11-08_19-46-18.jpg)

#### Taking Arms Against a Sea of objects
