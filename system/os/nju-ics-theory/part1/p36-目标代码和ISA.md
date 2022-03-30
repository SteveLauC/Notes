1. gcc的`-E`选项可以生成预处理结果，`-S`生成汇编代码

   ```shell
   $ gcc -E test.c -o test.i
   $ gcc -S test.i -o test.s

   # 或者直接一条命令创建汇编
   $ gcc -S test.c -o test.s

   # 汇编来将汇编语言转为机器语言
   $ gcc -c test.s -o test.o
   ```


2. 同样的代码

   ```c
   // test.c
   int add(int i, int j) {
       return i+j;
   }
   ```

   ```c
   // main.c
   int main() {
       return 0;
   }
   ```

   首先进行编译生成可执行文件

   ```shell
   $ gcc main.c test.c -o test
   ```

   然后进行编译生成可重定位的目标文件
   ```shell
   $ gcc -c test.c -o test.o
   ```

   然后对可执行文件和可重定位的目标文件反汇编
   ```shell
   $ objdump -d test
   test:   file format mach-o arm64


   Disassembly of section __TEXT,__text:

   0000000100003f60 <_main>:
   100003f60: 00 00 80 52  mov     w0, #0
   100003f64: c0 03 5f d6  ret

   0000000100003f68 <_add>:
   100003f68: ff 43 00 d1  sub     sp, sp, #16
   100003f6c: e0 0f 00 b9  str     w0, [sp, #12]
   100003f70: e1 0b 00 b9  str     w1, [sp, #8]
   100003f74: e1 0f 40 b9  ldr     w1, [sp, #12]
   100003f78: e0 0b 40 b9  ldr     w0, [sp, #8]
   100003f7c: 20 00 00 0b  add     w0, w1, w0
   100003f80: ff 43 00 91  add     sp, sp, #16
   100003f84: c0 03 5f d6  ret

   $ objdump -d test.o
   test.o: file format mach-o arm64


   Disassembly of section __TEXT,__text:

   0000000000000000 <ltmp0>:
       0: ff 43 00 d1   sub     sp, sp, #16
       4: e0 0f 00 b9   str     w0, [sp, #12]
       8: e1 0b 00 b9   str     w1, [sp, #8]
       c: e1 0f 40 b9   ldr     w1, [sp, #12]
      10: e0 0b 40 b9   ldr     w0, [sp, #8]
      14: 20 00 00 0b   add     w0, w1, w0
      18: ff 43 00 91   add     sp, sp, #16
      1c: c0 03 5f d6   ret
   ```

   会发现相同的指令，但是地址是不一样的，可执行文件的地址是确切的值，而可重定位目标文件的
   地址是从0开始的。

   可执行文件中的确切地址是虚拟地址。

   ![ppt](https://github.com/SteveLauC/pic/blob/main/Screen%20Shot%202022-03-30%20at%2012.28.26%20PM.png)
   可执行文件的虚拟地址是虚拟地址空间中的`.text`字段开始的(上面的反汇编的汇编代
   码地址和ppt中的地址不同的原因是ppt是老师的机器上反汇编得到的)

   ![ppt](https://github.com/SteveLauC/pic/blob/main/Screen%20Shot%202022-03-30%20at%2012.32.40%20PM.png)
   elf和最后得到的虚拟内存的装载的对应关系


3. 硬件可以提供的功能都以指令的形式提供，ISA是一种接口(ABI)，约束，要求软件按照
   这个样子去使用硬件。
