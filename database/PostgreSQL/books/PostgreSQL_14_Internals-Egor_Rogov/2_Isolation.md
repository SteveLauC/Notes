> This chapter is about:
>
> * Before I read it
>   * ...
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it, or adjust
> the listed stuff during reading process)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case

> What have you learned from it
>
> *
> *

> * 2.1 Consistency


# 2.1 Consistency

1. What is transaction and ACID

   A transaction is one or more operations that bring the data from one correct
   state to another correct state (consistency), these operations must be done
   atomically (atomicity), and won't be affected by other transactions(isolation).

   Once a transaction is committed, it can survive system failures (guaranteed
   by WAL).

2. Properties ACD are very basics for Databases, and should be implemented well.
   I is hard to do, so we have different Isolation levels.
