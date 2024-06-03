1. disk-oriented dbms arch
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2011-15-31.png)

2. volatile and non-volatile storage

   * volatile storage is **byte-accessible**, which means that we can jump to 
     any byte address and get the value there.

   * non-volatile storage is **page/block-accessible**. If we wanna fetch the 
     data at particular offset, we have to fetch the whole page/block containing
     that byte

3. System Design Goal

   * Allow the DBMS to manage databases that exceed the amount of memory available. 
   * Minimize the disk write/read
   * Maximize the sequential read/write

4. Why does't DBMS use the OS kernel buffer
   
   1. Transaction safety

      The OS can flush dirty pages at any time.

      For a transaction in DBMS, we want to flush dirty pages only if the 
      transaction has been committed.

   2. I/O stalls (block thread)

      DBMS has no idea which page are in the memory, if a thread tries to access
      a page that is not in, this blocked gets blocked for page fault.

   3. Error handling

      Difficult to validate pages(checksum, maintained outside of the page). Any 
      access can cause a SIGBUS that the DBMS must handle.

   4. Performance

      For a large database, the data structure used in the OS can be the bottleneck.

   > The DBMS knows better than the OS.

   > [Are You Sure You Want to Use MMAP in Your Database Management System?](https://db.cs.cmu.edu/mmap-cidr2022/)

4. Problems we are gonna figure out in this class
 
   1. how the dbms represents the database in the disk file(lecture 3)
   2. how the dbms manages its memory and move data back and forth from the disk
   (between the disk file and the buffer pool)(lecture 5)

5. File layout(what the disk file looks)
   
   1. Some dbms stores the whole database as a single file(like sqlite), whereas
   others store things across multiple files cause there is a max file size 
   limitation and you don't wanna hit it.

   2. The OS knows nothing about the database disk file, it is just a normal binary
   file.

   3. Page/block(Database system page, not the OS/hardware page) is a fixed-size
   block of data. It can contain tuples, metadata, indexes, logs. But most systems
   don't mix the different types of data within a page(i.e if one page contains a
   tuple, and all the stuff this page has are tuples)

   > 1. The hardware page, is the smallest unit that the disk can guarantee that
   >    read/write is atomic. (Either finished or not started)
   > 2. Bigger database page means more sequential I/O, for example, in MySQL, 
   >    page is 16KB, a write of such a page would result in 4 hardware page
   >    writes. But a bigger page makes more wasted I/O, for example, if I only
   >    want a tuple (say it is 20B), the almost all the 16KB is wasted. There
   >    is tradeoff.

   4. Each page has a unique ID which enables the DBMS to map the Id to the physical
      location. If the database is a single file, then ID can just be the page 
      offset (indirection layer allowing us to move the physical data freely, e.g.
      compact the data)

      > Most systems map PageID to (path, offset)


	
   #### Heap file organization
   A heap file is a collection of unordered pages where tuples are stored in a 
   random order. It needs some metadata to keep track of what pages exist and 
   which ones have free space

   Supported Operations:
   * Create a Page
   * Get a Page
   * Write a Page
   * Delete a Page
   * Iterating over all the pages

   A great way to represent heap file is using `page directory`

   #### Page directory(exists in both memory and disk, see diagram above)
   The database maintains a `special page` that tracks the location of data pages
   in the database files. The directory also records the number of free slots per
   page. The DBMS has to ensure that the directory page has to be in sync with
   the data pages

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-25-21.png)


6. page layout
    
   Every page has a header which contains the metadata of this page's contents:
   * page size
   * check sum
   * DMBS version
   * transaction visibility(about safe access)
   * compression info

   How to place tuples inside a page? The most common way to do this is called
   `slotted pages`. but before diving into it, let's just begin with the bad 
   tuple oriented one: simply insert the tuple to the page

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-53-16.png)

   The drawback: 
   1. It assumes(is only suitable for) the cases where the tuples are all 
   fixed-length. For example, in the above illustration, `Tuple 2` is deleted, 
   and then we are going to insert a new tuple which is bigger or smaller than 
   the space previously used by `tuple 2`, then we will get some segments.
   2. If we wanna know where the free spaces of this page are, we are gonna 
   sequently scan the whole page to get the answer, which is not that convenient

   #### slotted pages

   > For the structure of a page, there are two kinds of formats:
   > 1. tuple oriented (slotted-pages belongs to this)
   > 2. log-structured

   > SQLite uses this format, see 
   > [Storage of the SQL database schema](https://www.sqlite.org/fileformat.html#storage_of_the_sql_database_schema)
   >
   > A b-tree page is divided into regions in the following order:
   >
   > * The 100-byte database file header (found on page 1 only)
   > * The 8 or 12 byte b-tree page header
   > * The cell pointer array
   > * Unallocated space
   > * The cell content area
   > * The reserved region.
   >
   > PostgreSQL also uses this, but it does not employ a B-Tree, it simply uses the
   > heap organization: [doc](https://www.postgresql.org/docs/current/storage-page-layout.html)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-46-17.png)

   * slot array(indirection layer): mapping layer from the particular slot to
     the start location of the tuple you want which enables us to freely move 
     the tuple inside the page

   * free space

   * data(actual tuples)

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-47-39.png)

   Slot array grows from the beginning to the end and the data grows from the end
   to the beginning. When the borders of slot array and data meet, then we can
   say this page is full.

7. Record ID: (PageID, slot/offset)

   Each tuple is assigned a RecordID so that the DBMS can keep track of it. In 
   different system, this ID has different names, in PostgreSQL, it is called
   `ctid`, in MySQL and SQLite, it is called `ROWID`.

   Let's try this in PostgreSQL:

   ```SQL
   steve@(none):steve> select ctid, * from person;
   +-------+-----------+-----+
   | ctid  | name      | age |
   |-------+-----------+-----|
   | (0,1) | steve     | 1   |
   | (0,2) | mike      | 2   |
   | (0,4) | m         | 2   |
   | (0,5) | UPPERCASE | 1   |
   +-------+-----------+-----+
   ```

   You can even query with this system column:
   ```SQL
   steve@(none):steve> select ctid, * from person where ctid = '(0,1)';
   +-------+-------+-----+
   | ctid  | name  | age |
   |-------+-------+-----|
   | (0,1) | steve | 1   |
   +-------+-------+-----+
   ```
   But this is highly not recommended as PostgreSQL can alter it at any time.

8. tuple layout(how to represent a record inside the page, fixed-length record 
   and variable-length record)

   > The class does not cover how to store variable-length tuples while the 
   textbook does. Page 592

   A tuple is essentially a sequence of bytes, it's the job of the DBMS to 
   interpret those bytes into attributes types and values.

   ![demo](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2014-48-06.png)

   * header: 
     1. visibility info(concurrency control)
	  2. bitmap to indicate NULL values

   * data: attributes are typically stored in the order that specify when you 
     create the table
   	
	![demo](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2014-52-55.png)

9. Storage engine(manager) is a DBMS component that is responsible for maintaining
   the disk file(s)

10. What will DBMS do when query happens? Say, we want the information of the student
    whose name is steve.

    DBMS will first fetch a `record id` from index, which is a pair of `page_id, 
    slot_id`, then it will ask the `page directory` to give the the location of 
    that page, after knowing this, it will ask the `slot array` to give it the
    starting location of that tuple, then the DBMS gets it.


11. Two indirection layers
   1. Page ID in directory page
   2. Slot ID in slot array
