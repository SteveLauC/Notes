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

4. Build and install the binary to `/usr/local/pgcli`.

   > This is the default installation path

   ```sh
   $ pwd
   /usr/local/pgsql

   $ l
   Permissions Links Size User Group Date Modified Name
   drwxr-xr-x@     1    - root root  20 Jun 16:09  bin
   drwxr-xr-x@     1    - root root  20 Jun 16:09  include
   drwxr-xr-x@     1    - root root  20 Jun 16:09  lib
   drwxr-xr-x@     1    - root root  20 Jun 16:09  share
   ```

5. Create directory for PostgreSQL to store its data

   ```
   $ cd ~
   $ cd Document
   $ mkdir pg_data
   ```

6. PostgreSQL will create a lock file under `/var/run/postgresql`, change the 
   owner and group of this directory to `$USER` so that it is writable.

   ```sh
   $ chown steve:steve /var/run/postgresql
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

7. Make sure the port that the server will listen on is available, it is 5432 by
   default.

8. Initialize the data directory

   ```sh
   $ createdb -D ~/Documents/pg_data
   ```

9. Create a database named with `$USER`

   ```sh
   $ createdb $(echo $USER)
   ```

10. Connect with client

   ```sh
   $ pgcli
   ```
