# What we talk about in part 2

In part1, we talked about:

* Data model & Query language 
* Storage engine
* Encoding format

They are aspects that are essential for both non-distributed system and distributed
system, in part2, we talked about mostly about distributed system.

When do you need distributed system?

* Scalable
   
  Your workload cannot be hanlded on a single machine
  
* High availability

  When a machine (or multiple machines, or a data center) goes down, you don't
  want to lose your service 

* Latency 

  You want to split your service across multiple locations so that users can
  use the service that is closest to them.
  
1. The book discussed shared-disk architecute (Network Attached Storage, a.k.a., NAS)

   This architecture is used for some data warehousing workloads(AP), but contention
   and the overhead of locking limit the scalability of the shared-disk approach 
   
   > In 2024, we have many TP systems with this architecture
   
   > QUES: What does the "locking" refer to?
   
2. This book will focus on shared-nothing architecture, because this architecture
   requires the most caution from the application developer, they need to understand