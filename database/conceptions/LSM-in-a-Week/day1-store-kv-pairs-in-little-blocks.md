# What is Block in a LSMT

![diagram](https://github.com/SteveLauC/pic/blob/main/lsmt-SST.jpeg)

You can see that an SST is a collection of blocks, and block is the minumum unit
of read and caching in LSMT, which is usually 4KB in size, similar to the database
pages. In each block, we store a collection of sorted kv pairs.

## Internal representation of a block

```rust
/// A block is the smallest unit of read and caching in LSM tree. It is a 
/// collection of **sorted** key-value pairs.
pub struct Block {
    data: Vec<u8>,
    offsets: Vec<u16>,
}
```

```
|          data         |           offsets       |
|entry|entry|entry|entry|offset(2B)|offset(2B)|...|num_of_elements(2B)|
```

This is how an entry (kv pair) stored in the data section. `key_len` and 
`value_len` are 2 bytes long, which means the lengths of `key` and `value` 
are 65535 at most. We assume that key is not empty but a value can be empty,
an mepty value means that the corresponding key has been deleted (**tombstone**)

```
|                             entry1                            |
| key_len (2B) | key (varlen) | value_len (2B) | value (varlen) | ... |
```

At the end of this block, we will store the offsets of each entry and the total
number of entries. For example, if there are 2 entries in this block and the
first entry is at 0th position of this block, and the second is at 12th posion:

```
|          data         |           offsets         |
|entry|entry|entry|entry|0     |12    |...   |...   |2               |
```

The block has a size limit, which is called `target_size`. Unless the first 
key-value pair exceeds the target block size (?), you should ensure that the 
encoded block size is always less than or equal to `target_size`.
