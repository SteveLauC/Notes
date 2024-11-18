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
> * I learned about semi-synchronous replication/chain replication
> *


> TOC
>
> * Leader (replica, aka, primary) and Followers (replicas)
>   * Synchronious Versus Asynchronous Replication (And Semi-synchronous replication)
>   * Setting Up **New** Followers
>   * Headling Node Outages
>     * Follower failure: Catch-up 
>     * Leader failure: Failover
>   * Implementation of Replication Logs
>     * Statement-based (SQL-based) replication
>     * Write-ahead log (WAL) shipping
>     * Logical log replication (row-based replication)
>     * Trigger-based replication
> * Problems with Replication Lag
>   * Reading Your Own Writes (read-after-write consistency)
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

1. The complexity of replication lies in handling changed data, if your data won't
   be updated, then you literally do not need replication, just copy your data to
   other machines.
   
   Replication starts with sending writes to other replicas, and:
   
   1. The format of the write being sent
   2. Do you send them sequentically or concurrently
   3. Replica promotion, you don't want to lose your data
   4. If a node dies and restarts, how can it catch up
   5. Sync or async replication, that is the question
   6. You have copies, you have consistency issues, choose your concistency model

# Leader (replica, aka, primary) and Followers (replicas)

1. What is single-leader replication
 
   Among the replicas, there is a replica that would be considered the leader
   one, write request has to be handled by it first, then they will be sent 
   to other follower replicas.
   
   > This leader replica is commonly referred as primary.

   Postgres/MySQL/SQL server all have built-in support for this.
   
   > QUES: Postgres replication is done via WAL, so primary won't automatically
   > send the changes to the replica. Replicas will pull the changes, so I guess
   > it is kinda different to this mode. 
   
> NOTE: In the remaining notes, I will use primary and replica rather than leader
> and follower.
   
## Synchronious Versus Asynchronous Replication (And Semi-synchronous Replication)

* Synchronous Replication
   
  A write to the primary would block until all the replicas have been replicated
  successfully.
  
  Pros: the replicas would be guaranteed to be update-to-date
  Cons: If the replica does not respond, then the write to the primary won't 
  return.

* Semi-synchrounous Replication

  Due to the fact that a stateless replica can block the write operation in 
  synchronous replicatoin, it is usually impractical for all the replicas to be
  synchronous. In most cases, it would have only 1 replica that is synchronous,
  and other replicas are asynchronous. If the synchronous replica is slow or does
  not respond, one of asynchronous replicas is made synchronous.
  
  > YugabyteDB uses Raft for replication, and wait for a quorum of nodes to finish
  > their writes.
  
  With this replication mode, you are guaranteed to have an update-to-date data
  on 2 nodes, this mode is sometimes called semi-synchronous.

* Asynchronous Replication

  Write to the primary does not wait for the responses from the replicas. 
  
  Pros: write is fast
  Cons: you won't have an update-to-date copy of data when the primary is down.
  
> Tradeoff between different replication modes: write performance and availability.

* Chain Replication(research replication algorithm)

  A variant of synchronous replication that provides good performance and 
  availability.

## Setting Up **New** Followers

1. The procedures of setting up new replicas is different from replicating to
   replicas that exist at the first place.
   
   The storage engine should be able to provide snapshot of the stored data 
   at some point, it would be great if this can be done without locking the 
   database(stopping new writes).
   
   Then the process looks like this:
   
   1. Take a snapshot of the data
   2. Copy the snapshot to the new replica
   3. Start the new replica, it would request the log of the changes that happened
      since the snapshot was taken. So a snapshot need to be associated with the
      log of the last change included in it.
   4. Once the replica applies all the logs, we say it catches up, it can accept
      new read/replication requests.   

## Headling Node Outages
### Follower failure: Catch-up recovery

1. If a node crashes, and restarts, how to recover the replicas on it: A replica
   knows the log ID of its last log, so that it can request the logs that happened
   after the fault from the primary, once it applies all the changes since the
   fault, it can accept replication requests.
   
2. I think we can set up a new replica if the dead replica does not recover after
   a specific period of time. IMHO, we should also consider the number of remaining 
   replicas, if we have at least 3 copies of data, then we still have high 
   availability, so perhaps it is not necessary to set up new replicas in this
   case.

### Leader failure: Failover

1. If the primary is down, one of its replicas needs to be promoted to become the
   leader, the dead leader will become a replica, this process is called failover.

   Generally, this process looks like this:
   
   1. Determining that the primary has failed.
   
      Nodes bounce message back and forth between each other, if a node does not
      respond for a period of time, then we can assume that it is dead.
      
      > Though it may not be dead, nothing can be deterministic in a distributed
      > system.
      
   2. Choosing a new primary. If we are using asynchronous replication, we should
      use the replica with the most update-to-date data, to minimize data loss.
      
   3. Updating the metadata so that new writes will be redirectied to the new 
      primary.
      
2. Failover is fraught with things that can go wrong, and there are no easy 
   solutions to them(That's the fun part!):

   1. If we are using asynchronous replication, the promoted new primary may not
      have all the data that the the old primary has. The most common solution 
      is for the old leader’s unreplicated writes to simply be discarded, which
      may violate clients’ durability expectations.
      
      > Discarding writes is especially dangerous if other storage systems outside
      > of the database need to be coordinated with the database contents. For 
      > example, in one incident at GitHub [13], an out-of-date MySQL follower 
      > was promoted to leader. The database used an autoincrementing counter to 
      > assign primary keys to new rows, but because the new leader’s counter 
      > lagged behind the old leader’s, it reused some primary keys that were 
      > previously assigned by the old leader. These primary keys were also used 
      > in a Redis store, so the reuse of primary keys resulted in inconsistency 
      > between MySQL and Redis, which caused some private data to be disclosed 
      > to the wrong users.
      
   2. Split brain: 2 nodes in the cluster believe that they are both primary
   
   3. What is the right timeout to assume the old leader is dead? A longer timeout
      means that this shard will become unavailable for a longer time. However,
      if a timeout is too short, we may mistakenly assume the old leader is dead.
      
## Implementation of Replication Logs

This section introduces several replication methods briefly.

### Statement-based replication

> Postgres call this SQL-based replication

Send the SQL statement the primary receives to the replicas, replica parses the
received statement and execute it as if it was sent from the cient.

This approach is simple, but it has some drawbacks:

1. Anything that is non-deterministic cannot be replicated in this way.
2. If the statement depend on the existing data, then in order to make replicas
   get the same result after applying the statement, they have to has the same
   data, which can only be guaranteed if all replicas apply statements in the
   same order. (Primary send statements sequentically, replica apply statements
   sequentically)
   
> It is possible to work around those issues, however, because there are so many
> edge cases, other replication methods are now generally preferred.
>
> Statement-based replication was used in MySQL before version 5.1. It is still 
> sometimes used today, as it is quite compact, but by default MySQL now switches 
> to row-based replication (discussed shortly) if there is any nondeterminism 
> in a statement. 
>
> VoltDB uses statement-based replication, and makes it safe by requiring transactions
> to be deterministic 

One advantage of this approach is that statement (SQL) is stanardized, so it will
remain same across database versions, you don't need to worry about version
compatibility. SQLite directly stores DDL in `sqlite_master` for this reason.

### Write-ahead log (WAL) shipping

Primary sends WAL entries to replicas

### Logical log replication (row-based replication)

Even though it is called row-based, it does not use the row format used by storage
engine (if the storage engine uses row-based storage), thus the log format and
storage format are decoupled, this is the reason why it is called logical log,

Having its own log format can provides better compatibility between versions. 
And it does not have the issues of the statement-based approach. 

One disadvantage of this approach is that for DML like 'UPDATE <table> set <col> = x',
all the rows of <table> need to be sent over the network, whereas only one statement
is needed with the statement-base approach.

### Trigger-based replication

Not that interested in this one

# Problems with Replication Lag

1. What is eventual consistency?

   [Eventual consistency vs strong eventual consistency vs strong consistency](https://stackoverflow.com/q/29381442/14092446)
   
2. With asynchrouous replication, replicas can fall behind, in this section, we
   introduce the problems that may happen if this lag is too huge.

## Reading Your Own Writes (read-after-write consistency)

1. read-after-write consistency is a consistency model that guarantees that
   after a user (client) submits a write, the data can be read by the same user. It 
   makes no promises about other users.
   
   > QUES: 
   >
   > * How do you uniquely identifies a user/client
   > * What if your database is user-less
   
   
2. How to impl it with async replication
  
   1. If the read reads something that the user modifies, always use the primary.
      Otherwise, replicas can be used.
      
      This requires that you have some way of knowing whether something might 
      have been modified
      
   2. Monitor the replication progress, if the replication is not finished, read
      the primary, otherwise, read the replicas.

## Monotonic Reads 

1. If users reads twice, they see things move backwards in time, this is possible
   if the first read accesses a replica with little lag, then the next read accesses
   a replica with greater lag.

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