1. Environment variable
2. File descriptor table
3. Resource limit
4. Functions registered by `atexit(2)` or `on_exit(3)`
5. Real UID and GID
6. EUID and EGID (Not true on Linux)
7. Supplementary group IDs
8. set-UID and set-GID flags
9. Current working directory
10. Root directory
11. file mode creation mask (umask)
12. Signal mask
13. Memory mappings
14. Attached shared memory segments
15. Session ID
16. Process Group ID
17. Controlling terminal
18. Signal dispositions, user-installed hanlders will be reset to default 
    ones after calling `exec()`, ignored ones won't be changed.
