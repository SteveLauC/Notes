> wiki: https://perf.wiki.kernel.org/index.php/Tutorial


# Intro

1. Installation

   ```sh
   $ sudo dnf install perf
   ``` 

2. What are events

   You can treat a event as a metrics that we need to watch, based on their source,
   events can be divided into:

   1. Software events
      
      Software events come from the kernel itself, for example, context-switches, 
      minor-faults.

   2. Hardware events (or PMU hardware events)

      This kind of events come from the processor and its Performance Monitoring 
      Unit (PMU) 

      For AMD documents on this: https://www.amd.com/system/files/TechDocs/31116.pdf

   3. Tracepoint events 

      These events are implemented using the kernel 
      [ftrace](https://www.kernel.org/doc/html/latest/trace/ftrace.html) infrastructure.

      > ONLY available since kernel 2.6.3x. 

3. Obtain a list of supported events
  
   ```sh
   $ perf list
   ```

   Well, with perf 6.5.4, this command gives 1446 lines of output...

# Counting with `perf stat`

1. `ls`
   
   ```sh
   $ sudo perf stat ls

   Performance counter stats for 'ls':

          0.49 msec task-clock                       #    0.595 CPUs utilized
             0      context-switches                 #    0.000 /sec
             0      cpu-migrations                   #    0.000 /sec
            96      page-faults                      #  197.624 K/sec
     2,365,457      cycles                           #    4.869 GHz                         (89.67%)
        30,004      stalled-cycles-frontend          #    1.27% frontend cycles idle
       398,169      stalled-cycles-backend           #   16.83% backend cycles idle
     2,041,910      instructions                     #    0.86  insn per cycle
                                              #    0.19  stalled cycles per insn
       457,590      branches                         #  941.987 M/sec
        66,759      branch-misses                    #   14.59% of all branches             (10.33%)

   0.000816371 seconds time elapsed

   0.000000000 seconds user
   0.000845000 seconds sys
   ```

   > You can run `perf` without `sudo`, but that won't measure it at the kernel
   > level. By default, it will measure at  both user and kernel levels.

   By default, `perf stat` will give you the result of common events.

2. specify particular events with the `-e` option

   ```shell
   $ perf stat -e context-switches,cycles ls

   Performance counter stats for 'ls -l':

                 0      context-switches:u
           720,800      cycles:u

   0.001489388 seconds time elapsed

   0.001548000 seconds user
   0.000000000 seconds sys
   ```

   Since `sudo` is not passed, you only get the event at the user level, see the
   `:u` suffix.

   You can choose which level the `perf` command should be measured at:

   ```
   $ sudo perf stat -e context-switches:k ls -l
   ```

   With the `:k` suffix, we are explicitly asking to measure at the kernel level.

3. Modifier

   The suffix talked in the last node is called modifer.

   | Modifier | Description                                             | Example |
   |----------|---------------------------------------------------------|---------|
   |u         |monitor at priv level 3, 2, 1 (user)                     |event:u  |
   |k         |monitor at priv level 0 (kernel)                         |event:k  |
   |h         |monitor hypervisor events on a virtualization environment|event:h  |
   |H         |monitor host machine on a virtualization environment     |event:H  |
   |G         |monitor guest machine on a virtualization environment    |event:G  |

4. Scaling events with estimate if there are too many events

   If there are too many events that could be counted, the kernel uses multiplexing
   to 

   > I don't understand 
   > [this section](https://perf.wiki.kernel.org/index.php/Tutorial#multiplexing_and_scaling_events)

5. repeated measurements

   You can run a measurement mulitple times with the `-r` otion:

   ```shell
   $ sudo perf stat -r 2 ls

   Performance counter stats for 'ls' (2 runs):

             0.57 msec task-clock                       #    0.620 CPUs utilized               ( +- 10.34% )
                0      context-switches                 #    0.000 /sec
                0      cpu-migrations                   #    0.000 /sec
               98      page-faults                      #  173.214 K/sec                       ( +-  1.02% )
        2,732,499      cycles                           #    4.830 GHz                         ( +-  9.27% )  (87.24%)
           58,249      stalled-cycles-frontend          #    2.13% frontend cycles idle        ( +- 16.93% )
          697,970      stalled-cycles-backend           #   25.54% backend cycles idle         ( +- 20.45% )
        2,084,087      instructions                     #    0.76  insn per cycle
                                                 #    0.33  stalled cycles per insn     ( +-  1.11% )
          466,434      branches                         #  824.416 M/sec                       ( +-  1.05% )
           67,645      branch-misses                    #   14.50% of all branches             ( +- 14.28% )  (12.76%)

        0.0009129 +- 0.0000941 seconds time elapsed  ( +- 10.30% )
   ``` 

   The final result will be the mean of all these measurements, the standard 
   deviation will also be printed.

6. Environment Selection

   The `perf` tool can be used to count events on a:

   * per-thread

     The counter only monitors the execution of a designated thread.

   * per-process

     This can be seen as a variant of the per-thread mode, all threads of a process 
     will be monitored.

   * per-cpu

     All threads running on the designated processors are monitored.

   * system-wide

7. Inheritance

   By default, if you monitor a process with `per stat`, then all its threads 
   and its children processes will also be monitored, i.e., events will be 
   inherited.

   One can use the `-i` option to control this behavior.

   ```
   -i, --no-inherit      child tasks do not inherit counters
   ```

8. Monitor all CPUs

   By default, `perf stat` monitors in the per-thread mode, this does not mean
   it will only monitor a single thread of the process, it means that `perf stat`
   collects these events for each thread so that you can access the data for
   any particular thread if you want (using the `-t` option).

   If no such option is given, then `perf stat` will give you the aggregated 
   result of the all threads.

   `-a` option would make it monitor all the CPUs:

   ```shell
   $ sudo perf stat -a ls

   Performance counter stats for 'system wide':

         61.08 msec cpu-clock                        #   40.492 CPUs utilized
           112      context-switches                 #    1.834 K/sec
            33      cpu-migrations                   #  540.309 /sec
           108      page-faults                      #    1.768 K/sec
    10,827,122      cycles                           #    0.177 GHz                         (50.21%)
       107,145      stalled-cycles-frontend          #    0.99% frontend cycles idle        (78.48%)
     4,271,783      stalled-cycles-backend           #   39.45% backend cycles idle         (99.06%)
     6,429,031      instructions                     #    0.59  insn per cycle
                                              #    0.66  stalled cycles per insn
     1,482,129      branches                         #   24.267 M/sec
       203,759      branch-misses                    #   13.75% of all branches             (72.71%)

   0.001508352 seconds time elapsed
   ```

   You should note that this monitors all CPUs instead of just the part used by
   the `ls` process(i.e., it contains data of other processes).

9. Monitor specific CPUs using the `-C` option

   ```shell
   $ sudo perf stat -a -C 0 ls

   Performance counter stats for 'system wide':

          0.82 msec cpu-clock                        #    1.025 CPUs utilized
             2      context-switches                 #    2.441 K/sec
             0      cpu-migrations                   #    0.000 /sec
             2      page-faults                      #    2.441 K/sec
       157,945      cycles                           #    0.193 GHz                         (61.40%)
         3,533      stalled-cycles-frontend          #    2.24% frontend cycles idle
        48,080      stalled-cycles-backend           #   30.44% backend cycles idle
        84,406      instructions                     #    0.53  insn per cycle
                                              #    0.57  stalled cycles per insn
        20,039      branches                         #   24.457 M/sec
         5,072      branch-misses                    #   25.31% of all branches             (39.59%)

   0.000799591 seconds time elapsed
   ``` 

   The cycle field is enoronomously decreased compared to the previous output.

10. Attach to a running process or thread

    1. To attach to a specific process, use the `-p` option
    2. To attach to a specific thread, use the `-t` option

    For example, use the following command to monitor process <PID> for 2 seconds:

    ```shell
    $ sudo perf stat -p <PID> sleep 2
    ```

    The tailing command `sleep 2` is necessary or it will monitor until it is 
    killed.

# Sampling with `perf record`

1. Different from `perf stat`, `perf record` can not be used at the system-wide 
   mode, only the following three modes are supported:

   1. per-thread
   2. per-process
   3. per-cpu


# Analyze `perf.data` with `perf report`
# Source level analysis with `perf annotate`
