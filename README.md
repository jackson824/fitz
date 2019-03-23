# fitz
A tile based multiplayer console game, programmed in C99. Comes with two automated player so it's alright if you don't have friends!

## Compile Instruction
Compile `fitz.c` with gcc (remember to use flag `-std=c99`). Alternatively, `make` with the provided makefile. 

## Usage Instruction
See the section below for an explanation of how the game works. 
`Usage: fitz tilefile [p1type p2type [height width | filename]]`

### Parameters
- `tilefile` is the filename of a file containing the tiles to be used in this game.
- `p1type` and `p2type` to the type of players: use `h` for human player, `1` for automated player 1 and `2` for automated player 2. 
- `height` and `width` refer to the height and width of the board.

## The Game
The game begins with an empty board, displayed like this (this is an example of a 4x5 board):
```
.....
.....
.....
.....
```

The two players in the game will take turns getting a tile from a file and placing it onto the board. Tiles can be rotated clockwise (in multiples of 90 degrees) before being placed. For example, a tile shaped like this:
```
****
```
could be placed in any of the following ways (as well as others):
```
*....   .....   ....*   .....
*....   .....   ....*   .....
*....   ****.   ....*   .....
*....   .....   ....*   .****
```
The first player will indicate their tiles with * and the second player will use #. A tile can not be placed where it will overlap with existing tiles on the board. The game ends when a player can not place their next tile. That player loses. For example, the * player needing to place
```
**
**
```
on a board that looks like:
```
#####
.**##
**..#
.****
```
would lose the game. The tiles must be used in the order they appear in the file. If you run out of tiles, the program starts picking tiles again from the beginning of the file. 

### Tile file
Each tile is described as a 5x5 grid (unlike the game board, which can be [almost] any size) in the tile file, with `!` representing an occupied position and `.` representing an empty position. For example, a tile might look like:
```
!!!..
!!...
.....
.....
.....
```
Each tile is followed by an empty line. The tile file can contain as many tiles as you wish. A sample tile file `tiles` is provided. 

Note: Rotations of tiles are computed by the program when needed so there is no need to hard code four versions of the same tile in the tile file. 

### Interaction
Each time the game reaches a human player's turn, the tile the user is to place is displayed in console, followed by a prompt like `Player *] `. The game will then wait for the user to enter `row column rotate` (single space separated with no leading nor trailing spaces). The `row` and `column` dictates the position the tile will be placed on the game board (leftmost column and top row are 0 and increase as you go right/down) and `rotate` is the angle of (clockwise) rotation and can take a value of either `0`, `90`, `180` or `270`. Note that, `row` and `column` describe the middle of the tile. For illustration purposes only, the middle point is indicated below with an @:
```
!!!..
!!...
..@..
.....
.....
```
So to place the tile above like this:
```
........
..@.....
...**...
..***...
```
would require an input of `1 2 180` (`@` for illustration purposes only). Note also that `row` and `column` can be negative, as long as all **non-empty** positions of the placed tile are at positions with nonnegative coordinates. 
