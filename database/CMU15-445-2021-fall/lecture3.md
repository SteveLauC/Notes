1. disk-oriented dbms arch
   
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2011-15-31.png)

2. problems we are gonna figure out 
 
   1. how the dbms represents the database in the disk file(letcture 3)
   2. how the dbms manages its memory and move data back and forth from the disk
   (between the disk file and the buffer pool)(lecture 4)

3. file layout(what the pages look like inside the disk file)

   
   1. Some dbms stores the whole database as a single file(like sqlite), whereas
   others store things across multiple files cuase there is a max file limitation
   and you don't wanna hit it.

   2. The OS knows nothing about the database disk file, it is just a normal binary
   file.

   3. Page/block(Database system page, not the OS/hardware page) is a fixed-size
   block of data. It can contain tuples, metadata, indexes, logs. But most systems
   don't mix the different types of data within a page(i.e if one page contains a
   tuple, and all the stuff this page has are tuples)

   4. Each page has a unique Id which enables the DBMS to map the Id to the phycial
   location(indirection layer allowing us to move the phycial data freely, e.g. 
   compact the data)

	
   #### Heap file organization
   A heap file is a collection of unordered pages where tuples are stored in a 
   random order. It needs some metadata to keep track of what pages exist and 
   which ones have free space

   A great way to represent heap file is using `page directory`

   #### Page directory(exists in both memory and disk, see arch above)
   The database maintains a `special page` that tracks the location of data pages
   in the database files. The directory also reocrds the number of free slots per
   page. The DBMS has to ensure that the directory page has to be in sync with
   the data pages

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-25-21.png)


4. page layout
    
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
   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-46-17.png)

   * slot array(indirection layer): mapping layer from the particular slot to
   the start location of the tuple you want which enables us to freely move 
   the tuple inside the page

   * free space

   * data

   ![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2013-47-39.png)

   Slot array grows from the beginning to the end and the data grows from the end
   to the beginning. When the borders of slot array and data meet, then we can
   say this page is full.

5. tuple layout(how to represent a record inside the page, fixed-length reocrd 
   and variable-length record)

   A tuple is essentially a sequence of bytes, it's the job of the DBMS to 
   interpret those bytes into attributes types and values.

   ![demo](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2014-48-06.png)

   * header: 
   	1. visibility info(concurrency control)
	2. bitmap to indeicate NULL values

   * data: attributes are typically stored in the order that specify when you 
   create the table
   	
	![demo](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2014-52-55.png)

6. storage engine(manager) is a dbms component that is responsible for maintaining
   the disk file(s)

7. what will DBMS do when query happens? Say, we want the information of the student
   whose name is steve.

   DBMS will first fetch a `record id` from index, which is a pair of `page_id, 
   solt_id`, then it will ask the `page directory` to give the the location of 
   that page, after knowing this, it will ask the `slot array` to give it the
   starting location of that tuple, then the DBMS get it.

   ![demo](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202022-07-18%2014-28-13.png)

   For example, in postgresql, we have a field `ctid` which is exactly this stuff
