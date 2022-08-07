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

   Before we dive into the shell sort, let's write a generic `insertion sort`
   where we can specify the `start` point and the `step`. In our previous
   implementation, `start` is 0 and `step` is 1.

   ```rust
   fn insertion_sort<T: PartialOrd + Copy>(s: &mut [T], start: usize, step: usize) {
       for i_idx in ((start + step)..s.len()).step_by(step) {
           let key: T = s[i_idx];
           let mut j_idx: usize = i_idx - step;

           // overflow will happen
           if start < step {
               while s[j_idx] > key {
                   s[j_idx + step] = s[j_idx];
                   if j_idx == start {
                       break;
                   }
                   j_idx -= step;
               }

               if j_idx == start && s[start] > key {
                   s[start] = key;
               } else {
                   s[j_idx + step] = key;
               }
           } else {
               while j_idx >= start && s[j_idx] > key {
                   s[j_idx + step] = s[j_idx];
                   dbg!(j_idx, step);
                   j_idx -= step;
               }

               s[j_idx + step] = key;
           }
       }
   }
   ```

   Remeber that we make `j_idx` of type `usize`, so that we can not write `j_idx>=0`
   in the while condition, and to stop the overflow from happening, we use `if j_idx == 0`
   to `break` the while loop. Let make this trick generic.

   In our previous implementation, if we don't use that trick, when `j_idx` is 0,
   `j_idx -= 1` will elicit overflow. We alreay know the step of previous impl is `1`,
   so the generic version of `j_idx -= 1` is `j_idx -= step`. Ok, now let's figure
   out in what cases, this statement will trigger overflow. `j_idx` ranges from
   [start, i_idx-step], the smaller `j_idx` is, the more likely to overflow. But
   if `start > step`, even though we take the value of `j_idx` to its smallest value `start`, 
   `j_idx -= step` will still not overflow.

   Yes, we find the condition. When `start` is greater than `step`, the overflow
   will never happen. Let's vertify this using our previous case, `start` is 0
   and `step` is 1, 0 is smaller than 1, overflow will happen.

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-05_20-06-09.jpg)


   The above implementation separates these two cases, we can mix them:

   ```rust
   fn insertion_sort<T: PartialOrd + Copy>(s: &mut [T], start: usize, step: usize) {
       for i_idx in ((start + step)..s.len()).step_by(step) {
           let key: T = s[i_idx];
           let mut j_idx: usize = i_idx - step;

           while j_idx >= start && s[j_idx] > key {
               s[j_idx + step] = s[j_idx];
               if start < step && j_idx == start {
                   break;
               }
               j_idx -= step;
           }
           if start < step && j_idx == start && s[0] > key {
               s[start] = key;
           } else {
               s[j_idx + step] = key;
           }
       }
   }
   ```

   If we sort a array like `[9, 8, 7, 6, 5, 4, 3, 2, 1]` using plain `insertion sort`,
   note the last item `1` needs to be moved to the start of the array consuming 8 steps,
   which is rather slow.

   But if we use our generic `insertion sort`, make `start` have value of 0 and `step`
   have value of 4, then we are actually sorting `[9, 5, 1]`, the number of steps
   needed decreases to 2. And this is the trick we use to enhance our `insertion sort`.

   h-sorted array: if in a array, every `h`th entry(starting anywhere) yields a
   sorted sequence, then this array is named `h-sorted array`

   ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-06_19-54-56.jpg)

   So the basic idea of shell sort is: first we have a relatively big `h`, then
   we h-sort this array, moving items long distances with few steps. Then we 
   decrease `h`, thanks for our previous big `h`, our array is basicially ordered
   so that these small `h`-sort won't take too much time. When h turns out to be
   1 (i.e. plain insertion sort), our array is guaranteed to be totally in order.

   Why shell sort is fast:
   1. when h is big, the sub-array is very short(len: s.len()/h)
   2. when h is small, the sub-array is basically ordered

   Both of these two cases are suitable for insertion sort.

   ```rust
   fn shell_sort<T: PartialOrd + Copy>(s: &mut [T]) {
       fn insertion_sort<T: PartialOrd + Copy>(s: &mut [T], start: usize, step: usize) {
           for i_idx in ((start + step)..s.len()).step_by(step) {
               let key: T = s[i_idx];
               let mut j_idx: usize = i_idx - step;
   
               while j_idx >= start && s[j_idx] > key {
                   s[j_idx + step] = s[j_idx];
                   if start < step && j_idx == start {
                       break;
                   }
                   j_idx -= step;
               }
               if start < step && j_idx == start && s[0] > key {
                   s[start] = key;
               } else {
                   s[j_idx + step] = key;
               }
           }
       }
   
       let mut h: usize = s.len() / 2;
       while h > 0 {
           for i in 0..h {
               insertion_sort(s, i, h);
           }
   
           h /= 2;
       }
   }
   ```

   Performance test:

   ```shell
   $ ./mst 10000 shell -o
   Use <shell sort> to sort 10000 ordered numbers, consuming 8.150583ms
   $ ./mst 10000 shell
   Use <shell sort> to sort 10000 random numbers, consuming 12.409628ms
   $ ./mst 10000 insertion
   Use <insertion sort> to sort 10000 random numbers, consuming 206.004004ms
   $ ./mst 10000 selection
   Use <selection sort> to sort 10000 random numbers, consuming 1.663771744s
   ```

   The performance of `insertion sort` is dependent on the input data, so is
   `shell sort`. We should also note that the performance of `shell sort` is
   related to `h`, it is very hard to find the most suitable `h`.

   And `shell sort` is much more efficient than the `insertion sort` and 
   `selection sort`, the bigger the array is, the larger the array, the more
   efficient the `shell sort`.
