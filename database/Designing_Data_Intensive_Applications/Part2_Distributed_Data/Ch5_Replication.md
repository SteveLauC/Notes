> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Looks like the first section covers a bit cluster management
>   * Different implementations of replication
>     1. Via log (I guess this is single-leader replication)
>     2. Multi-leader replication
>     3. Leader-less replication
> * After I read it
>   * ...

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case
>   1. Pros and cons of synchronous and asynchronous replication
>   2. Logs based replication vs Raft based replication (pull vs push)
>   3. What is multi-leader replication
>   4. What is leader-less replication

> What have you learned from it
>
> *
> *


> TOC
>
> * Leaders and Followers
>   * Synchronious Versus Asynchronous Replication
>   * Setting Up New Followers
>   * Headling Node Outages
>   * Implementation of Replication Logs
> * Problems with Replication Lag
>   * Reading Your Own Writes
>   * Monotonic Reads 
>   * Consistent Prefix Reads
>   * Solutions for Replication Lag
> * Multi-Leader Replication
>   * Use cases for Multi-Leader Replication
>   * Handling Write Conflicts
>   * Multi-Leader Replication Topologies
> * Leader-less Replication
>   * Writing to the Database When a Node is Down
>   * Limitations of Quorum Consistency
>   * Sloppy Wuorums and Hinted Handoff
>   * Detecting Concurrent Writes
> * Summary

# Leaders and Followers
## Synchronious Versus Asynchronous Replication
## Setting Up New Followers
## Headling Node Outages
## Implementation of Replication Logs
# Problems with Replication Lag
## Reading Your Own Writes
## Monotonic Reads 
## Consistent Prefix Reads
## Solutions for Replication Lag
# Multi-Leader Replication
## Use cases for Multi-Leader Replication
## Handling Write Conflicts
## Multi-Leader Replication Topologies
# Leader-less Replication
## Writing to the Database When a Node is Down
## Limitations of Quorum Consistency
## Sloppy Wuorums and Hinted Handoff
## Detecting Concurrent Writes
# Summary