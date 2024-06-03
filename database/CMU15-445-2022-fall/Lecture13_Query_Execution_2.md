> Today's agenda:
>
> * Process Models 
> * Execution Parallelism 
> * I/O Parallelism

1. Difference between Parallel and Distributed

   * Parallel Database
     
     1. Resources are physically close to each other
     2. Resources communicate over high-speed internet
     3. Communication is assumed to be cheap and reliable
     
   * Distributed Database

     1. Resources can be far from each other
     2. Resources communicate over slow(er) internet
     3. Communication is costly and can fail

# Process Model

1. What is process model

   Process model is how a DBMS handles multiple concurrent requests, there are 
   typically 3 process models

   > A worker is the DBMS component that is responsible for executing tasks 
   > on behalf of the client and returning the results.

   1. Process per worker

      Every worker is a separate OS process

      * Scheduling relies on the OS

        > QUES: I think we can intervene the scheduling, for example:
        >
        > * Use `SIGSTOP` to suspend a process
        > * Use `SIGCONT` to resume a process
        > * Change a process's priority flag
        >
        > So is this statement correct?
        >
        > Andy says that nobody does the above things.

      * Use IPC for communication (every call is a syscall)

      * A process crash does not take down the entire system (crash isolation)

      We typically see this approach in older systems, like PostgreSQL/DB2/Oracle. 
      And the main reason why PostgreSQL uses this model is that it was invented
      before `pthread` exists, and every OS has `fork(2)` implemented.

      And the PostgreSQL community is going to [make PostgreSQL multiple threaded][link]

      [link]: https://www.postgresql.org/message-id/31cc6df9-53fe-3cd9-af5b-ac0d801163f4%40iki.fi
      
   2. Thread per worker

      Pretty much every system built within the last 20 years uses this approach 
      except for those PostgreSQL forks.

      For scheduling, the DBMS always knows better than the OS. An interesting 
      project is SQLOS(SQL Server Operating System), which is an OS that directly
      runs inside of SQL Server and manages the hardware resources.

      > Having multiple threads architecture does not mean intra-query is supported,
      >
      > > No idea what intra-query is, search it within this note.
      >
      > e.g., MySQL uses thread per worker model but it only uses 1 thread for a
      > query.

   3. Embedded DBMS

      Examples include:

      * SQLite
      * DuckDB
      * LevelDB
      * RocksDB
      * DataFusion

      Since embedded systems run in the same process as application, application
      is responsible for the scheduling.

      Though the embedded system itself can still spawn threads or processes.

      > The first embedded database is BerkeleyDB.

# Execution Parallelism

1. There are 2 kinds of query parallelism

   1. Inter-query parallelism
     
      Execute multiple queries at the same time.

   2. Intra-query parallelism

      Execute a single query with multiple workers.

2. Intra-query parallelism

   For every query algorithm we have talked about so far, there are parallel
   versions of it.

   There are 3 kinds of intra-query parallelism:

   1. Intra-operator (horizontal)
      
      > Most common one

      The DBMS introduces a new operator, Exchange operator in the physical plan
      tree to coalesce/split results.

      > In DataFusion, there are `RepartitionExec` and `CoalescePartitionsExec`.

      > So I guess the "operator" here mean the logical operator, not the physical
      > operator (a node in the physical plan).

   2. Inter-operator (vertical)
      
      > Common in streaming systems

   3. Bushy

# I/O Parallelism

1. Split the database across multiple disks
