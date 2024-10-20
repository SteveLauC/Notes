> This chapter is about:
>
> * Before I read it
>
>   * Defines 3 common terminologies we are gonna use throughout the book
>
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
>
>   Perhaps just 3 terms?
>   
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case

> What have you learned from it
>
> * 3 terms, they are dimensions of the way how we think about software
> * Latency vs response time
> * percentile response time - tail latency
> * tail latency is hard to optimize because they are often affected by things 
>   that are not in control

# Thinking About Data Systems

1. What are data systems

   Data systems are tools like
   
   * databases
   * caches
   * message queues
   * search indexes
   * stream processing
   * batch processing
   
   They are different things but all called data systems because:
   
   1. Many new tools no longer neatly fit into the above categories. The boundaries
      between the categories are becoming blurred. 
      
   2. Typical applications need more than 1 tool, e.g., Postgres for source data,
      clickhouse for analysis.

# Reliability

1. What does it mean

   An application is reliable if it works correctly, even when we have faults.
   
2. Differentiate faults and failures

   Faults typically means a component of an application goes wrong. Failure means that
   the whole application, as a whole, can no longer provide service to the user. We
   should build fault-tolerant applications so that faults could never cause failures.
   
3. How to be fault-tolerant

   1. Prevent it from happening
   2. Cure it after happening (This book mainly deals with this one)
   
4. Kinds of faults

   1. hardware faults
   2. software faults
   3. Human erros (which could lead to either hardware or software faults)

## Hardware faults

1. Hareware faults will be pretty common if you have a lot of machines.

2. How to cure hardware faults - redunancy

   * disk - RAID
   * CPU - make it plugable
   * Power - have backup or dual power
   
   
   And, having redundancy for hardware **components** will be sufficient for most
   applications. Multi-machine redundancy was only required by a small number
   of applications for which high availability was essential.
   
3. Hardware faults are often independent from each other. Though sometimes they
   can be correlated, e.g., overheat in temperature makes everything dead, but
   under most cases, they are not related.
   
   As a contrast, software fautls are usually related, and can have chain reaction.

## Software faults


Software faults are usually correlated, for instance,

1. A bug in a low-level software can cause everything to crash, e.g., bugs in 
   the Linux kernel.
   
2. If a process uses up all the resources on a machine, then everything else 
   on the machine will likely be unavailable.

3. An error in one component can trigger a fault in another component, which in
   turn triggers further faults.
   
There is no general, good way to prevent software faults, be careful and do more
tests!

## Human errors

TIL that the term telemetry (遥测) comes from rocket engineering.

## How important is reliability

# Scalability

1. If an application is scalable, then it can cope with increased load.

## Describing Load

1. If you want to discuss load growth, you have to be able describe the **current**
   load of the system, load can be described with few numbers, there is no general
   way to describe a load, it is hightly related to your application:
   
   * web server: query per second
   * database: transaction per seciond, ratio of reads to writes
   * cache: hit rate
   
2. Indexes can speed up queries, by doing more things at write time (insert 
   time) so that you do less at read time (query).

## Describing Performance

1. How to describe the performance of a system

   For a batch system like hardoop, we care about its throughput. For an online
   system, what is more important is server's response time.

2. Latency vs Response time
  
  * Response time is the time between a client sending a request and receiving
    a response.
    
  * Latency is the duration that a request is waiting to be handled, it refers 
    to the delay in the system. For example, we say that s3 has higher latency
    when compared to local disk because sending an I/O request to the s3 server
    takes more time than sending it to the local disk.
    
  * Response time can be seen as the sum of both round trip latency plus the
    processing time. 
    
3. When you measure a service's response time for multiple times, the value
   can vary a lot, thus, we should not treat the value as a single numbe,
   but a **distribution**.
   
   Intuitively, people would care about the average response time when it comes
   to multiple different values, but it is not a good metric, for a specific
   response time, it won't tell you how many users will experience it.
   
4. Percentiles can solve the problem, if you take a list of response times, and
   sort it from fastest to slowest, then if the median (helfway point) is 200ms,
   that means half of your requests are faster than 200ms, half of your requests
   are slower than it.
   
   And, half users will experience delay that is longer than 200ms, and the other
   half users won't.
   
   The median is known as 50th percentile, and sometimes abbreviated as p50.
   
5. High percentile response times - tail latency

   In order to figure out how bad your slow requests are, you can take a look
   at high percentile response times, such as p95, p99, p99.9, high percentile
   resposne times are also called "tail latency".
   
   If the requests handle the same amount of data, i.e., the process time
   are basically same among requests, then optimizing tail latencies is difficult
   because they are easily affected by random events outside of your control:
   
   * context switch
   * loss of network package
   * TCP retransmission
   * Garbage collection pause
   * page fault
   * Queue delays: the number of requests that the server can handle is fixed,
     limited by its number of CPU cores, when the number of requests to handle
     is greater than its capability, some requests have to be queued, and thus
     they have longer response times.
     
6. Load generating tools need to emulate queue delays, i.e., when sending
   requests, don't wait for the previous request to complete then send the next
   one.

# Approaches for Coping with load

> How to maintain good performance even when our load  parameters increase by
> some amount.

1. How to do that?

   You can:
   
   1. Scale up: moving to a more powerful machine
   2. Scale out: distributing the load across multiple simpler machines

2. Some systems are elastic, meaning that they can automatically add computing
   resources when detecting a load increase, whereas other systems are scaled
   maually, a human analyzes the capacity and decides to add more machiens to
   the system.
   
   Elastic system can be useful if load is unpredictable.
   
3. Distributing stateless services across multiple machines is straightforward,
   taking a stateful data system from a single node to a distribtued setup
   can introduce a lot of additional complexity.
   
4. An architecture that scales up for a particular application is based on an
   assumption that which operations will be common and which will be rare.
   
   You have to know your workload to make your system scalable.

# Maintainability

1. For a software, the majority of its cost is not in its initial development,
   but in its ongoing maintenance.
   
   * Fixing bugs
   * keeping the system operational
   * investigation failures
   * adapting it to new platforms
   * modifying it for new use cases
   * repaying technical debt
   * adding new features
   
2. No one wants to work with a lagacy system, we should minimize maintenance
   pain so that the project won't be lagacy, To do this, we have 3 design
   principles to keep in mind:
   
   1. Operability
   2. Simplicity
   3. Evolvability(extensibility)

## Operability(可运维性): Making Life Easy for Operations

Provide operation APIs?

## Simplicity: **Managing Complexity**

1. Small software projects can have delightfully simple and expressive code, but
   as the projets get larger, they often become very complex and difficult to
   understand, **in a hidden way** (so, be careful!).
   
2. Complex software has the following symptoms

   1. tangled dependencies
   2. inconsistent naming and terminology
   3. hacks aimed at solving performance problems
   4. special cases to work around issues
   
3. We should manage complexity and remove it, removing complexity does not mean
   to reduce functionality, we remove "accidental complexity", the complexity
   added by implementation rather than functionality.
   
4. This book says that "One of the best tools we have for removing accidental
   complexity is abstraction".
   
   I think abstraction won't reduce accidental complexity within the module,
   it hides it. It is still up to the programmer to reduce the complexity
   while implementing the module.

## Evolvability(extensibility): Making Change Easy

1. System requirements will change, which forces you to refactor your data
   system. Agile development provides some tips on how to work on a changing
   system, but that only covers few files. In this book, we need to extend
   the scope to the whole data system, and thus, we don't "agility", but
   "Evolvability".

# Summary