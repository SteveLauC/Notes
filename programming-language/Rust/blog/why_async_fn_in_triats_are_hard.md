> link: https://smallcultfollowing.com/babysteps/blog/2019/10/26/async-fn-in-traits-are-hard/ 

1. For the normal async function, after desuguaring

   ```rs
   async fn foo() { }

   fn foo() -> impl Future<Output = ()> {
       async {}
   }
   ```

   this should be done to the functions within a crate, which is not that possible
   until this [PR](https://github.com/rust-lang/rust/pull/115822) lands.

   The workaround provided by the `async_trait` macro turns the interface into:

   ```rs
   fn foo() -> Pin<Box<dyn Future<Output = ()>>> {
       let fut = async {};
       
       Box::pin(fut)
   }
   ```

   which is **different** from the normal async function.

2. async triat is hard for the following reasons


   1. Returning `impl triat` in traits is currently not supported (GAT, solved in 1.65)
   

      > Before that PR, the only places where `impl triat` are allowed are
      >
      > 1. argument of a normal function/a function within a impl block
      > 2. return value of a function/a function within a impl block
      >
      > These 2 types of functions are called: 
      >
      > 1. free function
      > 2. inherent function

      In the PR I mentioned above, the `impl trait` would be translated to a generic
      associated type:


      ```rs
      trait Foo {
          fn foo(&self) -> impl std::fmt::Display
      }

      trait Foo {
          type Foo: std::fmt::Display;

          fn foo(&self) -> Self::Foo;
      }
      ```

      And, the anonymous type representing a `Future`, is actually a state machine
      that catches (borrows) the context it is in, which would be something like:

      ```rs
      trait AsyncRead {
          async fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize>;
      }

      trait AsyncRead {
          type Read<'ctx>: std::future::Future + 'ctx;

          fn read<'ctx>(&mut self, buf: &mut [u8]) -> Self::Read<'ctx>;
      }
      ```

      Obviously, the `type Read<'ctx>: std::future::Future + 'ctx` is not a normal
      associated type, but a gerneric assocaiated type (GST), [which was landed in
      Rust 1.65](https://blog.rust-lang.org/2022/10/28/gats-stabilization.html).

   2.     
