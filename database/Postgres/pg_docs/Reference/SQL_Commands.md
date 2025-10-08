# `CREATE TYPE`

1. There are 5 ways to create a new type in Postgres, which will create different
   kinds of types:

   * Composite type
   * Enum type
   * Range type
   * Base type
   * Shell type: just a place holder.
   
2. To create a base type, you must be a superuser. Such an restriction is added
   because invalid base type can crash the server.
   
3. How to create a base type

   ```sql
   CREATE TYPE name (
       INPUT = input_function,
       OUTPUT = output_function
       [ , RECEIVE = receive_function ]
       [ , SEND = send_function ]
       [ , TYPMOD_IN = type_modifier_input_function ]
       [ , TYPMOD_OUT = type_modifier_output_function ]
       [ , ANALYZE = analyze_function ]
       [ , SUBSCRIPT = subscript_function ]
       [ , INTERNALLENGTH = { internallength | VARIABLE } ]
       [ , PASSEDBYVALUE ]
       [ , ALIGNMENT = alignment ]
       [ , STORAGE = storage ]
       [ , LIKE = like_type ]
       [ , CATEGORY = category ]
       [ , PREFERRED = preferred ]
       [ , DEFAULT = default ]
       [ , ELEMENT = element ]
       [ , DELIMITER = delimiter ]
       [ , COLLATABLE = collatable ]
   )
   ```
   
   > The order of parameters does not matter.
   
   1. Required functions
   
      1. `INPUT`: deserializes the type's textual representation to the internal 
         representation. 
         
         This function can be in form:
         
         1. `fn input_function(cstring) -> TYPE`
         2. `fn input_function(cstring, oid, integer) -> TYPE`
         
         * `cstring`: argument is always the textual repr of the type
         * `oid`: the type's OID. For the built-in array type, this argument is the
           OID of its elements' type
         * `integer`:  typmod of the destination column, if known (-1 will be 
           passed if not).        
           
         `INPUT` functions are normally declared with `STRICT`. 
         
         > QUES: why
           
      2. `OUTPUT`: serialization to text.
      
         ```rs
         fn output_function(TYPE) -> cstring
         ```

   2. Optional functions 
   
      1. `RECEIVE`: deserialize from binary format
      
         If this function is not supplied, the type cannot participate in binary 
         input. The binary repr should be designed in a way that makes the 
         conversion cheap, and the repr should also be portable(e.g., the built-in
         integer types use network order in order to be portable).
         
         ```rs
         type internal = * const StirngInfo;
         
         fn receive_function(internal) -> TYPE
         fn receive_function(internal, oid, integer) -> TYPE
         ```
         
         `RECEIVE` functions are normally declared with `STRICT`. 
         
         > QUES: why
       
      2. `SEND`: serialize to binary format
      
         ```rs
         fn send_function(TYPE) -> bytea
         ```
         
         
         > All the above functions require the `TYPE` to be defined, but these
         > functions are need in `TYPE` definition. How can we solve this?
         > The answer is to use shell type;
         >
         > ```sql
         > CREATE TYPE MY_TYPE; -- shell type, placeholder
         >
         > create function input_function(cstring) -> MY_TYPE;
         > create function output_function(MY_TYPE) -> cstring;
         >
         > create type MY_TYPE (
         >     INPUT = input_function,
         >     OUTPUT = output_function,
         > );
         > ```
      
      3. `TYPEMOD_IN`: related to type modifier, which I have no idea what that means.
      4. `TYPEMOD_OUT`
      5. `ANALYZE`
      
         The optional analyze_function performs type-specific statistics 
         collection for columns of the data type. By default, `ANALYZE` will 
         attempt to gather statistics using the type's “equals” and “less-than” 
         operators, if there is a default b-tree operator class for the type. 
         For non-scalar types this behavior is likely to be unsuitable, so it 
         can be overridden by specifying a custom analysis function. The analysis 
         function must be declared to take a single argument of type internal, 
         and return a boolean result. The detailed API for analysis functions 
         appears in `src/include/commands/vacuum.h`.
      
      6. `SUBSCRIPT`: Enable this type to be used as a subscript type if provided.
         subscript type is type which can be used to index something.

   3. Optional parameters
   
      1. `INTERNALLENGTH`: Specify the length info of this type, a positive number
         should be supplied if it has fixed length. Or `VARIABLE` if it is var-len
         type.
         
      2. `PASSEDBYVALUE`: a flag, i.e., no value needs to be provided.
      
         If present, the values of this type will be passed by value rather than
         reference. Types passed by value must be fixed-length, and the size can
         not be larger than `sizeof(Datum)`.(4 bytes on soem machines, 8 bytes on
         others)
         
      3. `ALIGNMENT`: can be 1, 2, 4, 8. Var-len types should have alignment larger
         than 4 because they likely will contain a `int32` field for recroding
         the length.   
   
      4. `STORAGE`: storage attribute, contains if values of this type will be 
         moved to TOAST table under certain cases.
         
      5. `LIKE`: specifies the like type, if specified, for the following 
         parameters:
         
         * `INTERNALLENGTH`
         * `PASSEDBYVALUE`
         * `ALIGNMENT`
         * `STORAGE`
         
         If they are not provided, then the corresponding config value of the like
         type will be used. (This parameter allows you to provide a template for
         the above 4 parameters)
         
      6. `CATEGORY` and `PREFERRED`
      
         Define if implict casting should be done in ambiguous situations.
         
      7. `DEFAULT`: default value of this type
      
      8. `ELEMENT`: If this type is similar to an array type, this parameter
         specifies the type of its elements.
         
      9. `DELIMITER`: If this type is similar to an array type, this parameter
         specifies the delimiter used in the textual repr.
      
      10. `COLLATABLE`: If true, then values of this type can be collated via the
          `COLLATE` clause.    
         