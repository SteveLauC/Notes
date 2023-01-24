1. Use `w`, `e` and `b` to move the cursors forward or backword, this is same with
   vim.


   Except for these three motions, `helix` also supports:

   * `W`
   * `E`
   * `B`

   You can use them to traverse WORDS instead of words, WORDS are WORDS are only 
   **separated by whitespace**, whereas words can be separated by other characters
   in addition to whitespace.

2. Type ; to collapse selections to single cursors.

   If you are getting used to the selection in normal mode, use `;` to get rid
   of it.

3. `u` to undo, `U` to redo.

4. use `C` to duplicate your curosr, `,` to remove the duplicate cursors.

   Type C to duplicate the cursor **to the next suitable line**. Notice how it skips 
   the line in the middle. Keys you type will now affect both cursors.

   ```
   --> Fix th two nes at same ime.
   -->
   --> Fix th two nes at same ime.
       Fix these two lines at the same time
   ```

5. Type s to select matches **in the selection** instead of the whole buffer.

   If there are multiple results in the selection, helix will duplicate the
   cursor for you.

6. `f` in helix is not just about finding some character, it is selecting until
   that character.

   Same applies to `t`.

   > Note: Unlike Vim, Helix doesn't limit these commands to the current line.
   > It searches for the character in the file.

7. use `r <char>` to replace all the characters in selection with `<char>`.

   world

   try select `world` then press `ra`, `world` then will be changed into `aaaaa`.

8. use `R` to replace the selected text with the contents stored in the clipboard.

9. use `J` to join lines, to join multiple lines, select them first, then press
   `J`.

10. use `CTRL a` to increase a number, `CTRL x` to decrease a number.

11. helix has multiple registers, to pick which register to use, type `"`.

    It seems that you have registers `"` and `:` for copy and paste.
    '@' for recording macros, and `/` for storing the last search.

12. macros

    use `Q` to start and stop recording macro. By default, macro will be stored
    in register `@`.

    use `q` to use it, you can prefix with `<number>` to run it multiple times.

13. Use `*` to copy what's in register `"` to register `/`, which allows us
    to search without pressing `/`.

14. jumplist

    > killer feature
    > Ohh, vim has this, it is just that I don't know it.

    Use `Ctrl s` to save the current selction into your jumplist, use `Ctrl-i`
    and `Ctrl-o` to move forward and backward in your jumplist.
