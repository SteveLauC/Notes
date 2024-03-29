#### 2. insertion sort
   
> spatial complexiry: O(1)
>
> time complexity: O(N^2)
>
> stable: Yes

Use a pointer i to iterate over the array from the first element, we assume 
that all the elements in the left of this pointer are ordered. For the 
current element pointed to by the pointer, we will try to insert it to 
our left sorted sub-array.  Temporarily store s[i] in a variable key.
Use another pointer j to iterate over our sorted sub-array from the location 
of i-1 to 0, if s[j] is bigger than key, move s[j] back by one position using
s[j+1] = s[j]

> Curious why we are not using binary search while inserting `cur` into the
> left sorted array, then I get this: 
> [Insertion Sort with binary search](https://stackoverflow.com/q/18022192/14092446)

```rust
// Old impl in `TheAlgorithms/Rust`

// This impl uses `swap`, which is slower because it doubles the number
// of array access
// see https://github.com/TheAlgorithms/Rust/issues/349
// But this impl is easy to memorize
pub fn insertion_sort<T>(arr: &mut [T])
where
    T: PartialOrd + Copy,
{
    for i in 1..arr.len() {
        let cur = arr[i];
        let mut j = i - 1;

        while arr[j] > cur {
            arr.swap(j + 1, j);
            if j == 0 {
                break;
            }
            j -= 1;
        }
    }
}
```

```rust
pub fn insertion_sort<T>(arr: &mut [T])
where
    T: cmp::PartialOrd + Copy,
{
    for i in 1..arr.len() {
        let cur = arr[i];
        let mut j = i - 1;

        // This loop has two exit points, and needs to be handled differently.
        // Exit point1
        while arr[j] > cur {
            arr[j + 1] = arr[j];
            if j == 0 {
                // Exit point2
                break;
            }
            j -= 1;
        }

        // Exits from "Exit point2"
        // This `&& arr[0] > cur` is needed here as there is a possibility that
        // after executing `j -= 1;`, `j` becomes 0 and then `arr[j] > cur` is
        // not satisfied so that we exit from "Exit point1" with `j` equals 0.
        if j == 0 && arr[0] > cur {
            arr[0] = cur;
        } else {
            // Exits from "exit point1" with condition not satisfied
            arr[j + 1] = cur;
        }
    }
}
```

> If you make `j_idx` of type `usize`, which can not be less than 0, the
> implementation can be pretty ugly. 
> 
> If `j_idx` has type `isize`, the code snippet can be written as:
>
> ```rust
> while j_idx >= 0 && s[j_idx as usize] > key {
>     s[j_idx as usize + 1] = s[j_idx as usize];
>     j_idx -= 1;
> }
> s[j_idx + 1] = key;
> ```
> which is pretty consistent and elegant.
>
> Full code:
>
> > This implementation is easy to memorize, I recommand this
>
> ```rust
> fn insertion_sort<T: PartialOrd + Copy>(arr: &mut [T]) {
>     for i in 1..arr.len() {
>         let cur = arr[i];
>         let mut j = (i - 1) as isize;
> 
>         while j >= 0 && arr[j as usize] > cur {
>             arr[(j + 1) as usize] = arr[j as usize]; // move items back
>             j -= 1;
>         }
>
>        arr[(j + 1) as usize] = cur;
>     }
> }
> ```
>
> But `j_idx` has type `usize`, so `j_idx >= 0` is redundant, then we have to
> prevent `overflow` from happening, so we add this:
>
> ```rust
> if j_idx == 0{
>     break;
> } 
> ```
> Then our `while` loop has 2 exits which should be handled differently
>

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
