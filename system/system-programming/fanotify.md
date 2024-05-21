1. Notification event of `FAN_ACCESS` won't be triggered if reading reaches EOF.

2. Events from same process (or threads if `FAN_REPORT_TID` is specified) will
   be merged if they are continuous in the queue.
