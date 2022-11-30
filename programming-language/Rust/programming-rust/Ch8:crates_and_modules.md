#### Ch8: Crates and Modules

> System programmers can have nice things
>
> ##### Crates
>
> ##### Modules
>
> ##### Turning a Program into a Library
>
> ##### The `src/bin` directory
>
> ##### Attributes
>
> ##### Tests and Documentation
>
> ##### Specifying Dependencies
>
> ##### Publishing Crates to crates.io
>
> ##### Workspaces
>
> ##### More nice things


##### Crates

1. The easiest way to see what crates are in your project is to use `cargo 
   b --verbose` to build it:

   ```shell
   $ cargo b --verbose
          Fresh unicode-ident v1.0.5
          Fresh proc-macro2 v1.0.47
          Fresh quote v1.0.21
          Fresh syn v1.0.104
          Fresh thiserror-impl v1.0.37
          Fresh thiserror v1.0.37
      Compiling rust v0.1.0 (/home/steve/Documents/workspace/rust)
       ...
      Finished dev [unoptimized + debuginfo] target(s) in 0.38s
   ```

2. Rust cares about code compatibility. But sometimes, breaking change can be 
   introduced. 
   [To evolve without breaking existing code, Rust uses `Edition`.](https://blog.rust-lang.org/2021/05/11/edition-2021.html)
   When Rust team would like to introduce breaking changes, they put it in a
   new edition of Rust.

   For example, in Rust 2018, it makes `async` and `await` become two keywords,
   making Rust programs where `async` and `await` are used as identifiers no longer
   compile.

3. Even though Rust uses Edition to separate breaking code changes, different 
   editions of Rust crates can still be compiled into a single bianry.

   In other words, a crate's edition only affect its source code.

3. `cargo` commands and their configuration section

   |Command    | Cargo.toml section used|
   |-----------|------------------------|
   |cargo b    | [profile.dev]          |
   |cargo b -r | [profile.release]      |
   |cargo t    | [profile.test]         |
   |cargo bench| [profile.bench]        |

   Normally, the default configurations are fine. One place where you need to 
   manually specify the section is when you use a 
   [`profiler`](https://en.wikipedia.org/wiki/Profiling_(computer_programming)).
   In such a case, you need both optimizations (release builds) and debuging 
   symbols (usually enabled in debug builds), to get them both:

   ```toml
   [profile.release] 
   debug = true # enable debug symbols in release builds
   ```

   To delete `debug info` or `symbols`, you use 
   [`strip`](https://doc.rust-lang.org/cargo/reference/profiles.html#strip)

   ```toml
   [profile.<name>]
   strip = OPTION
   ```

##### Modules

1. `pub (self)` makes item only visible in the current module (self) or any child
   modules, this is equivalent to `pub (in self)` or not using `pub` at all. 

   For more information, see 
   [pub(in path), pub(crate), pub(super), and pub(self)](https://doc.rust-lang.org/reference/visibility-and-privacy.html#pubin-path-pubcrate-pubsuper-and-pubself)

2. When Rust sees `mod a`, it will check both `a.rs` and `a/mod.rs`, if neither
   file exists or both exist, that's an error.

3. There are three modes you can empoly when separating modules among files, to
   define a module `fs` with submodule `file`

   1. A file named `fs.rs` with contents like this:

      ```rust
      pub mod file;
      ```

   2. A directory named `fs`:

      ```shell
      $ cd fs
      $ ls
      mod.rs file.rs

      $ cat mod.rs
      pub mod file;
      ```

    3. A directory named `fs` and a file named `fs.rs`

       > The `fs.rs` in this hierarch is equivalent to `mod.rs`

       > Personally I don't like this hierarcy, I perfer the second one.

       ```shell
       $ cat fs.rs
       pub mod file;

       $ cd fs
       $ ls
       file.rs
       ```

##### Turning a Program into a Library
##### The `src/bin` directory
##### Attributes
##### Tests and Documentation
##### Specifying Dependencies
##### Publishing Crates to crates.io
##### Workspaces
##### More nice things
