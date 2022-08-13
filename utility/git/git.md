1. How to rebase you PR
    
   ```shell
   # update your fork repo's master/main branch
   $ git checkout master/main
   $ git pull upstream master/main
   $ git push origin master/main

   $ git checkout your-pr-branch
   $ git rebase master/main

   # if have conflicts, resolve them 
   $ vim xxx
   $ git add {conflict-file-name}

   $ git push --force
   ```

2. How to squash last N commits into a single commit

   ```shell
   $ git reset --soft HEAD~N
   $ git commit # will launch nano to enter a new commit message
   $ git push --force
   ```
