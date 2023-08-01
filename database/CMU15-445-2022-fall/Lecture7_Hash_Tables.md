> Today's Agenda
>
> * Hash Functions
> * Static Hashing Schemas
> * Dynamic Hashing Schemas

1. Cource Status

2. While implementing a HashMap, what you need to consider:
 
   1. Hash function

      Trade-off:

      1. It always returns a constant, very fast, with collision.
      2. It is a perfect hash function that won't make collision, but needs a long
         time to compute.

   2. Hash schema: how to handle collision.

      Trade-off:

      1. Allocate an extremely giant array to avoid collision.
      2. Small array, execute additional instruments when collision happens.

3. The space complexity of HashTable is `O(n)`, where n the amount of items stored
   in this table.

   In practice, it can be `2n`, `3n` or `4n`, depending on the hash schema we use.

   > We pay more space for maintaining average `O(1)` time complexity
   
   > Another thing is, in the world of database, we care about `O(1)`, theoretical
   > result is different from the phycial one.

## Hash Functions

1. A hash function needs to be deterministic, i.e., for the same key, it always 
   generates the same result.

2. The DBMS need **NOT** to use a cryptographically secure hash function (e.g., SHA-256)
   because we do not need to worry about protecting the contents of keys. These 
   hash functions are primarily used internally by the DBMS and thus information 
   is not leaked outside of the system. In general, we only care about the hash
   function’s **speed and collision rate**. 

   The current state-of-the-art hash function is Facebook 
   [XXHash](https://github.com/Cyan4973/xxHash).

   And the xxhash3 is the fastest one in these xxhash functions.
 
## Static Hashing

1. Linear Probe Hashing

   * Data structure

     An fixed-sized array

   * Insert

     1. Get the hash value: `idx = hash(key)`
     2. Iterate over the array starting from `idx`, if the slot is empty, 
        insert and return.
     3. Or we keep searching the array (in cyclic way), insert it to the next 
        empty slot.

   * Get

     1. Get the hash value: `idx = hash(key)`
     2. Iterate over the array starting from `idx`, if we have searched all the
        slots or **reaches an empty slot**, return `None`.
     3. Else, return `Some(value)`
    
   * Remove

     > We cannot simply remove the `(Key, Value)` from the array as this will
     > break `Get` (`Get` will stop when it reaches an empty slot).

     1. Use the steps in `Get` to find the key 
     2. These are two approaches on how to "remove" this key from the table

        1. Update its value to a tombstone
           
           > Almost everyone does this.
           
           > With this, when inserting, you can insert to a slot that has a 
           > tombstone, and you may need periodic garbage collection (rehash 
           > to clear the tombstone)

        2. You back shift the items after the deleted one to fill the empty slot
           until an empty slot is met.

           > No one does this in reality.

           > In the most extreme case, these item's hash location is exactly
           > the deleted one, shift them back by one slot, making the first
           > item in its hash location and other items behind their hash 
           > location, this is totaly correct.

   * How to handle unique keys 

     1. Put them in a linked list
     2. Just treat them as normal key values

2. Robin Hood Hashing

   > What does Robin Hood mean:
   >
   > A person who takes or steals money from rich people and gives it to 
   > poor people

   This is a variant of `Linear Probe Hashing`, each item in the hashtable also
   stores the distance they are from their hash location, an item that has a lower
   distance is considered richer.

   The implmentation is basically the same as `Linear Probe Hashing` except for
   the `insert` operation.
   
   * Insert

     1. Get the hash location `idx = hash(key)`
     2. If the hash location is empty, insert and return.
     3. Keep iterating until an empty slot is met
        
        1. If a slot's distance is smaller than the new item's, replace the item
           in this slot with the new one. And find a new further place for the 
           item that has just been replaced.


3. Cuckoo Hashing

   * Insert
   * Get
   * Remove

## Dynamic Hashing

> Static Hashing has fixed capacity, which means the DBMS has to be clear about
> how many items it wants to store. Otherwise, it has to rebuild the whole 
> hashtable.
>
> Dynamic hashing, can resize the hashtable at runtime without need to rebuild
> the whole table.

1. Chained Hashing
2. Extendable Hashing

   See 
   [Advanced Indexing Techniques](https://github.com/SteveLauC/Notes/blob/main/database/Database_System_Concepts/Ch24_Advanced_Indexing_Techniques.md)

3. Linear Hashing

   
