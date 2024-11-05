> This chapter is about:
>
> * Before I read it (Look at the TOC)
>   * Encoding: Various encoding formats, their pros and cons
>   * Evolution: no idea about this
> * After I read it
>   * Introduces some textual formats: XML/JSON/CSV
>   * Introduces some binary formats: Thrift/Protobuf/Avro
>   * How these binary formats handle backward and forward compatibility

> What do you expect to learn from it (fill in this before reading it)
>
> * If you are new to it, ...
>   * What does "language-specific format" mean?
>   * Intro to XML
>   * What are the problems with JSON as a data encoding format (Too sparse?)
>     Yes
>   * Intro to Apache Thrift
>   * Intro to Protobuf
>   * Intro to Apache Avro 
>   * The whole section of "Modes of Dataflow"
>
> * If you already knew something about it, ... 
>   > tip: think more and read less in this case

> What have you learned from it
>
> * What are backward compatibility and forward compatibility
> * Well, so many programming languages have their specific serialization formats
> * Json is too sparse, and we have a lot of binary formats for JSON
>   * MessagePack
>   * BSON
>   * BJSON
>   * UBJSON 
>   * BISON
>   > XML has a lot of binary formats as well
> * Thrift and Protobuf use field tags instead of field names, which enables you
>   to flexibly change field names, or swap 2 field names. It is a layer of 
>   indirection, the OID of Postgres is kinda similar.
>   Another benefit of doing so is field tags take less space so that your encoding
>   is more compact.
> * ASN.1 looks cool


 TOC
>
> * Formats for Encoding Data
>   * Language-Specific Formats
>   * JSON, XML, and Binary Variants
>     * Binary encoding
>   * Thrift and Protocol Buffers
>     * Field tags and schema evolution
>   * Avro
>     * The writer's schema and the reader's schema
>     * Schema evolution rules
>     * But what is the writer's schema?
>     * Dynamically generated schemas
>     * Code generation and dynamically typed languages
>   * The merits of Schemas
> * Modes of Dataflow
>   * Dataflow Through Databases
>   * Dataflow Through Services: REST and RPC
>   * Messsage-Parsing Dataflow
> * Summary 

1. What are backward compatibility and forward compatibility?
   
   * Backward compatibility
     
     Newer code can read data that was written by older code
     
   * Forward compatibility
     
     Older code can read data that was written by newer code. 
     
   > These defs not only apply to data format, it also apply to configuration.

# Formats for Encoding Data

1. A program typically need to interact with 2 kinds of representations of
   data:
   
   1. In memory, data is kept in structs, vectors, hashmaps, trees, where they 
      can be connected via pointers.
   2. When you write data to disk or send it over network, you will serilize the 
      in-memory objects into a self-contained format.
      
   From 1 to 2, this is called serialization/encoding, the reverse process is
   called deserialization/decoding.

## Language-Specific Formats

1. Various programming languages have their own built-in support for de/serialization

   * Java: `java.io.Serializable`: The format does not have a name, nor a fixed 
     specification.
     
   * Python: pickle, a binary format specific to Python
   
   * Ruby: Marshal
   
   They are bad because:
   
   * They are language-specific
   * Compatibility is bad
     
   > Well, I think serde is amazing now since it can be used for many formats, even
   > binary formats. 
   
   So, do not use them!

## JSON, XML, and Binary Variants

1. Issues of XML

   1. You cannot distinguish numbers and a string with a dot
    
      ```xml
      <?xml version="1.0" encoding="UTF-8" ?>
      <string>1.1</string>
      <number>1.1</number>
      ```
      
   2. Does not support binary string (i.e., bytes type) 
   
   3. Optional schema support

2. Issues of JSON

   1. Json does not dintinguish intergers and floats
   2. Does not support binary string (i.e., bytes type) 
   3. Optional schema support
   
3. Issues of CSV

   1. Type-less
   
### Binary encoding

1. When the amount of data you need to enchange reaches the scale of terabytes,
   and you don't need the format to be human-readable, binary format that is
   more compact and faster to parse should be your choice.
   
2. JSON has several binary formats

   * MessagePack
   * BSON (by MongoDB)
   * BJSON
   * UBJSON
   * BISON
   
## Thrift and Protocol Buffers

1. Apache Thrift and protobuf both don't store field names, instead, they use
   field tags (numbers) to make the encoding more compact.
   
   This can also be seen as "a layer of indirection", with field tags, you can
   change the field name. (This kinda makes me understand why Postgres gives
   every object an `OID`).
   
   > QUES: Cap'n proto uses field number as well, does it store field names?
   
   Thrift:
   
   ```
   struct Person {
     1: required string        userName,
     2: optional i64           favoriteNumber,
     3: optional list<string>  interests
   }
   ```
   
   Protobuf:
   
   ```
   message Person {
     required string user_name         = 1;
     optional i64    favorite_number   = 2;
     repeated string interests         = 3;
   }
   ```
   
2. Thrift and protobuf don't encode `required/optional` info, check for "required"
   will be done at runtime.
   
   If a filed is None, nothing will be encoded.

### Field tags and schema evolution

As long as you don't break the field tags, i.e., keep the field tags unchanged,
compatibility should be generally viable.

1. Forward compatibility (new data, old code)
   
   You cannot remove required fields

2. Backward compatibility (new code, old data)

   You cannot add new required fields

### Datatypes and schema evolution

It is possible to change the data type and still maintains compatibility, e.g.,
changing a field from `i32` to `i64`, the parser could fill the missing bytes.
Though this could lose precision.

But generally, changing types break compatibility.

## Avro

1. Avro was started as a subproject of Hardoop, as a result of Thrift not being
   a good fit for Hardoop's use cases.
   
2. Avro has 2 schemas, one intended for human editing, one (in JSON) that is more 
   machine-readable:

   ```
   record Person {
       string                  userName;
       union {null, long }     favoriteNumber = null;
       array<sring>            interests;
   }
   ```
   
   ```
   {
       "type": "record",
       "name": "Person",
       "fields": [
           { "name": "userName",        "type": "string" },
           { "name": "favoriteNumber",  "type": ["null", "long"], "default": null },
           { "name": "interests",       "type": {"type": "array", "items": "string" }}
       ]
   }
   ```
   
   You can see that it does not have field tags, and more interestingly, the encoded
   data does not contain field name, only bytes of values! 
   
   So a reader can only decode the data with a correct schema.
   
### The writer's schema and the reader's schema

1. An application that writes the Avro data is called a wrier, and a reader will
   decode the data written by writer.
   
   The schemas of reader and write do not need to be exactly same, it would work
   as long as they are compatible.

### Schema evolution rules

1. To be compatible, you cannot remove or add fields without default values.

### But what is the writer's schema?
### Dynamically generated schemas

1. One advantage of Avro over Thrift and Protobuf is that it does not use field
   tags, so that schema change can be easier. With Thrift or Protobuf, you have
   to be careful to not use a field tag that has already been used.

### Code generation and dynamically typed languages

## The merits of Schemas

1. The idea we have seen in Thrift and Protobuf are not new, they have a lot in
   common with ASN.1, which was introduced in 1984.

# Modes of Dataflow

In this section, we talked about how data could flow from one process to another
process in cases they don't share memory:

* Via databases
* Via RPC or REST calls
* Via asynchronous message passing

## Dataflow Through Databases
## Dataflow Through Services: REST and RPC
## Messsage-Parsing Dataflow
