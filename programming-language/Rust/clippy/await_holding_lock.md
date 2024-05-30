Why is this bad

> OS Lock guards are not `Send` (because lock a guard in one thread and unlock
> it in another thread is undefined behavior), a future that has a `!Send` value
> corss an `.await` point is `!Send`, so such an future can ONLY be executed on
> the current thread (or a single threaded runtime).

Deadlock could happen, say we have 2 async tasks running on 1 OS thread (it has
to, see above), then 1 task acquires the lock guard, holding it across an 
`.await` point, runtime switches to another task, this task also wants to 
acquire the lock, which is not possible since the lock guard is held by another
task, then the thread gets suspened by the OS, the whole runtime just dies...
