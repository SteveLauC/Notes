> This chapter is mainly about:
>
> * How SQLite organizes tables into separate B+Trees and indexes into 
>   separate B-Tree
>
> * How trees are constructed in database files, and tuples are inserted and 
>   deleted, respectively, into and from the trees
>
> * B-Tree and B+Tree structures and algorithms to manipulate them
> * structure of internal, leaf and overflow pages

> * 6.1 Preview
> * 6.2 The Tree Interfeace Functions
> * 6.3 B+Tree Structure
>   * 6.3.1 Operations on B+Tree
>     * 6.3.1.1 Search
>     * 6.3.1.2 Search next
>     * 6.3.1.3 Insert
>     * 6.3.1.4 Delete
>   * 6.3.2 B+Tree in SQLite
> * 6.4 Page Structure
>   * 6.4.1 The Page Structure
>     * 6.4.1.1 Structure of page header
>     * 6.4.1.2 Structure of storage area
>     * 6.4.1.3 Structure of a cell
>   * 6.4.2 Overflow page structure
> * 6.5 The Tree Module Functionalities
>   * 6.5.1 Control data structures
>     * 6.5.1.1 Btree structure
>     * 6.5.1.2 BtShared structure
>     * 6.5.1.3 MemPage structure
>     * 6.5.1.4 BtCursor structure
>     * 6.5.1.5 Integrated control structures
>   * 6.5.2 Space management
>     * 6.5.2.1 Management of free pages
>     * 6.5.2.2 Management of page space

# 6.1 Preview
# 6.2 The Tree Interfeace Functions
# 6.3 B+Tree Structure
## 6.3.1 Operations on B+Tree
### 6.3.1.1 Search
### 6.3.1.2 Search next
### 6.3.1.3 Insert
### 6.3.1.4 Delete
## 6.3.2 B+Tree in SQLite

1. A tree is allocated by allocating its root page, a root page won't be relocated.

   Each tree is identified by its root page number, this number is stored in the
   `sqlite_master` (`sqlite_schema`) table, table `sqlite_master`'s root page will
   always be page 1 (the first page)

2. For a table B+Tree, internal nodes are just for navigation, the actual data is
   stored in leaf nodes.

   For an entry (A row), the key and data form the *payload*.

   > From the official document: https://www.sqlite.org/fileformat.html 
   > The payload of a leaf table node does not contain the key, is ONLY about
   > the arbitray lengh part.

3. A typical B+Tree in SQLite

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-10-20%2017-48-57.png)

# 6.4 Page Structure

> A SQLite database file consists of multiple pages, each page can be one of the 
> following types:
>
> 1. free
> 2. tree
> 3. pointer-map
> 4. lock-byte
>
> For more infomation about page types, see 3.2.3
>
> In this section, we introduce the structures of:
>
> 1. internal
> 2. leaf
> 3. overflow
>
> tree pages.

1. Any page in a database file can be any types, except for the first page.
   The first page will always a B+Tree table page

   > It is actually the root page of the `sqlite_master` (`sqlite_schema`) table

## 6.4.1 The Page Structure

1. What is cell

   The logical content of each internal/leaf page is partitioned into what are
   called *cells*.

   * For a B+Tree internal node
     
     A cell is the key (ROWID) and its preceeding pointer
      
   * For a B+Tree leaf node
     
     A cell is the payload (row data)

     > I think the row data will include the ROWID of that row

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-10-20%2020-15-28.png)

2. Cells are units of space allocation and deallocation on tree pages

3. Every page consists the following components

   > except for the first page, it has a 100 bytes database file header.

   1. A page header
   2. Cell pointer array (one layer of indirection)
   3. Unallocated space
   4. Cell content area

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-10-21%2019-41-01.png)

   > [slotted page structure](https://github.com/SteveLauC/Notes/blob/main/database/CMU15-445-2022-fall/Lecture3_Database_Storage_I.md)

   cells are placed in order logically, not physically, the cell pointer array
   is sorted but the cell content are is not.

### 6.4.1.1 Structure of page header

> metadata for that page

| Offset | Size | Description |
|--------|------|-------------|
|0       |1     | The one-byte flag at offset 0 indicating the b-tree page type. A value of 2 (0x02) means the page is an interior index b-tree page. A value of 5 (0x05) means the page is an interior table b-tree page. A value of 10 (0x0a) means the page is a leaf index b-tree page. A value of 13 (0x0d) means the page is a leaf table b-tree page. Any other value for the b-tree page type is an error. |
| 1      |2     | The two-byte integer at offset 1 gives the start of the first freeblock on the page, or is zero if there are no freeblocks. |
|3       |2     |The two-byte integer at offset 3 gives the number of cells on the page. |
|5       |2     |The two-byte integer at offset 5 designates the start of the cell content area. A zero value for this integer is interpreted as 65536 (which can not fit into u16) |
|7       |1     |The one-byte integer at offset 7 gives the number of fragmented free bytes within the cell content area |
|8       |4     |The four byte-page number at offset 8 is the right-most pointer. This value appears in the header of interior b-tree pages only and is omitted from all other pages |


### 6.4.1.2 Structure of storage area

1. Cell pointer array comes directly after the page header, each pointer takes
   2 bytes number indicating the offset of corresponding content within this
   page.

2. The cell content area are **NOT** necessarily **continuous**, and these unused
   space is called *freeblock*, a freeblock is at least 4 bytes, the offsets of 
   these blocks are stored in a singly linked list in this page, in a ascending 
   order of their addresses, the first node of this list is stored in the page 
   header(offet 1, size 2)

   A freeblock is at least 4 bytes because these bytes are used to store metadata:

   1. first 2 bytes: offset of the next free block
     
      > 0 indicates there is no next freeblock
       
   2. second 2 bytes: size of this free block
    
      > including this 4 bytes metadata

   Unused space less than 4 bytes won't be stored in this freeblock list.

   The total size of these freeblocks is stored in the page header (offset 7 size 1), 
   since it only uses 1 byte, the maximum size is 255.

   
### 6.4.1.3 Structure of a cell

1. Structure of a cell


   > When you think that structures of a cell between leaf node and internal 
   > node are so different, and are curious how they do the conversion, well
   > they don't, in B+Tree, a leaf node will always be a leaf node, same applies
   > to internal node.
   >
   > Not sure about the B-Tree though.


   | Size | Description |
   |------|-------------|
   |4     |The preceeding child node ptr (omitted on leaf nodes)|
   |var(1-9)|Number of bytes of data (payload) (Omitted on index-tree page or internal table-tree pgae)|
   |var(1-9)|Number of bytes of key (Or the key itself if this is a table-tree page)|
   |*     |Payload      |
   |4     |Page number of the first overflow page (omitted if no overflow or in a internal table-tree page) |

   * a cell within a internal table-tree page:

      > Contains ONLY:
      > 1. key (ROWID)
      > 2. preceeding ptr

      > it does not have overflow pages

      | Size | Description |
      |------|-------------|
      |4     |The preceeding child node ptr |
      |var(1-9)|key (ROWID)|

   * a cell within a leaf table-tree page:

      > Contains ONLY:
      > 1. key (ROWID)
      > 2. payload length (Row data length)
      > 3. payload (Row data)
      > 4. first overflow page if exists

      | Size | Description |
      |------|-------------|
      |var(1-9)|Number of bytes of data (payload) |
      |var(1-9)|key (ROWID)|
      |*     |Payload      |
      |4     |Page number of the first overflow page (omitted if no overflow) |


   * a cell within a internal index-tree page:

     | Size | Description |
     |------|-------------|
     |4     |The preceeding child node ptr |
     |var(1-9)|Number of bytes of key |
     |*     |Payload (key, for an index b-tree, the key is always arbitrary in length and hence the payload is the key) |
     |4     |Page number of the first overflow page (omitted if no overflow) |

   * a cell within a leaf index-tree page:

     > For index-tree pages, internal pages and leaf pages are not that different,
     > with the ONLY difference that leaf nodes don't have pointers.

     | Size | Description |
     |------|-------------|
     |var(1-9)|Number of bytes of key |
     |*     |Payload (key, for an index b-tree, the key is always arbitrary in length and hence the payload is the key) |
     |4     |Page number of the first overflow page (omitted if no overflow) |

2. diagram

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-10-22%2009-30-24.png)

3. Variable-length interger (Varint)

   SQLite uses variant-length integers (varint) to represent integer types, 
   a varint is 1-9 bytes long, which can encode integers up to 64-bit long.

   This encoding is called huffman code, it is preferred by SQLite because it
   will only take 2 byes in most cases.

   A variable-length integer or "varint" is a static Huffman encoding of 64-bit 
   twos-complement integers that uses less space for small positive values. A 
   varint is between 1 and 9 bytes in length. The varint consists of either 
   zero or more bytes which have the high-order bit set followed by a single 
   byte with the high-order bit clear, or nine bytes, whichever is shorter. The 
   lower seven bits of each of the first eight bytes and all 8 bits of the ninth 
   byte are used to reconstruct the 64-bit twos-complement integer. Varints are 
   big-endian: bits taken from the earlier byte of the varint are more significant 
   than bits taken from the later bytes.

   > still have no idea how can I decode a varint, need to read SQLite's source 
   > code

4. How does payload fractions work

   > Page 165

## 6.4.2 Overflow page structure

The frist 4 bytes are used to store the pointer of the next overflow page

The remaining part is for content, and every overflow page except the last
one is completely filled with data of length equal to usable space minus
4 bytes

> QUES: How is this possible, what if a payload is only 1 byte long, then it
>       will only take 5 bytes

but an overflow page never stores content from two payloads.


# 6.5 The Tree Module Functionalities
## 6.5.1 Control data structures
### 6.5.1.1 Btree structure
### 6.5.1.2 BtShared structure
### 6.5.1.3 MemPage structure
### 6.5.1.4 BtCursor structure
### 6.5.1.5 Integrated control structures
## 6.5.2 Space management
### 6.5.2.1 Management of free pages
### 6.5.2.2 Management of page space
