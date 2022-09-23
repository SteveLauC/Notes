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

   For characters that are hard to read or wrie (control character: 0-32), you 
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

