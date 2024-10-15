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

2. How to cure

## Software faults
## Human errors
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