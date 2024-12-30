> NOTE: the main reason I read this paper is that Elasticsearch's replication 
> algorithm is inspired by this paper.

> TOC
>
> * 1. Introduction
> * 2. PacificA replication framework
>
>   > This section describes the replication framework
>
>   * 2.1 Primary/Backup Data Replication
>   * 2.2 Configuration Management
>   * 2.3 Leases and Failure Detection
>   * 2.4 Reconfiguration, Reconciliation, and Recovery
>   * 2.5 Correctness
>   * 2.6 Implementations
>   * 2.7 Discussions
> * 3. Replication for Distributed log-based storage systems
>
>   > This section presents different approaches that the framework can be applied
>   > to a general log-based storage system.
>
>   * 3.1 Logical Replication
>   * 3.2 Layered Replication 
>   * 3.3 Log merging
> * 4
> * 5
> * 6

# 1. Introduction

1. Pacific A is strongly consistent

2. There are 3 different replication strategies in Pacific A

   QUES: need details

3. Pacific A has the following features

   1. The separation of replica group configuration management from data 
      replication, with Paxos for managing configurations and primary/backup 
      data replication
   2. decentralized monitoring for detecting failures and triggering 
      reconfiguration, with monitoring traffic follows the communication 
      patterns of data replication
   3. a general and abstract model that clarifies correctness and allows different 
      practical instantiations.

# 2. PacificA replication framework

1. Term: *Configuration* of replica group

   The composition of a replica group, which indicates who the primary is and who the 
   replicas are, is referred to as the *configuration* of the replica group.

   > cluster nodes management

   Every time the *configuration* of a replica group changes(*reconfiguration*), 
   the *version* of the *configuration* gets increased by 1.

   > the configuration version is kinda similar to the term in Raft, though 
   > still different.

## 2.1 Primary/Backup Data Replication

1. Term

   * *query*: client request that does not modify data
   * *update*: client request that modifies data

2. Pacific A uses this Primary/Backup paradigm, all client requests are sent to
   the primary, *query* will be handled locally, and processing *update*s require
   involving replicas.

   > This is different from what Elasticsearch does, in Elasticsearch, *queries*
   > can be processed on replicas.
   >
   > But, as a replication algorithm, do you need to care about how reads are handled?
   
3. Every *update* request will be assigned a continuous and monotonic serial 
   number by the primary, then the request, along with its serial number and 
   the configuration version, get replicated to all the replicas.

3. term: replication message is called *prepare* in Pacific A
  
   > *AppendEntires* in Raft

4. term: *prepared list*

   Every time a replica receives a *prepare* request, it put the received *update*
   request in its *prepared list* following request's serial number, so this is an
   ordered list.

   This list has a *committed point*, the requests before the *committed point* 
   are applied by the replica. This part of *prepared list* is called *committed
   list*

5. Write flow

   1. Primary receives an *update* request, assigns it a serial number 
   2. Send *prepare* requests to all the replicas (contains *configuration verion* and *serial number*)
   3. Replica acknowledge the request and send a *prepared* message to the primary
   4. Once primary receives *prepared* messages from all the replicas, this client 
      request is then considered *committed*
   5. The primary can inform replicas that *committed* gets bumped, you can adjust
      your *committed point* accordingly

   In Pacific A, a request becomes *committed* only if it has been written by 
   all the replicas, which is different from what Raft does, Raft only requires
   quorum write.

## 2.2 Configuration Management
## 2.3 Leases and Failure Detection
## 2.4 Reconfiguration, Reconciliation, and Recovery
## 2.5 Correctness
## 2.6 Implementations
## 2.7 Discussions
# 3. Replication for Distributed log-based storage systems
## 3.1 Logical Replication
## 3.2 Layered Replication 
## 3.3 Log merging
# 4
# 5
# 6