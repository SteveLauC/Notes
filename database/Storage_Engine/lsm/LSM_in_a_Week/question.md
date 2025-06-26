1. In my understanding, for databases that do not support txn, they have to
   fsync after every WAL write to ensure data won't be lost. If txn is 
   supported, then they can buffer and batch the WAL writes in memory and 
   write/fsync the WAL upon commit.

   In this tutorial, the LSM we are gonna implement supports txn, does it buffer
   WAL writes?

2. Why is MyRocks space-efficient?