#### 1. selection sort: swap the ith smallest item with s[i]
   
   > spatial complexity: O(1)
   >
   > time complexity: O(N^2)
   >
   > stable: [No](https://stackoverflow.com/a/4601081/14092446)

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

         > What are the situations in other sorting algorithms
