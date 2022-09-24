1. byte literal

   Represents a ASCII code.
   
   ```rust
   let b: u8 = b'a';
   ```

   ```rust
   let b = b'我';
   ```
   If the byte literal is not a valid ASCII code, a compiler error will be present.

   ```
    error: non-ASCII character in byte constant
     --> src/main.rs:4:15
      |
    4 |     let b = b'我';
      |               ^^
      |               |
      |               byte constant must be ASCII
      |               this multibyte character does not fit into a single byte
    
   ```

   For characters that are hard to read or write (control character: 0-32), you 
   can write their code in hexadecimal instead. For example, the `escape` char:

   ```rust
   let escaple = b'\x1b';

   // why not just write `0x1b_u8`
   // Well, when you wanna emphasize it is a ASCII code, you should use this 
   // notation.
   ```


2. count the `1`s or `0`s in its binary representation

   ```rust
   // Returns the number of ones in the binary representation of self.
   pub const fn count_ones(self) -> u32

   // Returns the number of zeros in the binary representation of self.
   pub const fn count_zeros(self) -> u32
   ```

   These two methods are available for all integer types

3. checked, wrapping, saturating and overflowing operations

   1. checked operations

      When a overflow happens, in debug build, the program will panic. And it won't 
      panic in a release build.
   
      When you want an explicit panic when a overflow happens, you can use:
   
      ```rust
      // Checked integer multiplication. Computes self * rhs, returning None if 
      // overflow occurred.
   
      pub const fn checked_OPT<T>(self, rhs: T) -> Option<T>
   
      // where `OPT` is the name of that operation, and `T` is the integer type.
      ```
   
      ```rust
      fn main() {
          let mut i = i32::MAX;
          let res = i.checked_add(1).expect("overflow happens");
          println!("{}", res);
      }
      ```
   
      ```shell
      $ cargo r
      thread 'main' panicked at 'overflow happens', src/main.rs:3:32
      note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace
      ```
   
      The `checked_OPT` calculates the arithmetic, if the result can be 
      represented in that type, return `Some(res)`, else, return None.

   2. Wrapping operations

      When the result can not be represented in that type, module it and return.
      This is the default behavior in release build.

   3. Saturating operations

      Saturating operatinos return the representable value that is **closest** to the
      mathematically correct result.
   
      ```rust
      fn main() {
          let min = i32::MIN;
          assert_eq!(min.saturating_sub(1), i32::MIN);
      
          let max = i32::MAX;
          assert_eq!(max.saturating_add(1), i32::MAX);
      }
      ```
   
      ```shell
      $ cargo r -q
      
      $
      ```

   4. overflowing operations

      Overflowing operations return a tuple (T, bool), where `T` is the wrapping
      value of that operation, the `bool` value is used to indicate whether overflow
      happens or not.
   
      ```rust
      let max = i32::MAX;
      assert_eq!(max.overflowing_add(1), (i32::MIN, true));
      ```

   ![available operations](https://github.com/SteveLauC/pic/blob/main/photo_2022-09-22_17-29-21.jpg)

4. `1.` is a valid floating point literal.
   
   ```rust
   let x: f64 = 1.;
   ```

5. IEEE 754 special constants
  
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-09-24%2009-27-05.png)

   And in rust, we have `INFINITY`, `NEG_INFINITY` and `NAN`. And we have functions
   like `is_subnormal()` to check if a float belongs to the subnormal (subset of
   denormalized numbers) category.

   > Currently IDK what are subnormal number actually.

   > We don't have `ZERO` and `NEG_ZERO` in Rust, but in the underlying 
   > binary representation, they are different.
   >
   > ```rust
   > fn main() {
   >     let zero = 0.0_f64;
   >     let neg_zero = -0.0_f64;
   >     println!("{:064b}", zero.to_bits());
   >     println!("{:064b}", neg_zero.to_bits());
   >     // And they are EQUAL as this is specified by IEEE754
   >     assert_eq!(zero, neg_zero);
   > }
   > ```
   >
   > ```shell
   > $ cargo r -q
   > 0000000000000000000000000000000000000000000000000000000000000000
   > 1000000000000000000000000000000000000000000000000000000000000000
   > ```

   ```rust
   // std::num::FpCategory
   // A classification of floating point numbers.
   // return type of `f32::classify()` and `f64::classify()`

   pub enum FpCategory {
       Nan,
       Infinite,
       Zero,
       Subnormal,
       Normal,
   }
   ```

   And module `std::f32/f64::const` also provides some commonly used constans:

   ```
   E        Euler’s number (e)
   FRAC_1_PI        1/π
   FRAC_1_SQRT_2        1/sqrt(2)
   FRAC_2_PI        2/π
   FRAC_2_SQRT_PI        2/sqrt(π)
   FRAC_PI_2        π/2
   FRAC_PI_3        π/3
   FRAC_PI_4        π/4
   FRAC_PI_6        π/6
   FRAC_PI_8        π/8
   LN_2        ln(2)
   LN_10        ln(10)
   LOG2_10        log2(10)
   LOG2_E        log2(e)
   LOG10_2        log10(2)
   LOG10_E        log10(e)
   PI        Archimedes’ constant (π)
   SQRT_2        sqrt(2)
   TAU        The full circle constant (τ)
   ```

5. `as` operator
   
   `as` can only be used with **primitive types**. And can perform either 
   [coercion](https://doc.rust-lang.org/reference/type-coercions.html#coercion-types)
   or the conversions in the following table:
   
   ![table](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-09-24%2010-26-32.png)

   Rust uses `as` to perform cheap and infailable conversions.

   [Diff between `as` and `std::convert::From`](https://stackoverflow.com/q/48795329/14092446):

   * The operation permitted by `as` is a small set and determined by the compiler.

   * `From` is a trait, programmer can impl this for their own type.

   * In number conversions, `From` must be loseless (From<i32> for i64). Using `as`
     you can convert a `i64` to a `i32` (forced type conversion).

6. char literal
  
   Like byte literal, you can use `'\xHH'` to create a literal from a hex value.
   Different from byte literal, char literal is not requested to be a valid ASCII
   (00-7f) character.

   A valid Unicode code point always has range `[0x0000, 0xD7FF]` and 
   `[0xE000, 0x10FFFF]`, so you can write a char literal like:

   ```rust
   let c = '\u{HHHHHH}'; // where H is a hex bit.
   ```

   ```rust
   fn main() {
       let c = '\u{000CA0}';
   
       println!("{}_{}", c, c);
   }
   ```
   ```shell
   $ cargo r -q
   ಠ_ಠ
   ```
   
   `u8` is the only type the `as` operator will convert to `char`:

   ```rust
   fn main() {
       let n = 9_u8;
       let c = n as char;
       // From<u8> for char
       let cc = char::from(n);
       assert_eq!(c, cc);
   }
   ```

7. some intersting methods on `char`

   ```rust
   // Encodes this character as UTF-8 into the provided byte buffer, and then 
   // returns the subslice of the buffer that contains the encoded character.

   pub fn encode_utf8(self, dst: &mut [u8]) -> &mut str
   ``` 

   ```rust
   // Returns the number of bytes this char would need if encoded in UTF-8.
   // That number of bytes is always between 1 and 4, inclusive.

   pub const fn len_utf8(self) -> usize
   ```

   ```rust
   fn main() {
       const C: char = '🦸';
       const BUF_LEN: usize = C.len_utf8();
       let mut buf = [0; BUF_LEN];
       C.encode_utf8(&mut buf);
       println!("{:?}", buf);
   }
   ```
   ```shell
   $ cargo r -q
   [240, 159, 166, 184]
   ```

8. Tuple can only be indexed by `integer literal`, neither integer variable nor
   `constants` is allowed.

   ```rust
   fn main() {
       let t = (1, 2, 3);
       for i in 0..3 {
           println!("{}", t.i);
       }
       const IDX: usize = 0;
       println!("{}", t.IDX);
   }
   ```

   ```shell
   $ cargo b -q
   error[E0609]: no field `i` on type `({integer}, {integer}, {integer})`
    --> src/main.rs:4:26
     |
   4 |         println!("{}", t.i);
     |                          ^
   
   error[E0609]: no field `IDX` on type `({integer}, {integer}, {integer})`
    --> src/main.rs:7:22
     |
   7 |     println!("{}", t.IDX);
     |                      ^^^
   
   For more information about this error, try `rustc --explain E0609`.
   error: could not compile `rust` due to 2 previous errors
   ```

   Tuples like this can be used to index a slice (impl<T> SliceIndex<[T]> for 
   (Bound<usize>, Bound<usize>)):
   
   ```rust
   use std::ops::Bound;
   
   fn main() {
       let a = [1, 2, 3];
       let t = (Bound::Included(0_usize), Bound::Included(1_usize));
   
       println!("{:?}", &a[t]);
   }
   ```

   ```shell
   $ cargo r -q
   [1, 2]
   ```

9. `slice::split_at(&sefl, mid: usize)` will panic if `mid` is not a char boundary.

   > Panics if mid is not on a UTF-8 code point boundary, or if it is past the 
   > end of the last code point of the string slice.

   If you try to implement it yourself, simply use the `reslicing`, this safety
   panic will still occur.

   ```rust
   fn main() {
       let str = "我";
       println!("{:?}", split_at(str, 1));
   }
   
   
   fn split_at(slice: &str, mid: usize) -> (&str, &str) {
       (&slice[..mid], &slice[mid..])
   }
   ```

   ```shell
   $ cargo r -q
   thread 'main' panicked at 'byte index 1 is not a char boundary; it is inside 
   '我' (bytes 0..3) of `我`', library/core/src/str/mod.rs:127:5
   ```

   ```rust
   fn main() {
       let str = "我";
       println!("{:?}", str.split_at(1));
   }
   ```
   ```shell
   $ cargo r -q
   thread 'main' panicked at 'byte index 1 is not a char boundary; it is inside 
   '我' (bytes 0..3) of `我`', library/core/src/str/mod.rs:127:5
   ```

10. Rust permits an extra tailing `comma` everywhere commas are used:
    
    ```rust
    let t1 = (1, 2, 3);
    let t2 = (1, 2, 3,);

    assert_eq!(t1, t2);
    ```

    NOTE: Most cases, this extra comma is optional, but for tuple with only
    one element, this is necessary to distinguish between a tuple and a
    parentheses-enclosed value.

    ```rust
    use std::any::Any;
    
    fn main() {
        let t_with_single_value = (String::new(),);
        let parentheses_enclosed_value = (String::new());
    
        println!(
            "{:?} {:?}",
            t_with_single_value.type_id(),
            parentheses_enclosed_value.type_id()
        );
    
        assert_ne!(
            t_with_single_value.type_id(),
            parentheses_enclosed_value.type_id()
        );
    }
    ```

    ```shell
    $ cargo r -q
    warning: unnecessary parentheses around assigned value
     --> src/main.rs:5:38
      |
    5 |     let parentheses_enclosed_value = (String::new());
      |                                      ^             ^
      |
      = note: `#[warn(unused_parens)]` on by default
    help: remove these parentheses
      |
    5 -     let parentheses_enclosed_value = (String::new());
    5 +     let parentheses_enclosed_value = String::new();
      |
    
    TypeId { t: 14418814694218527669 } TypeId { t: 9611275717051495212 }
    ```

11. Any type which is allocated on the heap will involve `std::alloc::Allocator`

    ```rust
    // Box
    pub struct Box<T, A = Global>(_, _) 
    where
        A: Allocator,
        T: ?Sized;

    // Vec
    pub struct Vec<T, A = Global> 
    where
        A: Allocator, 
    { /* private fields */ }

    // VecDeque
    pub struct VecDeque<T, A = Global> 
    where
        A: Allocator, 
    { /* private fields */ }
    ```

