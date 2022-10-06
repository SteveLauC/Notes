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
