1. Internal structure of `HashMap`

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-05-18%2019-43-48.png)

   A hashmap consists of an array of `Bucket`s, each `Bucket` is a list of key-value
   pairs.


   ```rust
   #[derive(Debug)]
   struct Bucket<K, V> {
       entries: Vec<(K, V)>,
   }


   #[derive(Debug)]
   pub struct HashMap<K, V> {
       buckets: Vec<Bucket<K, V>>,
       n_entries: usize,
   }
   ```

2. Insertion

   Every key will be hashed into a small number, and we use this hash number to 
   mod `number_of_buckets` to get the bucket index of the bucket where this entry
   will be put.

3. O(1) Complexity & load factor

   To maintain a O(1) complexity of lookup, the amount of entries in a bucket
   needs to be small (so that iterating can be done in constant time).

   `Total amount of entries` / `amount of buckets` is called `load factor`, and
   theoretically, to get the best performance of hashmap, `load factor` should
   be kept under `0.75`.

   > Every bucket should have 0.75 kv pair.

   With more entries inserted into the hashmap, your `load factor` can exceed
   `0.75`, at this time, you need to double the buckets.

   [code: when to rehashing](https://github.com/jonhoo/rust-basic-hashmap/blob/b36f7b7375e2fe8de1bd0991540b8f6bdeec9a6b/src/lib.rs#L109)

4. Rehashing

   Doubling the bucket means you need to create another `HashMap` that is twice
   large as the previous one, and re`insert` the existing entries to the new 
   bucket.

   This is **quite expensive**, we should try to avoid this as much as possible.
