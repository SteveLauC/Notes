1. selection sort: swap the ith smallest item with s[i]

   ```rust
   fn selection_sort<T: PartialOrd>(s: &mut [T]) {
       let len: usize = s.len();
       for i_idx in 0..len {
           let mut min_idx: usize = i_idx;
           for j_idx in (i_idx + 1)..len {
               if s[j_idx] < s[min_idx] {
                   min_idx = j_idx;
               }
           }
           s.swap(min_idx, i_idx);
       }
   }
   ```

   1. analysis 

      * the number of comparision:
     
        When `i_idx`,`j_idx` ranges from `i_idx + 1` to `len`

        |i_idx|range|the number of elements|
        |-----|-----|----------------------|
        |0    |[1, N)| N-1|
        |1    |[2, N)|N-2|
        |..|..|..|
        |N-1|[N-1, N)| 1|
        
        `1+..+(N-1) = (N-1)(1+N-1)/2 = (N^2 - N)/2 ~ N^2/2`

      * the number of exchange: N

   2. there are two unique features of selection sort:

      1. the time complexity is indenpent from the input for the reason that 
      for any kind of input, it will just scan the array to find the minimal value
      over and over again.
     
         If you give a sorted data to the selection sort, you will find it consumes
         the same time as the unordered one.

         ```shell
         $ mst 10000 selection
         Use <selection sort> to sort 10000 random numbers, consuming 1.675552184s

         $ mst -o 10000 selection
         Use <selection sort> to sort 10000 ordered numbers, consuming 1.663471248s
         ```
   
      2. It only moves data n times(exclusive to selection sort)

2. insertion sort

   Use a pointer i to iterate over the array from the first element, we assume 
   that all the elements in the left of this pointer are ordered. For the 
   current element pointed to by the pointer, we will try to insert it to 
   our left sorted sub-array.  Temporarily store s[i] in a variable key.
   Use another pointer j to iterate over our sorted sub-array from the location 
   of i-1 to 0, if s[j] is bigger than key, move s[j] back by one position using
   s[j+1] = s[j]

   ```rust
   pub fn insertion_sort<T: PartialOrd + Copy>(s: &mut [T]) {
       for i_idx in 1..s.len() {
           let key: T = s[i_idx];
           let mut j_idx: usize = i_idx - 1;
   
           while s[j_idx] > key {
               s[j_idx + 1] = s[j_idx];
               if j_idx == 0 {
                   break;
               }
               j_idx -= 1;
           }
           // exit from `break`
           if j_idx == 0 && s[0] > key {
               s[0] = key;
           } else {
               // exit from while
               s[j_idx + 1] = key;
           }
       }
   }
   ```

   > If you make `j_idx` of type `usize`, which can not be less than 0, the
   > implementation can be pretty ugly. 
   > 
   > If `j_idx` has type `isize`, the code snippet can be written as:
   > ```rust
   > while j_idx >= 0 && s[j_idx as usize] > key {
   >     s[j_idx as usize + 1] = s[j_idx as usize];
   >     j_idx -= 1;
   > }
   > s[j_idx + 1] = key;
   > ```
   > which is pretty consistent and elegant.
   > But `j_idx` has type `usize`, so `j_idx >= 0` is redundant, but we have to
   > prevent `overflow` from happening, so we add this:
   > ```rust
   > if j_idx == 0{
   >     break;
   > } 
   > ```
   > Then our `while` loop has 2 exits which should be handled differently

   Unlike `selection sort`, the consuming time of insertion sort is decided by
   the input data(note the condition of our while loop is `s[j_idx] > key`)

   1. analysis

      * the number of comparsion:
        
        > Time consumping is dependent on the input data, so the number of 
        > comparsion is also determined by the data.

        * best case(data is ordered by default): n-1
          
          > try once and exit the while loop

        * worst case(data is in reverse order): 

          |i_idx|the number of comparsion|
          |-----|------------------------|
          |1    |1|
          |2    |2|
          |.....|........................|
          |n|n|

          (n*n+1)/2 = (n^2)/2
        
        * average case: worst case/2 = (n*n+1)/4 = (n^2)/4

      * the number of exchange: 
        
        * best case: 0

        * worse case: 
          
          |i_idx|the number of exchange|
          |-----|------------------------|
          |1    |1|
          |2    |2|
          |.....|........................|
          |n|n|

          (n*n+1)/2 = (n^2)/2

        * average case: worst case/2 = (n*n-1)/4 = (n^2)/4

   2. feature: If the input data is partially or totally sorted, then insertion 
   sort is super fast
      
      ```shell
      $ mst 10000 insertion
      Use <insertion sort> to sort 10000 random numbers, consuming 209.427357ms
      $ mst -o 10000 insertion # super fast!
      Use <insertion sort> to sort 10000 ordered numbers, consuming 403.46µs
      ```

3. shell sort
