# Memory

We can do memory usage analysis wiht `valgrind`, the most famous feature that 
Valgrind provides is checking memory leaks, but it can do other things as well.

Valgrind has a lot of tools that serve different purposes, 

* memcheck – memory leaks,

* massif – heap memory usage profiling,

* cachegrind – cache usage profiling,

* callgrind_ – call graph tracing,

* drd – data race condition detection,

* helgrind – deadlock/livelock detection.

* DHAT: a dynamic heap analysis tool (pretty good for detecting temp object that can be omitted, according to a [tweet][tweet_url] from Sled's author.)

  [tweet_url]: https://x.com/sadisticsystems/status/1230402417856073729

the default one, memcheck, is used to check memory leaks

```sh
$ valgrind --tool=memcheck you_program

# which is equivalent to 
$ valgrind your_program
```

## massif

Use it with the following command:

```sh
$ sudo valgrind --tool=massif your_program
```

then you will get a sample file under the current working directory, to view it, you can either:

1. use the `ms_print` command

2. or a GUI app made by KDE Massif-Visualizer


# Time

`perf` can do this, JetBrains IDE has it integrated, you can click the `profile`
button to do it.
