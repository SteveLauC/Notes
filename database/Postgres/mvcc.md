# Snapshot

Every txn has a snapshot that defines a point-in-time view of the database, in 
Postgres, a snapshot contains 3 parts:

1. xmin: txn ID of the oldest active (not committed) txn
2. xmax: txn ID that is not assigned to any txns when this txn started
3. xip: txn ID of the active txns when this txn started

# Understanding

When a txn starts, all txns with ID in `[3, current_unassgined_xid]` have been
started. Within these txns, only the changes made by the committed ones are 
visible to the current txn, i.e., changes made by the active txns should be 
invisible, so a list of active txn IDs should be technically sufficient, that's
the part 3.

And part 1 and part 2 exist to avoid the scan of the `xip` array, which is slow.
All the txn with xid lower than `xmin` are committed, so they are visible to the
current tx. All the txn with xid greater than or equal to `xmax` have not been 
created when the crurrent txn got started, so the changes made by them are invisible.

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%202025-07-03%20at%202.40.46%E2%80%AFPM.png)