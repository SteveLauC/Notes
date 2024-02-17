> * 62.1 overview
> * 62.2 retrieving and modifying terminal attributes
> * 62.3 the `stty` command
> * 62.4 terminal special characters
> * 62.5 terminal flags
> * 62.6 terminal i/o modes
> * 62.6.1 canonical mode
>   * 62.6.2 noncanonical mode
>   * 62.6.3 cooked, cbreak, and raw modes
> * 62.7 terminal line speed(bit rate)
> * 62.8 terminal line control
> * 62.9 terminal window size
> * 62.10 terminal identification
> * 62.11 summary


1. Historically, users connect to a UNIX system (probably a server shard by a 
   lot of people) through a terminal machine, which is a CRT that is capable 
   of display characters.

   ![tty](https://github.com/SteveLauC/pic/blob/main/tty.jpg)

   In even earlier times, terminals were hard-copy teletype machines:

   ![teletype machine](https://github.com/SteveLauC/pic/blob/main/teletype_machine.jpg)

2. At that tiem, ways on how to control a terminal device (here, I guess we are
   referring to the CRT terminals):

   1. Move the cursor back and forth
   2. clear the current line
   3. and so on...

   are not standardized.

   Eventually, some vendor implemnetations of such `eacape sequences`, fo example,
   Digital' VT-100 bcase the defacto standard, the ANSI standard.

   > The first picture in this file is a VT-100.

3. The GUI era

   The canoncial window system in the UNIX world is call X, before X, we had another
   system that is called W.

   The X system debuted in 1984:

   > 19 June 1984
   >
   > From: rws@mit-bold (Robert W. Scheifler)
   >
   > To: window@athena
   > 
   > Subject: window system X
   > Date: 19 Jun 1984 0907-EDT (Tuesday)

   > I've spent the last couple weeks writing a window
   > system for the VS100. I stole a fair amount of code
   > from W, surrounded it with an asynchronous rather
   > than a synchronous interface, and called it X. Overall
   > performance appears to be about twice that of W. The
   > code seems fairly solid at this point, although there are
   > still some deficiencies to be fixed up. 
   >
   > We at LCS have stopped using W, and are now
   > actively building applications on X. Anyone else using
   > W should seriously consider switching. This is not the
   > ultimate window system, but I believe it is a good
   > starting point for experimentation. Right at the moment
   > there is a CLU (and an Argus) interface to X; a C
   > interface is in the works. The three existing
   > applications are a text editor (TED), an Argus I/O
   > interface, and a primitive window manager. There is
   > no documentation yet; anyone crazy enough to
   > volunteer? I may get around to it eventually. 
   >
   > Anyone interested in seeing a demo can drop by
   > NE43-531, although you may want to call 3-1945
   > first. Anyone who wants the code can come by with a
   > tape. Anyone interested in hacking deficiencies, feel
   > free to get in touch. 

   With X, we no longer have real terminals, intead, we have terminal emulators,
   the terminal emulator of X is called `xterm`, you can still run it on Fedora
   39 at 2024:

   ![xterm](https://github.com/SteveLauC/pic/blob/main/Screenshot%20from%202024-02-16%2015-46-23.png)

   > Wayland
   >
   > The successor of the X window system is called Wayland.

   > Job control
   >
   > The reason why job control exists is that with we normally have ONLY one
   > window, and it is inconvenient to execute mutlple commands in one window.

# 62.1 overview
# 62.2 retrieving and modifying terminal attributes
# 62.3 the `stty` command
# 62.4 terminal special characters
# 62.5 terminal flags
# 62.6 terminal i/o modes
## 62.6.1 canonical mode
## 62.6.2 noncanonical mode
## 62.6.3 cooked, cbreak, and raw modes
# 62.7 terminal line speed(bit rate)
# 62.8 terminal line control
# 62.9 terminal window size
# 62.10 terminal identification
# 62.11 summary


