# What is Apache Parquet

Apache Parquet is a free and open-source **column-oriented** data storage format in 
the Apache Hadoop ecosystem. It is similar to RCFile and ORC, the other 
columnar-storage file formats in Hadoop, and is compatible with most of the data 
processing frameworks around Hadoop. It provides efficient data compression and 
encoding schemes with enhanced performance to handle complex data in bulk.

# Logical Format

Parquest is expressed in schema that starts with the `message` keyword, every schema 
contains one or multiple fields, each of which has 3 associated attributes:
1. Repeatability
   * Required: means that this field occurs ONLY once
   * Optional: Occurrence: 0 or 1
   * Repeated: Occurrence: 0 or x

2. Type
   * Primitative types like:
     * `INT32`
     * `INT64`
     * `INT96`
     * `FLOAT`
     * `DOUBLE`
     * `BYTE_ARRAY`: byte slice
     * `BOOLEAN`:  1 bit boolean

   > The types supported by the file format are intended to be as minimal as 
   > possible, with a focus on how the types effect on disk storage.  
   >
   > Logical types are used to extend the types that parquet can be used to 
   > store, by specifying how the primitive types should be interpreted. 
   > For more information, see 
   > [LogicalTypes.md](https://github.com/apache/parquet-format/blob/master/LogicalTypes.md)
   > from their github repo.

   * group
     > group type is for nested structure, the support for nested structure is one
     > advantage parquet has over other formats like OCR.

3. Name

## Examples
Let's take a look at some examples:

Address Book

   Each address book has an owner, who may have more than 1 phone numbers. And
   for the contacts part, each of which is a structure like:

   ```rust
   struct Contact {
       name: String,
       phone: Option<String>,
   }
   ```

   To express Address Book in the schema of parquet:

   ```
   message AddressBook {
      required string owner,
      repeated string phone_number,

      repeated group Contacts {
          required string name,
          optional string phone,
      } 
   }
   ```
   
   We can see that nested structure like `Contacts` is supported.

# Phycial Format

Each parquet file consists of 3 parts:
1. Header
2. Data
3. Footer

![diagram](https://github.com/SteveLauC/pic/blob/main/apache_parquet_phycial_format.png)

```
// In the following example, there are N columns in this table, split into M row
// groups.

// Header
4-byte magic number "PAR1"

// Data
// Row Group 1
<Column 1 Chunk 1 + Column Metadata>
<Column 2 Chunk 1 + Column Metadata>
    ...
<Column N Chunk 1 + Column Metadata>

// Row Group 2
<Column 1 Chunk 2 + Column Metadata>
<Column 2 Chunk 2 + Column Metadata>
    ...
<Column N Chunk 2 + Column Metadata>

...

// Row Group M
<Column 1 Chunk M + Column Metadata>
<Column 2 Chunk M + Column Metadata>
    ...
<Column N Chunk M + Column Metadata>


// Footer
File Metadata: Contains the starting locations of all the <Column Metadata>
length in bytes of file metadata (4B)
magic number "PAR1" (4B)
```

# Phycial Format of The Data Part

![diagram](https://github.com/SteveLauC/pic/blob/main/parquet_phycial_format_simplified_version.png)

Hierarchically, a file consists of one or more row groups. A row group contains 
exactly one column chunk **per column**. Column chunks contain one or more pages.


1. Row Group
   A logical horizontal partitioning of the data into rows. There is no physical 
   structure that is guaranteed for a row group. 

   A row group consists of a column chunk for each column in the dataset.

2. Column Chunk

   A chunk of the data for a particular column. They live in a particular 
   row group and are guaranteed to be contiguous in the file.

   > Remeber that parquet is a columnar format
   >
   > ```
   > name age 
   > Steve 18
   > Miku  20
   > ```
   >
   > For example, the above table will be stored in a format like "SteveMiku2820". 
   > Multiple values of the same volumn are stored together and we name it as
   > `Column Chunk`.

   > You can infer that the amount of `Column Chunk` equals the number of Columns.

3. Page

   Column chunks are divided up into pages. A page is conceptually an indivisible 
   unit (in terms of compression and encoding). There can be multiple page types 
   which are interleaved in a column chunk.

# Metadata

## Metadata Hierarchy

1. File Metadata

   Type in `parquet`: `parquet::file::metadata::FileMetaData`
   
2. Row Group Metadat

   Type in `parquet`: `parquet::file::metadata::RowGroupMetaData`

3. Column Chunk Metadata
   
   Type in `parquet`: `parquet::format::ColumnMetaData`

> See the format diagram from section [Phycial Format](#phycial-format) for 
> more detail.

> `Parquet` has a type for all the metadata: `parquet::file::metadata::ParquetMetaData`.

## API

> All APIs are based on crate version 36.0.0.

### To set:

All the metadata can be set through:

```rust
parquet::file::properties::WriterPropertiesBuilder

set_xxx();
```

Once a `WriterProperty` is built, you can pass it to:

```rust
impl<W: Write> ArrowWriter<W> {
    pub fn try_new(
        writer: W,
        arrow_schema: SchemaRef,
        // pass it here
        props: Option<WriterProperties>
    ) -> Result<Self>
}
```

### To get:

1. From writer:

   When the write is doen, `.close(self)` the writer returns `Result<FileMetadata>`

2. From reader:

   1. From trait `FileReader`
  
      ```rust
      pub trait FileReader: Send + Sync {
          fn metadata(&self) -> &ParquetMetaData;
      }
      ``` 

      This trait is impled for `SerializedFileReader`.

   2. `ParquetFileArrowReader` has a method `metadata`

      ```rust
      pub fn metadata(&self) -> &ParquetMetaData
      ```

   3. `parquet::file::footer::parse_metadata()`
     
      ```
      pub fn parse_metadata<R: ChunkReader>(
          chunk_reader: &R
      ) -> Result<ParquetMetaData>
      ```

      trait `ChunkReader` is impled for `std::fs::File`.



## Some Metadata you may be intersted in

1. Writer Version (Parquet Format Version)

   API to set this Metadata:

   ```
   set_writer_version(self, value: WriterVersion) -> Self
   ```

   > One should note that parquet does not provide a metadata to store the 
   > version info about schema.
   >
   > To achive this, set a customized key value pair:
   > ```
   > { key: "SchemaVersion", value: "1" }
   > ```

2. created_by

   API to set this Metadata:

   ```rust
   pub fn set_created_by(self, value: String) -> Self
   ```

# Statistic Info
`parquet` stores statistics data at the following levels:

* ColumnChunk
* Page

```rust
/// Statistics for a column chunk and data page.
#[derive(Debug, Clone, PartialEq)]
pub enum Statistics {
    Boolean(ValueStatistics<bool>),
    Int32(ValueStatistics<i32>),
    Int64(ValueStatistics<i64>),
    Int96(ValueStatistics<Int96>),
    Float(ValueStatistics<f32>),
    Double(ValueStatistics<f64>),
    ByteArray(ValueStatistics<ByteArray>),
    FixedLenByteArray(ValueStatistics<FixedLenByteArray>),
}

/// Statistics for a particular `ParquetValueType`
#[derive(Clone, Eq, PartialEq)]
pub struct ValueStatistics<T> {
    min: Option<T>,
    max: Option<T>,
    // Distinct count could be omitted in some cases
    distinct_count: Option<u64>,
    null_count: u64,

    /// If `true` populate the deprecated `min` and `max` fields instead of
    /// `min_value` and `max_value`
    is_min_max_deprecated: bool,

    /// If `true` the statistics are compatible with the deprecated `min` and
    /// `max` fields. See [`ValueStatistics::is_min_max_backwards_compatible`]
    is_min_max_backwards_compatible: bool,
}
```

The supported statictis info are:
* min
* max
* distinct count
* null count

