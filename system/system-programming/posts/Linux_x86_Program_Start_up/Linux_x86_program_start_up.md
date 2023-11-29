> http://www.dbp-consulting.com/tutorials/debugging/linuxProgramStartup.html

> This is for people who want to understand how programs get loaded under linux.
> In particular it talks about dynamically loaded x86 ELF files. The information 
> you learn will let you understand how to debug problems that occur in your 
> program before main starts up.

1. Here is a disgram showing how a program starts

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-11-29%2015-01-32.png)

   The loader, can be your shell or your GUI launcher. It will setup things like
   environment variables, open file tables, and push `argc` `argv` and `envp` to
   the stack.

   After that, it will call `_start()`.

2. `argv` is a real array, where each element is a pointer to the actual 
   arguments, the last element is a NULL pointer indicating that it is
   the end.

   Argument are continuous in memory

   ```
   arg1\0arg2\0arg3\0
   ```

   Here is a program iterating over `argv` without using `argc`

   > For `envp`, to iterate over it, you have to do so cause we don't have a 
   > `envc`.

   ```c
   #include <stdio.h>

   int main(int argc, char *argv[]) {
       printf("argc: %d\n", argc);

       int ac_count = 0;
       while(1) {
           if (argv[ac_count]!=NULL) {
               ac_count += 1;
           } else {
               break;
           }
       }

       printf("ac_count: %d\n", ac_count);

       return 0;
   }
   ```

   ```sh
   $ gcc main.c && ./a.out
   argc: 1
   ac_count: 1
   ```

3. `envp` is right after `argv`, what is after `envp`, it is the ELF auxiliary 
   vector

   > Array `argv` `envp` and ELF auxiliary vector are continuous in memory, with
   > a NULL pointer separated.
   
   > ELF auxiliary vectors are a mechanism to transfer certain kernel level 
   > information to the user processes

   We can set the environment variable `LD_SHOW_AUXV` to 1 so that this vector
   can be printed out by the linker:

   ```sh
   $ cat src/main.rs
   fn main() {}

   $ cargo b

   $ LD_SHOW_AUXV=1 ./target/debug/rust
   AT_SYSINFO_EHDR:      0x7ffe3a713000
   AT_MINSIGSTKSZ:       3376
   AT_HWCAP:             178bfbff
   AT_PAGESZ:            4096
   AT_CLKTCK:            100
   AT_PHDR:              0x563d000a7040
   AT_PHENT:             56
   AT_PHNUM:             12
   AT_BASE:              0x7f542d94b000
   AT_FLAGS:             0x0
   AT_ENTRY:             0x563d000bcb50
   AT_UID:               1000
   AT_EUID:              1000
   AT_GID:               1000
   AT_EGID:              1000
   AT_SECURE:            0
   AT_RANDOM:            0x7ffe3a603db9
   AT_HWCAP2:            0x2
   AT_EXECFN:            ./target/debug/rust
   AT_PLATFORM:          x86_64
   AT_??? (0x1b): 0x1c
   AT_??? (0x1c): 0x20
   ```

   We can use the `getauxval(3)` libc function to do it in our program.

   > Can I write to this memory? 
   >
   > Yes, you can, see the next note.

4. Read the first element in ELF auxiliary vector in Rust:

   ```rs
   use nix::libc::c_ulong;

   extern "C" {
       pub static __environ: *const *const u8;
   }

   fn main() {
       unsafe {
           let mut envp = __environ;
           while !(*envp).is_null() {
               envp = envp.add(1);
           }

           // The memory layout of `auxvec` is
           // ```
           // [key][value][key][value][AT_NULL]
           // ```
           // where keys will never be 0 (AT_NULL)
           let auxvec: *const c_ulong = envp.add(1).cast();

           println!("{} = {}", *auxvec, *auxvec.add(1));
       }
   }
   ```

   ```sh
   $ cargo r -q
   33 = 140733270794240
   ```

   33 is the value of `AT_SYSINFO_EDHR`.

   ```rs
   use nix::libc::c_ulong;

   extern "C" {
       pub static __environ: *const *const u8;
   }

   fn main() {
       unsafe {
           let mut envp = __environ;
           while !(*envp).is_null() {
               envp = envp.add(1);
           }

           // The memory layout of `auxvec` is
           // ```
           // [key][value][key][value][AT_NULL]
           // ```
           // where keys will never be 0 (AT_NULL)
           let auxvec: *mut c_ulong = envp.add(1).cast_mut().cast();

           println!("{} = {}", *auxvec, *auxvec.add(1));
           *auxvec = 0; // set it to AT_NULL
           println!("{} = {}", *auxvec, *auxvec.add(1));
       }
   }
   ```
   ```sh
   $ cargo r -q
   33 = 140721539366912
   0 = 140721539366912
   ```

5. We can register a function to the `.fini_array` section so that it will be
   involved at exit:

   ```rs
   #[link_section = ".fini_array"]
   static ARGV_INIT_ARRAY: extern "C" fn() = {
       extern "C" fn init_wrapper() {
           println!("exit");
       }
       init_wrapper
   };

   fn main() {}
   ```
   ```sh
   $ cargo r -q
   exit
   ```

   If I understand correctly, this is different from `atexit(3)/on_exit(3)`
