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
