## Message queue

## Semaphore Set

1. Semaphores are allocated in a group, every semaphore in a group is a **counting 
   semaphore**.

2. APIs

   1. semget()

      ```c
      #include <sys/sem.h>

      int semget(key_t key, int nsems, int semflg);
      ```

      Create or open a semaphore set, which will have `nsems` semaphores crated 
      in it, return a non-negative semaphore set indentifier.

      Per the POSIX standard, when a set is created, the semaphores in it won't
      be initialized (dirty values in memory). The Linux implementation, like 
      other implementations, will initialize them to 0.

      Initialization can be done using `semctl(2)` `SETVAL` or `SETALL` operation.
      Where multiple peers do not know who will be the first to initialize the set, 
      checking for a nonzero (means it is not initialized yet) `sem_otime` in the 
      associated data structure retrieved by a `semctl(2)` `IPC_STAT` operation 
      can be used to avoid races.

      Generating the `key` argument can be done via `ftok(3)`:

      ```c
      #include <sys/ipc.h>

      key_t ftok(const char *pathname, int proj_id);
      ```

      Though we should note that this function is based on i-node, which means 2 
      different files with the same i-node number should give the same result. 
      (if given the same `proj_id`)

      The `sem_flg` arugment can be any value ORed from the below values:

      1. IPC_CREAT 
          
         Used in `semget(2)`, if a semaphore set associated with `key` does not exist.

      2. IPC_EXCL 

         Used in `semget(2)`, should be used with `IPC_CREAT`, when used together,
         ensure that this create operation will create a new semaphore set.

         > exclusive create

      3. Permission values
         
         ```c
         S_IRUSR
         S_IWUSR
         S_IXUSR
         S_IRGRP
         S_IWGRP
         S_IXGRP
         S_IROTH
         S_IWOTH
         S_IXOTH
         ```

   2. semop()

      ```c
      #include <sys/sem.h>

      int semop(int semid, struct sembuf *sops, size_t nsops);
      int semtimedop(int semid, struct sembuf *sops, size_t nsops, const struct 
                     timespec *_Nullable timeout);
      ```

      Perform `nsops` operations stored in `sops` to the semaphores stored in the
      set specified by `semid` (**sequentically** and **atomically**).

      > Every semaphore has the following properties:
      >
      >     ```c
      >    // the value of this semaphore
      >    unsigned short  semval; 
      >
      >    // the number of threads (they have performed an operation where `sem_op` is 0)
      >    // 
      >    // that have been waiting for this semaphore to become zero
      >    unsigned short  semzcnt; 
      >
      >    // the number of threads (they have performed an operation where `sem_op` is negative)
      >    // 
      >    // that have been waiting for this semaphore to increase
      >    unsigned short  semncnt;  
      >
      >    // PID of process (rather than thread) that last modified the semaphore value
      >    pid_t           sempid;   
      >    ```

      `struct sembuf` is defined as follows:

      ```c
      // a semaphore can be as an ordered array, this is the index of the semaphore
      // to which the operation will be performed.
      unsigned short sem_num;
      /// semaphore operation
      short          sem_op; 
      /// operation flags
      short          sem_flg;
      ```

      There are 3 kinds of operations depends on whether `sem_op` is positive, 
      0, or negative:

      1. Increase (if `sem_op` is positive)
      2. WaitForZero (if `sem_op` is 0)
      3. TryToDecrease (if `sem_op` is negative)


   3. semctl()

      ```c
      #include <sys/sem.h>

      // Performs `cmd` on the `sumnum`-th semaphore of the set identified by `semid`
      int semctl(int semid, int semnum, int cmd, ...);
      ```

## Shared memory segments
