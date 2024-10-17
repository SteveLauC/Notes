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
> *
> *

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
## Describing Load
## Describing Performance
# Approaches for Coping with load
# Maintainability
## Operability: Making Life Easy for Operations
## Simplicity: Managing Complexity
## Evolvability: Making Change Easy
# Summary