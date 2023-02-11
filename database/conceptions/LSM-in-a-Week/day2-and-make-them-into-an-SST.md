# Sorted String Table

An SST consists of two parts:
1. Some data blocks
   > block is covered in day1
2. Some index blocks (meta blocks)

> Generally, an SST file is of 256MB in size.


# Encoding of SST

```
| data block | .. | data block | meta block | .. | meta block | meta block offset (u32) |
```

Every `data block` has a corresponding `meta block` for metadata storage, which 
keeps track of the offset and the first key of that `data block`.

```rust
pub struct BlockMeta {
    /// Offset of this data block.
    pub offset: usize,
    /// The first key of the data block.
    pub first_key: Bytes,
}
```

```
// Encoding of a meta block
|            meta block               |
| offset (usize) | first_key (varlen) |
```
