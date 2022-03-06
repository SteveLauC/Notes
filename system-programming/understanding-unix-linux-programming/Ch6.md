1. 终端的输入模式，规范模式和非规范模式
   
   > 17.3 Two Styles of Input: Canonical or Not
   POSIX systems support two basic modes of input: canonical and noncanonical.

   In canonical input processing mode, terminal input is processed in lines 
   terminated by newline ('\n'), EOF, or EOL characters. No input can be 
   read until an entire line has been typed by the user, and the read function 
   (see I/O Primitives) returns at most a single line of input, no matter how 
   many bytes are requested.
   * In canonical input mode, the operating system provides input editing facilities:
   some characters are interpreted specially to perform editing operations within 
   the current line of text, such as ERASE and KILL. See Editing Characters.
   The constants _POSIX_MAX_CANON and MAX_CANON parameterize the maximum number 
   of bytes which may appear in a single line of canonical input. See Limits for 
   Files. You are guaranteed a maximum line length of at least MAX_CANON bytes, 
   but the maximum might be larger, and might even dynamically change size.
   * In noncanonical input processing mode, characters are not grouped into lines, 
   and ERASE and KILL processing is not performed. The granularity with which bytes 
   are read in noncanonical input mode is controlled by the MIN and TIME settings. 
   See Noncanonical Input.
   Most programs use canonical input mode, because this gives the user a way to 
   edit input line by line. The usual reason to use noncanonical mode is when the 
   program accepts single-character commands or provides its own editing facilities.
   The choice of canonical or noncanonical input is controlled by the ICANON flag 
   in the `c_lflag` member of struct termios. See Local Modes.

