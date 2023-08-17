> https://gavv.net/articles/file-locks/


Different locks are associated with different things, and this stuff seems a 
little bit of counterintuitive.

For example, BSD locks are associated with the `File Object`, then if different
processes `open(2)` the same file, they get different `File Object`s, they should
not block each other as they are holding **different** `File Object`s.

The above thought is wrong, you should think the thing a lock is associated with
is a thread, and the file itself is what the lock tries to protect, so with different
file objects, they are against each other.


![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-17%2017-58-11.png)

For duplicate file descriptors, they share the same file object, and thus are not 
exclusive to each other.

![diagram](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202023-08-17%2018-00-07.png)
