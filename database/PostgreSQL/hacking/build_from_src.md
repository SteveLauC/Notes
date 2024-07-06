1. Requirements on Fedora

   ```sh
   $ sudo dnf install make meson gcc tar flex bison perl readline-devel -y
   ```

   > I encountered some build errors, these packages are also needed.
   > 
   > ```sh
   > $ sudo dnf install perl-File-Compare
   > $ sudo dnf install perl-FindBin
   > ```

2. Pull the source code
   
   ```sh
   $ git clone https://github.com/postgres/postgres.git
   ```

3. Check if everything is ready with

   ```sh
   $ cd postgres
   $ ./configure
   ```

   The log of this `configure` program will be written to `./config.log`.

4. Build and install the binary

   1. Build the binaries with `$ make`, binaries will be put under `src/bin`:
     
      ```sh
      $ pwd
      /home/steve/Documents/workspace/postgres/src/bin
      $ l
      Permissions Links Size User  Group Date Modified Name
      drwxr-xr-x@     1    - steve steve 23 Jun 13:43  initdb
      .rw-r--r--@     1  875 steve steve 20 Jun 10:34  Makefile
      .rw-r--r--@     1  536 steve steve 20 Jun 10:34  meson.build
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_amcheck
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_archivecleanup
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_basebackup
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_checksums
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_combinebackup
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_config
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_controldata
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_ctl
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_dump
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_resetwal
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_rewind
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_test_fsync
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_test_timing
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_upgrade
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_verifybackup
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_waldump
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pg_walsummary
      drwxr-xr-x@     1    - steve steve 20 Jun 15:54  pgbench
      drwxr-xr-x@     1    - steve steve 20 Jun 10:34  pgevent
      drwxr-xr-x@     1    - steve steve 20 Jun 15:55  psql
      drwxr-xr-x@     1    - steve steve 20 Jun 15:55  scripts

      $ l initdb
      Permissions Links Size User  Group Date Modified Name
      .rw-r--r--@     1  46k steve steve 20 Jun 10:34  findtimezone.c
      .rw-r--r--@     1 8.4k steve steve 20 Jun 15:54  findtimezone.o
      .rwxr-xr-x@     1 175k steve steve 23 Jun 13:43  initdb
      .rw-r--r--@     1  96k steve steve 20 Jun 10:34  initdb.c
      .rw-r--r--@     1  97k steve steve 23 Jun 13:43  initdb.o
      lrwxrwxrwx@     1   33 steve steve 20 Jun 15:54  localtime.c -> ../../../src/timezone/localtime.c
      .rw-r--r--@     1  17k steve steve 20 Jun 15:54  localtime.o
      .rw-r--r--@     1 2.0k steve steve 20 Jun 10:34  Makefile
      .rw-r--r--@     1 1.1k steve steve 20 Jun 10:34  meson.build
      .rw-r--r--@     1  801 steve steve 20 Jun 10:34  nls.mk
      drwxr-xr-x@     1    - steve steve 20 Jun 10:34  po
      drwxr-xr-x@     1    - steve steve 20 Jun 10:34  t
      ```

   2. Put the built binaries to `/usr/local/pgsql` (the default installation location)

      ```sh
      $ sudo make install # sudo is required for permission reasons

      $ pwd
      /usr/local/pgsql

      $ l
      Permissions Links Size User Group Date Modified Name
      drwxr-xr-x@     1    - root root  20 Jun 16:09  bin
      drwxr-xr-x@     1    - root root  20 Jun 16:09  include
      drwxr-xr-x@     1    - root root  20 Jun 16:09  lib
      drwxr-xr-x@     1    - root root  20 Jun 16:09  share

      $ l bin
      Permissions Links Size User Group Date Modified Name
      .rwxr-xr-x@     1  88k root root  20 Jun 16:09  clusterdb
      .rwxr-xr-x@     1  88k root root  20 Jun 16:09  createdb
      .rwxr-xr-x@     1  88k root root  20 Jun 16:09  createuser
      .rwxr-xr-x@     1  83k root root  20 Jun 16:09  dropdb
      .rwxr-xr-x@     1  83k root root  20 Jun 16:09  dropuser
      .rwxr-xr-x@     1 1.1M root root  20 Jun 16:09  ecpg
      .rwxr-xr-x@     1 175k root root  20 Jun 16:09  initdb
      .rwxr-xr-x@     1 115k root root  20 Jun 16:09  pg_amcheck
      .rwxr-xr-x@     1  54k root root  20 Jun 16:09  pg_archivecleanup
      .rwxr-xr-x@     1 173k root root  20 Jun 16:09  pg_basebackup
      .rwxr-xr-x@     1  89k root root  20 Jun 16:09  pg_checksums
      .rwxr-xr-x@     1 177k root root  20 Jun 16:09  pg_combinebackup
      .rwxr-xr-x@     1  52k root root  20 Jun 16:09  pg_config
      .rwxr-xr-x@     1  71k root root  20 Jun 16:09  pg_controldata
      .rwxr-xr-x@     1 114k root root  20 Jun 16:09  pg_createsubscriber
      .rwxr-xr-x@     1  86k root root  20 Jun 16:09  pg_ctl
      .rwxr-xr-x@     1 448k root root  20 Jun 16:09  pg_dump
      .rwxr-xr-x@     1 136k root root  20 Jun 16:09  pg_dumpall
      .rwxr-xr-x@     1  82k root root  20 Jun 16:09  pg_isready
      .rwxr-xr-x@     1 120k root root  20 Jun 16:09  pg_receivewal
      .rwxr-xr-x@     1 120k root root  20 Jun 16:09  pg_recvlogical
      .rwxr-xr-x@     1  81k root root  20 Jun 16:09  pg_resetwal
      .rwxr-xr-x@     1 220k root root  20 Jun 16:09  pg_restore
      .rwxr-xr-x@     1 173k root root  20 Jun 16:09  pg_rewind
      .rwxr-xr-x@     1  59k root root  20 Jun 16:09  pg_test_fsync
      .rwxr-xr-x@     1  48k root root  20 Jun 16:09  pg_test_timing
      .rwxr-xr-x@     1 193k root root  20 Jun 16:09  pg_upgrade
      .rwxr-xr-x@     1 138k root root  20 Jun 16:09  pg_verifybackup
      .rwxr-xr-x@     1 127k root root  20 Jun 16:09  pg_waldump
      .rwxr-xr-x@     1  80k root root  20 Jun 16:09  pg_walsummary
      .rwxr-xr-x@     1 211k root root  20 Jun 16:09  pgbench
      .rwxr-xr-x@     1  10M root root  20 Jun 16:09  postgres
      .rwxr-xr-x@     1 732k root root  20 Jun 16:09  psql
      .rwxr-xr-x@     1  93k root root  20 Jun 16:09  reindexdb
      .rwxr-xr-x@     1 102k root root  20 Jun 16:09  vacuumdb
      ```

5. Create directory for PostgreSQL to store its data

   ```
   $ cd ~
   $ cd Document
   $ mkdir pg_data
   ```

6. ~~PostgreSQL will create a lock file and a UNIX socket file under 
   `/var/run/postgresql`, change the owner and group of this directory 
   to `$USER` so that it is writable~~.

   Well, this only happen if you install Postgres through RPM, Postgres
   installed from src creates these 2 files under `/tmp`.
   

   ```sh
   $ sudo mkdir /var/run/postgresql
   $ sudo chown steve:steve /var/run/postgresql
   ```

   ```sh
   $ cd /var/run/postgresql
   $ ll
   Permissions Links Size User  Group Date Modified Name
   drwxr-xr-x@     2    - steve steve 20 Jun 16:19  .
   drwxr-xr-x@    55    - root  root  20 Jun 15:39  ..
   srwxrwxrwx@     1    0 steve steve 20 Jun 16:19  .s.PGSQL.5432
   .rw-------@     1   89 steve steve 20 Jun 16:19  .s.PGSQL.5432.lock
   ```

   > Update: this directory can be configured via `$ postgres -k`, though I have
   > no idea how to let `pg_ctl` accept this argument.

7. Make sure the port that the server will listen on is available, it is 5432 by
   default.

8. Initialize the data directory

   ```sh
   $ initdb -D ~/Documents/pg_data
   ```

9. Start the server

   ```sh
   pg_ctl -D ~/Documents/pg_data -l ~/Documents/pg_data/logfile start
   ```

   > `logfile` is only needed to store the log during start, after that, it won't
   > be used.

9. Create a database named with `$USER`

   ```sh
   $ createdb $(echo $USER)
   ```

10. Connect with client

   ```sh
   $ pgcli # or psql
   ```
