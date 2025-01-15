> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Looks like the first section covers a bit cluster management
>   * Different implementations of replication
>     1. Via log (I guess this is single-leader replication)
>
>        > Future steve aftere finishing this chapter, replication via log does 
>        > not necessary to be single-leader replication
>
>     2. Multi-leader replication
>     3. Leader-less replication
> * After I read it
>   * Single-leader replication
>   * Async replication and anomalies that could happen when lag is huge
>   * Common replication implementations 
>   * Multileader and leaderless replications
>   * Conflicts and how to resolve them

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
> * The concept of semi-synchronous replication and chain replication
> * Common replication approaches
>   1. statement-base
>   2. WAL streaming
>   3. (logical) row-based
> * Some common anomolies that could happen with async replication
> * The concept of multi-leader replication, it suffers from write conflicts
> * The concept of leaderless replication, due to the concurrent write to all the
>   replicas, I think this is hard to implement, or the actual implementation should
>   be quite limited
> * How to detect concurrency (Well, I do not understand this)


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
>     * Multi-datacenter operation
>     * Clients with offline operation
>     * Collaborative editing
>   * Handling Write Conflicts
>     * Synchronous versus asynchronous conflict detection
>     * Conflict avoidance
>     * Converging toward a consistent state
>     * Custom conflict resolution logic
>     * What is a conflict? 
>   * Multi-Leader Replication Topologies
> * Leader-less Replication
>   * Writing to the Database When a Node is Down
>     * Read repair and anti-entropy
>     * Quorums for reading and writing
>   * Limitations of Quorum Consistency
>     * Monitoring staleness
>   * Sloppy Quorums and Hinted Handoff
>     * Multi-datacenter operation
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

1. The procedures of setting up new replicas are different from replicating to
   replicas that already exist.
   
   The storage engine should be able to provide snapshot of the stored data 
   at some point, it would be great if this can be done without locking the 
   database(stopping new writes, if you don't update-in-place, it should be 
   easy to take a snapshot).
   
   > Looks like `pg_dump` can take a snapshot at the a database without stopping
   > users from accessing it, no idea how this works.
   
   Then the process looks like this:
   
   1. Take a snapshot of the data
   2. Copy the snapshot to the new replica
   3. Start the new replica, it would request the log of the changes that happened
      since the snapshot was taken. So a snapshot need to be associated with the
      log of the last change included in it.
   4. Once the replica applies all the logs, we say it catches up, it can accept
      new read/replication requests.   

## Headling Node Outages

> High availability

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
      use the **replica with the most update-to-date data**, to minimize data loss.
      
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

> I think "Logs" here mean "messages"

This section introduces several replication methods briefly.

> QUES: can you do concurrent replication for all the writes?
>
> > This question is not related to the book, I am currently working on replication
> > for Pizza, so...
>
> Elasticsearch does concurrent replication, I doubt it only does this for indexing
> new documents. Writes like update and delete typically depend on existing data.

### Statement-based replication

> Postgres calls this SQL-based replication

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

For row-based storage engine, WAL usually describes the change on a very low 
level.

[pg](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-11-23%20at%2010.26.22%E2%80%AFAM.png)

### Logical log replication (row-based replication)

> Logical log, as a contrast to physical log, WAL.

Even though it is called row-based, it does not use the row format used by storage
engine (if the storage engine uses row-based storage), thus the log format and
storage format are decoupled.

Having its own log format can provides better compatibility between versions. 
And it does not have the issues of the statement-based approach. 

One disadvantage of this approach is that for DML like 'UPDATE <table> set <col> = x',
all the rows of `<table>` need to be sent over the network, whereas only one statement
is needed with the statement-base approach.

### Trigger-based replication

Not that interested in this one

# Problems with Replication Lag

1. What is eventual consistency?

   [Eventual consistency vs strong eventual consistency vs strong consistency](https://stackoverflow.com/q/29381442/14092446)
   
2. With asynchrouous replication, replicas can fall behind, in this section, we
   introduce the problems that may happen if this lag is too huge.
   
   > You should think about whether these issues exist with synchornous 
   > replication as well.
   

## Reading Your Own Writes (read-after-write consistency)

1. read-after-write consistency is a consistency model that guarantees that
   after a user (client) submits a write, the data can be read by the same user. It 
   makes no promises about other users.
   
   > (Strict) Synchronous replication does not have this issue as it an acknowledged
   > write is guaranteed to be replicated to all the replicas.
   
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
   
   > (Strict) Synchrouous replication does not have this issue. 
   
2. Monotonic read can be achieved by ensuring that each user always read from the
   same replica, e.g., shard users to replicas in a fixed way.

## Consistent Prefix Reads

> QUES: what does "consistent prefix" mean? Especially, what does "prefix" mean?

1. This consistency level ensures that if a sequence of writes happens in a 
   certain order, then anyone reading those writes will see them appear in
   the same order.
   
   This is a particular problem for sharded databases, because shards usually 
   operate independently, there is no global ordering of writes.

## Solutions for Replication Lag

QUES: I have no idea why the book says that "This is why transactions exist: 
they are a way for a database to provide stronger guarantees so that the 
application can be simpler"

Does transation necessarily mean strong consistency? Or are they related?

# Multi-Leader Replication

1. What is multi-leader replication?

   Having more than one node act as leader nodes, every leader node is also a 
   follower of other leader nodes. Replication from a leader to other leaders
   are usually asynchronous to enable simultaneous writes.

## Use cases for Multi-Leader Replication

Postgres has an extension BDR (Bi-Direction replication), it was acquired by EDB.

### Multi-datacenter operation

When your database cluster crosses multiple datacenters, leader has to be in 1
datacenter. When your user is far from this datacenter, the write lacenty can
be quite high. If you have multple leader across these datacenters, your user
can write to the nearest datacenter to mitigate the issue.

Replication between a leader to other leaders (act like a follower) is 
asynchronous, so 2 writes to different datacenters can be conflicting, we need
conflict resolution.

### Clients with offline operation

Say you have a calendar APP that you can use on all your devices, phone, pad, and
your laptop. You can edit things on any devices, and the modification will be synced
to the server and other devices. In this case, every device can be seen as a leader.

### Collaborative editing
## Handling Write Conflicts
### Synchronous versus asynchronous conflict detection

In principle, you could make the conflict detection synchronous, i.e., wait for 
the write to be replicated to all replicas before telling the user that the 
write was successful. (synchronous replication) 

However, by doing so, you would lose the main advantage of multi-leader 
replication: allowing each replica to accept writes independently. If you want 
synchronous conflict detection, you might as well just use single-leader replication.

### Conflict avoidance

This is actually a recommended approach because:

* It works
* Multi-leader replication implementations suck at resolving conflicts

You can avoid conflicts by always forwarding the changes to the same entry to
a specific leader.

### Converging toward a consistent state

Conflict resolution algorithm must let the replicas converge toward to a consistent
state so that the database will be consistent.

1. Popular conflict resolution algorithms

   1. Given every write a unique ID (timestamp, UUID, ...), the write with the 
      longer ID wins.
      
      If timestamp is used, this is called *last write wins*. LWW will discard
      early-written data, and thus cause data loss.
      
   2. Give each replica a unique ID, and let writes that originated at a 
      higher-numbered replica always take precedence over writes that originated 
      at a  lower-numbered replica. This approach also implies data loss.
      
      > 抽象
      
   3. Somehow merge the values together, e.g., order them alphabetically and 
      then concatenate them (in Figure 5-7, the merged title might be something 
      like “B/C”).
      
      > 太抽象了，还不如不用这算法
      
   4. Record the conflict in an explicit data structure that preserves all 
      information, and write application code that resolves the conflict at 
      some later time (perhaps by prompting the user).

### Custom conflict resolution logic

Some databases allow you to write your custom confict resolution code that can
be executed by the database.

### What is a conflict? 
## Multi-Leader Replication Topologies
# Leader-less Replication

1. This replication mode became fashionable after Amazon used it for its Dynamo
   kv database.
   
   > Dynamo, not DynamoDB, the latter uses leader-based repliation.

2. How does leaderless replication work

   * write
     
     A write operation would write to all the nodes, the client can mark the write
     successful once quorum nodes finish the write.
     
   * read
   
     The client will read from all the nodes, but it waits for the quorum nodes to
     respond, then it uses the value with the largest version number.
     
3. Generic version of leaderless replication(does not necessarily to write/read to
   quorum nodes): say we have `n` nodes, and in a write operation, we wait for `w` 
   nodes to accept the write, and wait for `r` nodes to return the query result, as
   long as `w + r > n`, this replication mode works. 
   
   If `w + r = n + 1`, the most unfortunate case is that you only read from 1 node 
   that has the latest version of value, so you can still get the right value. 
   (1 overlap node)
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202024-11-26%20at%209.49.30%E2%80%AFAM.png)
   
   In a real-world database, `w` and `r` are configurable, e.g., if your use case
   does few writes but many reads, you can set `w = n ` and `r = 1`, this will make
   read faster because you will wait for less nodes to respond.
   
4. Fault-tolerance of leaderless replication

   * If `w < n`, you can still write even though `n - w` nodes are down
   * If `r < n`, you can still read even though `n - r` nodes are down

## Writing to the Database When a Node is Down
### Read repair and anti-entropy

1. A write operation does not need to write to all the nodes so it can tolerate
   node failures, but for those nodes with stale data, how do we update them?
   
   * Read repair
     
     When the client reads from `r` nodes, and finds that some nodes return a stale
     value, it writes the latest version of the value to these nodes.
     
     This won't work for data that are barely read.
     
   * Anti-entropy
   
     In addition, some datastores have a background process that constantly looks
     for differences in the data between replicas and copies any missing data from
     one replica to another. 
     
     Unlike the replication log in leader-based replication, this anti-entropy 
     process does not copy writes in any particular order, and there may be 
     a significant delay before data is copied.

### Quorums for reading and writing
## Limitations of Quorum Consistency

Even though condition `w + r > n` is satisfied, it is still likely that you can
read a stale value:

* If there are 2 concurrent writes, since it will write to all the nodes, there
  is no guarantee which write operation arrives first, and the order may diff
  across nodes. If conflicts will not be handled appropriately, then we can read
  stale value (or inconsistent value)
  
* If a failed write does not get rolled back

  > In this case, you won't read stale data, the response is simply invalid
  
* There are more cases listed by the book where you can read stale data, I think
  they are just implementation defects, so they are not included in my notes. 
  
QUES: The book says that the anomolies mentioned in section "Problems with 
Replication Lag" can also occur with leaderless replication, but 
[some posts](https://blog.bytebytego.com/i/136805316/quorum-writes-and-reads) say
leaderless replication provides strong consistency, then how these anomolies
could happen?

I think concurrent writes can break consistency.

### Monitoring staleness

With leader-based replication, systems usually expose metrics for the replication
lag, this is possibly due to that primary and replicas apply logs in the same 
order, so it is quite simple to figure out the replication lag. For example, Openraft
provides such metrics.

With leaderless replication, there is no fixed order in which writes are applied, 
making monitoring hard.

## Sloppy Quorums and Hinted Handoff

In a write operation, if the client can only write to `m` nodes (`m < w`), you 
can either:

1. Mark the writes failed
2. Accept the writes to `m` nodes, and write the value to nodes that are not part
   of `n` nodes. If `w - m ` nodes come back, you can write the value back.
   
   > QUES: why the fuck you have other nodes in your cluster, but they are not 
   > included in the cluster.
   
The second approach is called sloppy quorum, and the process that writes the value
back is called hinted handoff.

Sloppy quorum can increase write availability, i.e., more write operations can 
succeed, at the cost of bigger possibility of reading stale data.

### Multi-datacenter operation
## Detecting Concurrent Writes

1. Due to the fact that concurrent writes are allowed in leaderless replication,
   so with this replication strategy, we need to handle conflicts.
   
2. How to define 2 operations are concurrent?
   
   2 operations are concurrent if they don't know about/depend on each other
   
3. QUES: I think I will revisit this section in the future, currently, I do not 
   understand the point of this concurrency detection algorithm.
   
   QUES: when do you need to use version vector?

4. The definition of "concurrency" is a bit different from the "concurrency"
   that we are aware of, concurrency here does not need the operations to 
   happen at the same time or are quite close in time.

   TODO: re-read this chapter

# Summary