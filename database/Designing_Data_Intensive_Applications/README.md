1. What does data-intensive mean? We alreay know CPU-intensive and IO-intensive,
   CPU-intensive means the application's bottleneck is mostly CPU, thus we need
   more cores and threads, IO-intensive means the bottleneck is I/O, we need async
   runtime and the underlying poll/epoll/aio/kqueue technique.
   
   Data-intensive means that the application's bottlenecks are usually:
   
   * How data is stored
   * How data is processed
   
   > We are having more and more cores, we need something like "Building multi-core
   > applications".
   
2. This book is not exactly for database developers, it is for application
   developers, but you cannot design a good app wihtout learning about the tools
   you use, so it will cover some DB internal stuff.
   
   I think the reason why it is so popular among database developers is that
   it guides and navigates you through the all the important stuff you need
   to know to become a DB developer:
   
   * Data model
   * Query language
   * Storage engine
   * Data organization on disk
   * Data replication
   * Partition/sharding
   * transactions
   * distributed systems
   * How consisitency is achieved (consensus algorithm)
   * How to combine multiple components
   * Steam processing
   