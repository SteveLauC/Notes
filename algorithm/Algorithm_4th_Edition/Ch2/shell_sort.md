#### 3. shell sort

> spatial complexity: O(1)
>
> time complexity: depends on the value of `h` and how it changes
>
> stable: No


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
will never happen. 

![diagram](https://github.com/SteveLauC/pic/blob/main/photo_2022-08-05_20-06-09.jpg)

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
            if start < step && j_idx == start && s[start] > key {
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
`selection sort`, the bigger the array is, the more efficient the `shell sort`.
