#### Ch6: Expressions

> ##### An Expression Language
>
> ##### Precedence and Associativity
>
> ##### Blocks and Semicolons
>
> ##### Declarations
>
> ##### If and match
>
> ##### If let
>
> ##### Loops
>
> ##### Control Flow in Loops
>
> ##### return Expressions
>
> ##### Why Rust has loop
>
> ##### Function and Method Calls
>
> ##### Reference Operations
>
> ##### Arithmetic, Bitwise, Comparison, and Logical Operations
>
> ##### Assignment
>
> ##### Type Casts
>
> ##### Closures
>
> ##### Onward

##### An Expression Language

##### Precedence and Associativity

1. Trait `std::ops::Not`

   ```rust
   // The unary logical negation operator !

   pub trait Not {
       type Output;
   
       fn not(self) -> Self::Output;
   }
   ```

2. Trait `std::ops::Neg`

   ```rust
   // The unary negation operator `-`

   pub trait Neg {
       type Output;
    
       fn neg(self) -> Self::Output;
   }
   ```


##### Blocks and Semicolons

1. In Rust, pretty much everything is `expression`, which has some return value.
   When a `;` is added to the tail of an expression, the return value is dropped.
   Thus, the meaning of the semicolon differs from the one in C/C++ in Rust.

   ```rust
   fn foo() -> i32 {
       1
   }
   
   fn main() {
       let with_semicolon: () = {
           foo();
       };
       
       let without_semicolon: i32 = {
           foo()
       };
   }
   ```

##### Declarations
##### If and match
##### If let
##### Loops

1. You can write loop like this:
  
   ```rust
   fn main() {
       for i in 0..20 {
           println!("{}", i);
       }
   }
   ```

   because `std::ops::Range` (the type of `..`) has `Iterator` implemented, and 
   thus has `IntoIterator` implemented.

   Only ranges that have `start` are iterable, since iterator must have somewhere
   to start. (`Range/RangeFrom/RangeFull`).

##### Control Flow in Loops

1. You can give `break` an expression, which will become the return value of the
   loop where that `break` exists.

   ```rust
   fn next_line() -> Option<&str> {
       Some("line")
   }
   fn main() {
       let answer = loop {
           if let Some(line) = next_line() {
               if line.starts_with("answer") {
                   break line;
               }
           } else {
               break "answer: nothing";
           }
       };
   }
   ```

##### return Expressions
##### Why Rust has loop
##### Function and Method Calls
##### Reference Operations
##### Arithmetic, Bitwise, Comparison, and Logical Operations

1. Integer division rounds toward zero, this is exactly removing the decimal part.

2. Not operation

   We have two kinds of `not`, one is logical not, the other is bitwise not.
   Both of them is represented by `std::ops::Not` in Rust:

   ```rust
   pub trait Not {
       type Output;
   
       fn not(self) -> Self::Output;
   }
   ```

   In C, we use `!` for logical not, `~` for bitwise not. In Rust, we use `!`
   for both.

##### Assignment

1. Shadowing won't drop the original value

   ```rust
   struct Foo;
   
   impl Drop for Foo {
       fn drop(&mut self) {
           println!("Dropped");
       }
   }
   fn main() {
       let f = Foo;
       let f = Foo;
       println!("end of main");
   }
   ```
   ```shell
   $ cargo r -q
   end of main
   Dropped
   Dropped
   ```

   Overwriting will:

   ```rust
   struct Foo;
   
   impl Drop for Foo {
       fn drop(&mut self) {
           println!("Dropped");
       }
   }
   fn main() {
       let mut f = Foo;
       f = Foo;
       println!("end of main");
   }
   ```

   ```shell
   $ cargo r -q
   Dropped # prior value of `f` is dropped
   end of main
   Dropped
   ```

2. `+` and `+=` are different traits in Rust, one is `std::ops::Add`, the other
   is `std::ops::AddAssign`

   ```rust
   use std::ops::Add;
   
   struct Foo;
   
   impl Add for Foo{
       type Output = Foo;
   
       fn add(self, _rhs: Self) -> Self::Output {
           Foo
       }
   }
   
   fn main() {
       let f1 = Foo;
       let mut f2 = Foo;
       f2 = f1 + f2;
       f2 += f1; // error: requires the implementation of `AddAssign`.
   }
   ```

   ```shell
   $ cargo r -q
   error[E0368]: binary assignment operation `+=` cannot be applied to type `Foo`
      --> src/main.rs:17:5
       |
   17  |     f2 += f1;
       |     --^^^^^^
       |     |
       |     cannot use `+=` on type `Foo`
       |
   note: an implementation of `AddAssign<_>` might be missing for `Foo`
      --> src/main.rs:3:1
       |
   3   | struct Foo;
       | ^^^^^^^^^^ must implement `AddAssign<_>`
   note: the following trait must be implemented
      --> /home/steve/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/src/rust/library/core/src/ops/arith.rs:758:1
       |
   758 | pub trait AddAssign<Rhs = Self> {
       | ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   
   For more information about this error, try `rustc --explain E0368`.
   error: could not compile `tt` due to previous error
   ```

##### Type Casts

1. Rust support C-like enum:
   
   ```rust
   enum Type {
       A,
       B,
       C,
   }
   
   fn main() {
       let a = Type::B;
   
       println!("{}", a as isize);
   }
   ```

   ```shell
   $ cargo r -q
   1
   ```

   And you can specify the value you want:

   ```rust
   #[repr(i32)]
   #[derive(Debug, Clone, Copy)]
   #[allow(non_camel_case_types)]
   /// `attrnamespace` argument
   pub enum AttrNamespace {
       /// User namespace EA
       EXTATTR_NAMESPACE_USER = libc::EXTATTR_NAMESPACE_USER,
       /// System namespace EA
       EXTATTR_NAMESPACE_SYSTEM = libc::EXTATTR_NAMESPACE_SYSTEM,
   }
   ```

   Though you should note that the default integer type employed is `isize`,
   if you don't like it, use `#[repr(TheTypeYouWant)]` to specify it.


   Most conveniently, you can use `as` to cast thoes C-like enum directly to
   integer types they represent

   > This is documented in 
   > [Ch3 fundamental_types.md: 6](https://github.com/SteveLauC/Notes/blob/main/programming-language/Rust/programming-rust/Ch3:fundamental_types.md)

   ```rust
   pub fn extattr_delete_fd<S: AsRef<OsStr>>(
       fd: RawFd,
       attrnamespace: AttrNamespace,
       attrname: S,
   ) -> Result<()> {
       // Use `as` to cast it here
       let namespace = attrnamespace as libc::c_int;
   ```

##### Closures
##### Onward
