#### 4. top-down merge sort

> spatial complexity: O(N)  
> time complexity: O(NlogN)

The core operation of `merge sort` is merging two disjoint ordered array. 

```rust
fn merge<T: Copy + Ord>(a: &mut [T], mid: usize) {
    let aux: Vec<T> = a.to_vec();
    let len: usize = a.len();
    let mut i: usize = 0;
    let mut j: usize = mid;
    let mut p: usize = 0;

    while i < mid && j < len {
        if aux[i] < aux[j] {
            a[p] = aux[i];
            i += 1;
        } else {
            a[p] = aux[j];
            j += 1;
        }
        p += 1;
    }

    if i < mid {
        a[p..len].clone_from_slice(&aux[i..mid]);
    }
    if j < len {
        a[p..len].clone_from_slice(&aux[j..len]);
    }
}
```

Merge sort is recursive, if you wanna sort a big array, you have to sort the
left half side, then sort the right half side, and combine these two parts
together.

```rust
fn top_down_merge_sort<T: Copy + Ord>(a: &mut [T]) {
    // already ordered
    if a.len() <= 1 {
        return;
    }

    let mid: usize = a.len() / 2;

    merge_sort(&mut a[..mid]);
    merge_sort(&mut a[mid..]);
    merge(a, mid);
}
```

Why it works? Let's take a look at a simple example:

We have a array whose value is just `[2, 1]`, to sort such an array, we have
to sort `[2]` and `[1]` first, then merge them. As the lengths of `[2]` and 
`[1]` are both `1`, which means these sub-arrays are already ordered. So we
can directly merge them, then we will get `[1, 2]`, sorting is done.

> In the above example, the `merge` operation looks like a `swap` operation,
> don't get confused, it's merging instead of simply swaping.

![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-08_20-03-12.jpg)

If the array's length is longer than 2, nothing complex, just make the `process
tree` deeper.

So the `merging operation` is the part that *moves* things, all those preceding
recursive calls do is to just halve the length until it reaches 2

![a deeper process tree](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-08_20-14-59.jpg)

The process of `merge sort` is to traverse this tree. We first traverse down 
as we recursively call `merge_sort`, then we merge sub-arrays, move from 
child nodes to parent node.


1. analysis
   
   1. spatial complexity: 

      To simplify the case, we take the value of N to be 16 (a power of 2, so 
      that the process can be easily analysed)

      ![a deeper process tree](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-10_20-33-49.jpg)

      WRONG ANALYSIS: The depth of the `process tree` is `lg(N) + 1`, but 
      only the last `lg(N)` layers are memory-allocated. Each layer will 
      allocate `N` items, so that the space used is `lg(N) * N`

      This is a recursive algorithm, what you did above is calculating the **total
      space consumption** through the **whole** execution.

      ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-24_20-06-45.jpg)
       
      What you calculated is the area of the `pink area`, but the `green line`
      (limit of memory usage) is what we actually want. 

      > this curve is not exact as the memory consumption is not continuous.

      CORRECT ANALYSIS: So what is the maximum memory usage during the execution?
      Apparently it should be `N` as we will consume the most memory in our last
      merge. For each `merge`, the memory allocated will be freed so the real
      case will be something like:

      ![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-24_20-20-41.jpg)

   2. time complexity:

      1. the number of compares:

         First we should consider the number of compares used in the merge 
         operation:

         ```rust
         let mut i: usize = 0;
         let mut j: usize = mid;

         while i < mid && j < len {
             if aux[i] < aux[j] {
                 a[p] = aux[i];
                 i += 1;
             } else {
                 a[p] = aux[j];
                 j += 1;
             }
             p += 1;
         }
         ```

         It's easy to find that `i` or `j` will be added 1 after each compare,
         so `(i+j)-(0+mid) = i + j - mid` is the answer. The max value is the 
         length of the array to be sorted in that `merge`. In this case, when 
         while loop exits, `i` will be `mid` and `j` will be `the length`. 
         The min value is `half the lenght`, in this case, one half is smaller 
         than the other half, so when while loop exits, `i` or `j` will reach 
         its threshold (mid or len), and the other one remains its initial value.

         Now that we know the maximum and minimal number of compares used 
         in `merge`, and `merge sort` is recursive, and in the first call`len=N`.
         we can write a recurisve equation to express the max and min compares:

         > ```rust
         > merge_sort(&mut a[..mid]);
         > merge_sort(&mut a[mid..]);
         > merge(a, mid);
         > ```

         ```
         C(N) <= C(N/2) + C(N/2) + N
         C(N) >= C(N/2) + C(N/2) + N/2
         ```

         `C(N)` is used to denote the number of compares to sort an array
         of length `N`, the first two terms on the right side are the number
         of compares to sort the left and right halves repectively. The last
         term on the right side is the number of compares consumed by the 
         merge operation.

         We can not derive an **exact solution** for all the N values, but 
         if N is a power of 2 (say 2^n) and the equality of our first equation
         holds, then:

         ```
         C(N) = C(N/2) + C(N/2) + N
         // merge the 2 items on the right
         C(N) = 2C(N/2) + N
         // replace N with 2^n
         C(2^n) = 2C(2^n-1) + 2^n
         // devide both sides by 2^n
         C(2^n)/2^n = C(2^n-1)/2^n-1 + 1

         // we can name a function F(n) = C(2^n)/2^n, then
         F(n) = F(n-1) + 1

         // repeatedly substitute the first item on the right side with the left side, until
         F(n) = F(0) + n
         // then substitute F(n) with C(2^n)/2^n
         C(2^n)/2^n = C(2^0)/2^0 + n
         C(2^n)/2^n = C(1) + n
         // Obviously, C(0) = C(1) = 0
         C(2^n)/2^n = 0 + n
         // Multiply both sides by 2^n
         C(2^n) = n * 2^n
         // replace 2^n with N
         C(N) = lgN * N = NlgN
         ```

         > There is a conslusion that the number of compares is between 
         > `NlgN/2` and `NlgN` (page 272)
         > But a precise proof is not provided

      2. the number of array accesses 
         
         At most 6NlgN array accesses to sort an array of length N
     
         Same process as we analyzed the number of compares. First, we calculate
         the max and min number of compares in `merge`:
         1. 2N for copying from `a` to `aux`
         2. 2N for assignment (`a[p] = aux[i];` or `a[p] = aux[j];`)
         3. the number of compare is in range [N/2, N]. Two accesses used for one
            compare, so the range for this is [N, 2N].

         ```
         C(N) <= C(N/2) + C(N/2) + 2N + 2N + 2N
         C(N) >= C(N/2) + C(N/2) + 2N + 2N + N
         ```

         Take the value of `N` as `2^n` and the equality of our first equation
         holds:

         ```
         C(N) = C(N/2) + C(N/2) + 2N + 2N + 2N
         C(N) = 2C(N/2) + 6N
         C(2^n) = 2C(2^n-1) + 6 * 2^n
         C(2^n)/2^n = C(2^n-1)/2^n-1 + 6
         F(n) = F(n-1) + 6
         F(n) = F(0) + 6n
         C(2^n)/2^n = C(2^0)/2^0 + 6n
         C(2^n)/2^n = C(1) + 6n
         C(2^n)/2^n = 0 + 6n
         C(2^n) = 6n * 2^n
         C(N) = 6 * logN * N = 6NlogN
         ```
     
      > The analysis of the number of compares and accesses tells us the time
      > complexity of `merge sort` is proportional to `NlogN`.

2. Improve top-down merge sort

   `Insertion sort` can be very quick when the length of array is small,
   we can test this using our `mst`:

   ```shell
   $ mst 10 insertion
   Use <insertion sort> to sort 10 random numbers, consuming 2.652µs
   
   $ mst 10 merge
   Use <merge sort> to sort 10 random numbers, consuming 17.408µs
   ```

   ```rust
   fn top_down_merge_sort<T: Copy + Ord>(a: &mut [T]) {
       // already ordered
       if a.len() <= 1 {
           return;
       }

       // use insertion sort for small arrays
       if a.len() <= 15 {
           insertion_sort(a);
       }
   
       let mid: usize = a.len() / 2;
   
       merge_sort(&mut a[..mid]);
       merge_sort(&mut a[mid..]);
       merge(a, mid);
   }
   ```

   > Most recursive algorithms can be improved by handling small cases differently

#### 5. Bottom-up merge sort

> spatial complexity: O(N)  
> time complexity: O(NlogN)

```rust
fn bottom_up_merge_sort<T: Copy + Ord>(a: &mut [T]) {
    if a.len() <= 1 {
        return;
    }

    let len: usize = a.len();
    let mut sub_array_size: usize = 1;
    while sub_array_size < len {
        let mut start_index: usize = 0;
        // still have more than one sub-arrays to sort
        while len - start_index > sub_array_size {
            let end_idx: usize = if start_index + 2 * sub_array_size > len {
                len
            } else {
                start_index + 2 * sub_array_size
            };
            // merge a[start_index..start_index+sub_array_size] and a[start_index+sub_array_size..end_idx]
            // NOTE: mid is a relative index number starting from `start_index`
            merge(&mut a[start_index..end_idx], sub_array_size);
            // update `start_index` to merge the next sub-arrays
            start_index = end_idx;
        }
        sub_array_size *= 2;
    }
}
```

Top-down merge sort is a typical `divide and conquer` algorithm. It recursively
call the function to prepare for the final `merge operation`. And we know it is
the `merge operation` that makes the whole algorithm work, so why not directly
`merge` the elements to avoid the overhead of those stacks.

Here is a process to sort `[10, 8, 4, 3, 1, 9, 2, 7, 5, 6]`:

```
sub_array_size: 1
    Array: [8, 10, 4, 3, 1, 9, 2, 7, 5, 6]
    Array: [8, 10, 3, 4, 1, 9, 2, 7, 5, 6]
    Array: [8, 10, 3, 4, 1, 9, 2, 7, 5, 6]
    Array: [8, 10, 3, 4, 1, 9, 2, 7, 5, 6]
    Array: [8, 10, 3, 4, 1, 9, 2, 7, 5, 6]
sub_array_size: 2
    Array: [3, 4, 8, 10, 1, 9, 2, 7, 5, 6]
    Array: [3, 4, 8, 10, 1, 2, 7, 9, 5, 6]
sub_array_size: 4
    Array: [1, 2, 3, 4, 7, 8, 9, 10, 5, 6]
sub_array_size: 8
    Array: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
```

1. analysis
   
   1. time complexity
      
      1. the number of compares

         Use the result of our previous analysis, the number of compares comsumed in mergeing 
         an array with `N` items is in range [N/2, N]

         So in first outer while loop, we try to merge an array of length `2`
         (sub_array_size=1), and repeat this process `N/2` times. So the number
         of compares is `N/2 * [2/2, 2] = [N/2, N]`. Similar process applies to
         second loop, which is `N/4 * [4/2, 4] = [N/2, N]`.

         We will find the number of compares will always have the same value for all
         ourter while loop, which is `[N/2, N]`. Now the question is how many loops
         do we have?

         The condition of outer while loop is `sub_array_size < len`, and the evolution
         of `sub_array_size` is `1, 2, 4, 8, ...`. In the previous example, `N` is `10`,
         and we have `4` loops, we can infer the number of loops is `F(lgN)`, where 
         function `F` is used to find out minimal integer which is just bigger than `lgN`.

         So the total number of compares used in bottom-up merge sort is `F(lgN)*[N/2, N]
         ~[1/2 NlgN, NlgN]`.

      2. the number of accesses

         The number of compares used for merging an array with size `N` is the sum of:

         1. 2N for copying from a to aux
         2. 2N for assignment (a[p] = aux[i]; or a[p] = aux[j];)
         3. the number of compare is in range [N/2, N]. Two accesses used for 
	    one compare, so the range for this is [N, 2N].
	 
	 which is `[5N, 6N]`.

	 And for each outer while loop is also `[5N, 6N]`. So the total number of compares
	 is `[5N, 6N] * F(lgN) ~ [5NlgN, 6NlgN]`.
