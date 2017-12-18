Terminal Tetris by foblo
Dec 17, 2017

Instructions:

compile main.c with the following command:

    gcc -o run main.c -lm

then run it with this one:

    ./run

keep in mind that your terminal must be at least 24 wide by 26 tall
for this game to work properly.

Controls:

r - rotates the piece
s - moves the piece downward faster than the usual falling rate
a - moves the piece to the left
d - moves the piece to the right

note that to use an action, one must hit the key they wish to use and
press enter afterwards to "submit" it. this allows for a queue of 
commands to be built up, and then can be run at will with the enter
key.

Scoring:

-10 points are given for a piece landing
-500 points times two to the power of the number of rows eliminated at 
once plus one are earned when rows are completed
  eg: 1 row = 2000 points, 2 rows = 4000, 3 rows = 8000
