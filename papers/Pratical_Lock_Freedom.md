# 1. Introduction
## 1.1 Motivation

1. Locks are not suitable for highly-parallel systems in a number of ways:
  
   1. Mutex would restrict parallelism by serialising non-conflicting updates,
      this can be mitigated by using fine-grained locks, but lock convoying
      and cache performance may then become an issue.

   2. Cache line bouncing

      Every core has its own L1 cache, and cache line is the smallest unit that
      cache works, since all the cores has its own L1 cache, when modification
      has been made to it by a core, it has to synchronize the contents to other
      cores, by copying cache lines.

      Let's say that we have a global variable, which is smaller than the size of
      a cache line, 

      > On Amd64, cache line is 64 bytes.

      it will be modified by 2 threads, these 2 threads will run on 2 cores, so
      this variable will have 2 copies in 2 cores' cache lines. When one thread
      modifies it, the cache line has to be synchronized to the other thread, and
      vice versa.

      The fact of transferring cache lines between more cores is called `cache line
      bouncing`, it feels like the cache line bounces between different cores.

      For locks, those counters will likely be smaller than a cache line, and it
      will be modified by many threads (many cores), and thus has cache line bouncing.

   3. In a cluster, deadlock can occur if a node fails while holding a lock.

      > Leased lock can be used to solve such issues.

## 1.2 Terminology discussion

1. lock-free

   Use the term lock-free to refer to a system that is guaranteed to make 
   progress within a finite number of execution steps.

   > Locks have many drawbacks incuding a very weak progress guarantee. A
   > stalled thread/process can make the whole system stalled.

   So that A program that does not use locks is not necessarily lock-free.

   > I believe `non-blocking` (won't get stuck) is a better word, the block here
   > means that a thread won't be blocked if it failed to do something, don't 
   > confuse it with the term `obstruction-freedom` that will be introduced
   > in the next section.
   >
   > The term `non-blocking` used in this paper is a general term.

2. compare-and-swap (CAS)

   Atomic read-modify-write primitive provided by the hardware

   > [Which CPU architectures support Compare And Swap](https://stackoverflow.com/q/151783/14092446)

3. double-word compare-and-swap (DCAS)

   CAS allows you to atomically modify one value, DCAS would allow you to do two.
   Both values will be atomically updated if and only if both contain the expected
   value.

4. recurisve helping

   > I don't think I quite understand this

   If two operations block each other, say operation A and B, then A will help
   B to complete its work, once B is done, then A will do its own work.

5. mutual-exclusiino lock (a.k.a, Mutex)

6. multi-reader locks (a.k.a., RwLock)
   
## 1.3 Contribution

1. 2 lock-free abstractions will be presented in this paper:

   1. multi-word compare-and-swap
   2. software transactional memory

## 1.4 Outline
## 1.5 Pseudiocode conventions

1. word type

   A integer type that has the same size as a word 

2. new keyword

   Would create a variable, and the memory will be garbage-collected

# 2. Background
## 2.1 Terminology

1. All the non-blocking properties described here guarantee that a stalled
   processs can not cause all other proesses to stall indefinitely. 

### 2.1.1 Lock-freedom
### 2.1.2 Wait-freedom

1. A data structure is wait-free if and only if every operation on the structure
   completes after it has executed a finite number of steps.

   A FIFO using `AtomicPointer` is not wait-free because we have a `loop` that
   could possibly take forever.

2. wait-free is really hard and impossible under most cases.

### 2.1.3 Obstruction-freedom
## 2.2 Desirable algorithmic features

### 2.2.1 Disjoint-access parallelism

1. A set of operations are disjoint-access parallel if and only if any pair
   of operation invocations which access disjoint sets of memory locations
   do not directly affesct each otehr's execution.

   However, it does not prevent an operation from indirectly affecting another' s
   performance (perhaps via cache effects)

### 2.2.2 Linearisability

1. For a operation, if it is implemented as a synchronous precedure, then a
   call to that procedure is a request, the eventual return from that 
   procedure is a response.

   An operation is linearisable if and only if it appears to execute instantaneously
   at some point between its request and response.

   > The definition is quite hard to understand, but the behavior should be
   > understandable, linearisable procedures behave as if the data they access
   > is protected by a single muteal-excusion lock.

## 2.3 Related work
### 2.3.1 Non-blocking primitives
### 2.3.2 Universal constructions
### 2.3.3 Programming abstractions
### 2.3.4 Ad hoc data structures
### 2.3.5 Memory management

# 3. Pratical lock-free programming abstractions
## 3.1 Introduction
## 3.2 Multi-work compare-and-swap (MCAS)
### 3.2.1 Design
## 3.3 Software transactional memory
### 3.3.1 Programming interface
### 3.3.2 Design
### 3.3.3 Further enhancements
## 3.4 Summary

# 4. Search structures
## 4.1 Introduction
## 4.2 Functional mappings
## 4.3 Skip lists
### 4.3.1 FSTM-based design
### 4.3.2 MCAS-based design
### 4.3.3 CAS-based design
## 4.4 Binary search trees
### 44.1 MCAS-based design
## 4.5 Red-black trees
### 4.5.1 FSTM-based design
### 4.5.2 Lock-based designs
### 4.6 Summary

# 5. Implementation issues
## 5.1 Descriptor identification
## 5.2 Storage management
### 5.2.1 Object aggregation
### 5.2.2 Reference counting
### 5.2.3 Epoch-based reclamation
## 5.3 Relaxed memory-consistency models
### 5.3.1 minimal consistency guarantees
### 5.3.2 Memory barriers
### 5.3.3 Inducing required orderings
### 5.3.4 Very relaxed consistency models
## 5.4 Summary

# 6. Evaluation
## 6.1 Correctness evaluation
## 6.2 Performance evaluation
### 6.2.1 Alternative lock-based implemantations
### 6.2.2 Alternative non-blocking implementations
### 6.2.3 Results and discussion
## 6.3 Summary

# 7. Conclusion
## 7.1 Summary
## 7.2 Future research
