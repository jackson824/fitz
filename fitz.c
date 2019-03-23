#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "head.h"

/* The main function */
int main(int argc, char** argv) {
    Game currentGame;
    FILE* tileFile;
    check_arg_count(argc);
    tileFile = open_tile_file(argv[1]);
    char** tiles = process_get_tiles(tileFile);
    int tilesCount = get_tiles_count(tileFile);
    process_shape_display(argc, tiles, tilesCount, tileFile);
    char* playerType1 = argv[2];
    char* playerType2 = argv[3];
    check_player_types(playerType1, playerType2, tiles, tilesCount);
    process_new_game(argc, argv, &currentGame, tiles, tilesCount, tileFile,
            playerType1, playerType2);
    process_saved_game(argc, argv, &currentGame, tiles, tilesCount, tileFile,
            playerType1, playerType2);

    // element 0 contains most recent move by player 0 stored in an int array
    // in the format {r, c}; element 1 for player 1; element 2 for either
    int** recentPlays = malloc(sizeof(int*) * 3);
    for (int i = 0; i < 3; i++) {
        recentPlays[i] = malloc(sizeof(int) * 2);
        recentPlays[i][0] = -10; //initialise to -10
        recentPlays[i][1] = -10; //initialise to -10
    }

    play_game(&currentGame, tiles, tilesCount, playerType1, playerType2,
            recentPlays, tileFile);

    print_winner(currentGame.nextPlayer);

    free_mem_recent_plays(recentPlays);
    free_tiles_mem(tiles, tilesCount);
    fclose(tileFile);
    free(currentGame.grid);
    return 0;
}

/* Checks whether the number of arguments is valid. If valid, return 0; else
return 1. */
void check_arg_count(int argc) {
    if (!(argc == 2 || argc == 5 || argc == 6)) {
        fprintf(stderr, "Usage: fitz tilefile [p1type p2type [height width");
        fprintf(stderr, " | filename]]\n");
        exit(1);
    }
}

/* Attempts to open a file stream to the tile file given its path as argument.
Exits the program if unsuccessful; else return the file stream. */
FILE* open_tile_file(char* filename) {
    FILE* tileFile = fopen(filename, "r");
    if (tileFile == NULL) {
        fprintf(stderr, "Can't access tile file\n");
        exit(2);
    }
    return tileFile;
}

/* Attempts to get the tiles from the tile file. If tile file contents invalid,
exit the program. Otherwise return a char** object containing the tiles. */
char** process_get_tiles(FILE* tileFile) {
    char** tiles = get_tiles(tileFile);
    if (tiles == NULL) {
        fprintf(stderr, "Invalid tile file contents\n");
        fclose(tileFile);
        exit(3);
    }
    return tiles;
}

/* Checks whether the number of arguments is valid. If valid, return 0; else
return 1. */
void check_player_types(char* playerType1, char* playerType2, char** tiles,
        int tilesCount) {
    if (!valid_player_types(playerType1, playerType2)) {
        free_tiles_mem(tiles, tilesCount);
        fprintf(stderr, "Invalid player type\n");
        exit(4);
    }
}

/* Checks whether argc == 2. If true, then display the tiles with their
rotations and exit the program. */
void process_shape_display(int argc, char** tiles, int tilesCount,
        FILE* tileFile) {
    if (argc == 2) {
        shape_display(tiles, tilesCount);
        free_tiles_mem(tiles, tilesCount);
        fclose(tileFile);
        exit(0);
    }
}

/* Attempts to initialise a new game given the dimensions of the board from
user input. Exits the game if invalid dimensions. */
void process_new_game(int argc, char** argv, Game* currentGame, char** tiles,
        int tilesCount, FILE* tileFile, char* playerType1, char* playerType2) {
    if (argc == 6) {
        currentGame->nextTile = 0;
        currentGame->nextPlayer = 0;
        currentGame->rows = strtol(argv[4], NULL, 10);
        currentGame->cols = strtol(argv[5], NULL, 10);
        if (currentGame->rows > 999 || currentGame->rows < 1 ||
                currentGame->cols > 999 || currentGame->cols < 1) {
            free_tiles_mem(tiles, tilesCount);
            fprintf(stderr, "Invalid dimensions\n");
            fclose(tileFile);
            exit(5);
        }
        currentGame->grid = new_grid(currentGame->rows, currentGame->cols);

        print_grid(*currentGame);
        display_next_tile(playerType1, playerType2, currentGame->nextPlayer,
                currentGame->nextTile, tiles);
    }
}

/* Attempts to read data from a saved game file. Exits the game if errors
encountered (invalid save file contents or can't access save file). */
void process_saved_game(int argc, char** argv, Game* currentGame, char** tiles,
        int tilesCount, FILE* tileFile, char* playerType1, char* playerType2) {
    if (argc == 5) {
        FILE* savedGame = fopen(argv[4], "r");
        if (savedGame == NULL) {
            free_tiles_mem(tiles, tilesCount);
            fprintf(stderr, "Can't access save file\n");
            fclose(tileFile);
            exit(6);
        } else {
            currentGame->grid = read_file(savedGame, tilesCount);
            if (currentGame->grid == NULL) {
                free_tiles_mem(tiles, tilesCount);
                fprintf(stderr, "Invalid save file contents\n");
                fclose(savedGame);
                fclose(tileFile);
                exit(7);
            } else {
                int* parameters = read_parameters(savedGame);
                currentGame->nextTile = parameters[0];
                currentGame->nextPlayer = parameters[1];
                currentGame->rows = parameters[2];
                currentGame->cols = parameters[3];
                free(parameters);
                print_grid(*currentGame);
                display_next_tile(playerType1, playerType2,
                        currentGame->nextPlayer, currentGame->nextTile, tiles);
            }
        }
        fclose(savedGame);
    }
}

/* This function handles the game play. */
void play_game(Game* currentGame, char** tiles, int tilesCount,
        char* playerType1, char* playerType2, int** recentPlays,
        FILE* tileFile) {
    while (!game_over(*currentGame, tiles)) {
        if(move(currentGame, playerType1, playerType2, tiles, recentPlays)
                == 2) {
            fprintf(stderr, "End of input\n");
            fclose(tileFile);
            free(currentGame->grid);
            free_mem_recent_plays(recentPlays);
            free_tiles_mem(tiles, tilesCount);
            exit(10);
        }
        currentGame->nextPlayer = (currentGame->nextPlayer + 1) % 2;
        currentGame->nextTile = (currentGame->nextTile + 1) % tilesCount;
        print_grid(*currentGame);
        display_next_tile(playerType1, playerType2, currentGame->nextPlayer,
                currentGame->nextTile, tiles);
    }
}

/* Prints to stdout the winner of the game. */
void print_winner(int nextPlayer) {
    if (nextPlayer == 0) {
        printf("Player # wins\n");
    } else {
        printf("Player * wins\n");
    }
}

/* Display to stdout the next tile. */
void display_next_tile(char* pType1, char* pType2, int nextPlayer,
        int nextTile, char** tiles) {
    if (nextPlayer == 0) {
        if (strcmp(pType1, "h") == 0) {
            fwrite(tiles[nextTile], sizeof(char), 30, stdout);
        }
    } else {
        if (strcmp(pType2, "h") == 0) {
            fwrite(tiles[nextTile], sizeof(char), 30, stdout);
        }
    }
}

/* A function that frees the "recent_plays" 2D array in the main function.  */
void free_mem_recent_plays(int** recentPlays) {
    for (int i = 0; i < 3; i++) {
        free(recentPlays[i]);
    }
    free(recentPlays);
}

/* Checks whether the current game is over; returns 1 if yes, otherwise 0. */
Bool game_over(Game currentGame, char** tiles) {
    int nextTile = currentGame.nextTile;
    int nextPlayer = currentGame.nextPlayer;
    int boardLength = currentGame.cols;
    int boardHeight = currentGame.rows;

    // check every possible tile placement
    for (int i = -2; i < boardHeight + 2; i++) {
        for (int j = -2; j < boardLength + 2; j++) {
            for (int angle = 0; angle < 360; angle += 90) {
                if (valid_tile_placement(currentGame.grid, boardLength,
                        boardHeight, tiles[nextTile], nextPlayer, i, j,
                        angle)) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

/* Prints the grid char array of the current game to stdout. */
void print_grid(Game currentGame) {
    int length = currentGame.rows * (currentGame.cols + 1);
    fwrite(currentGame.grid, sizeof(char), length, stdout);
}

/* Reads grid from a saved game file and returns the grid as a char array;
     returns NULL if invalid contents. */
char* read_file(FILE* input, int tilesCount) {
    rewind(input);
    char* grid = malloc(sizeof(char) * 16);
    int position = 0;
    int next = 0;
    int currentSize = 16; //buffer length
    int gridLength = 0;

    // don't start reading until after the first line
    Bool startReading = 0;
    while (startReading == 0) {
        next = fgetc(input);
        if (next == '\n') {
            startReading = 1;
        }
    }

    Bool endReading = 0;
    while (!endReading) {
        next = fgetc(input);
        if (next == EOF) {
            endReading = 1;
        } else {
            if (position == currentSize) {
                currentSize *= 2;
                grid = realloc(grid, currentSize);
            }
            grid[position++] = (char)next;
            gridLength++;
        }
    }

    int* parameters = read_parameters(input);
    if(check_saved_game(parameters, grid, gridLength, tilesCount) == 0) {
        free(parameters);
        return grid;
    } else {
        free(parameters);
        free(grid);
        return NULL;
    }
}

/* Creates new board as a char array given the number of rows and columns and
   returns it. */
char* new_grid(int rows, int cols) {
    int length = rows * (cols + 1);
    char* grid = malloc(sizeof(char) * length);

    int currentCol = 1;

    for (int i = 0; i < length; i++) {
        if (currentCol <= cols) {
            grid[i] = '.';
            currentCol++;
        } else {
            grid[i] = '\n';
            currentCol = 1;
        }
    }
    return grid;
}

/* Reads the save file data (excluding the board) and stores the read data in
an int array, given a file stream to the save file. */
int* read_parameters(FILE* input) {
    rewind(input);
    // stores the read data in a string array
    char** parametersStorage = malloc(sizeof(char*) * 4);
    // stores the read data in an int array
    int* parameters = malloc(sizeof(int) * 4);
    for (int i = 0; i < 4; i++) {
        parametersStorage[i] = malloc(sizeof(char) * 10);
    }
    int i = 0, j = 0, next = 0;
    while (i < 4) {
        next = fgetc(input);
        if (next == ' ' || next == '\n') {
            parametersStorage[i][j] = '\0';
            j = 0;
            i++;
        } else {
            parametersStorage[i][j] = next;
            j++;
        }
    }
    parameters[0] = strtol(parametersStorage[0], NULL, 10);
    parameters[1] = strtol(parametersStorage[1], NULL, 10);
    parameters[2] = strtol(parametersStorage[2], NULL, 10);
    parameters[3] = strtol(parametersStorage[3], NULL, 10);
    for (int i = 0; i < 4; i++) {
        free(parametersStorage[i]);
    }
    free(parametersStorage);
    return parameters;
}

/* Checks the saved game file for valid contents. Returns 0 if valid, otherwise
     return 1. */
int check_saved_game(int* parameters, char* grid, int gridLength,
        int tilesCount) {
    int nextTile = parameters[0];
    int nextPlayer = parameters[1];
    int rows = parameters[2];
    int cols = parameters[3];
    // check next player parameter is valid
    if (!(nextPlayer == 0 || nextPlayer == 1)) {
        return 1;
    }
    // check actual numbers of rows and cols against parameters
    int rowCount = 0;    //actual row count
    int colCount = 0;    //actual col count
    for (int i = 0; i < gridLength; i++) {
        if (grid[i] == '.' || grid[i] == '*' || grid[i] == '#') {
            colCount++;
        } else if (grid[i] == '\n') {
            if (colCount != cols) {
                return 1;
            }
            colCount = 0;
            rowCount++;
        }
    }
    if (rowCount != rows) {
        return 1;
    }
    if (nextTile >= tilesCount || nextTile < 0) {
        return 1;
    }
    return 0;
}

/* Given a grid of dimensions rows*cols, find the x-coordinate of the character
   at the specified index of the char array representing the grid (top left has
     coordinate (0,0)). */
int index_to_x_coordinate(int rows, int cols, int index) {
    int yCounter = -1;
    int xCounter = -1;
    for (int i = 0; i <= index; i++) {
        xCounter++;
        if (xCounter == 0) {
            yCounter++;
        } else if (xCounter == cols) {
            xCounter = -1;
        }
    }
    return xCounter;
}

/* Given a grid of dimensions rows*cols, find the y-coordinate of the character
   at the specified index of the char array representing the grid (top left has
     coordinate (0,0)). */
int index_to_y_coordinate(int rows, int cols, int index) {
    int yCounter = -1;
    int xCounter = -1;
    for (int i = 0; i <= index; i++) {
        xCounter++;
        if (xCounter == 0) {
            yCounter++;
        } else if (xCounter == cols) {
            xCounter = -1;
        }
    }
    return yCounter;
}

/* Does the opposite of the function above. Here y represents the row number
   and x represents the column number, both starting at 0 from top left. */
int coordinate_to_index(int rows, int cols, int y, int x) {
    return y * (cols + 1) - 1 + (x + 1);
}

/* Rotates a tile by 90 degrees. It is assumed that the input is valid (a char
     array of length 30 containing a tile with valid dimensions). */
char* rotate_tile_90(char* grid) {
    // the rotated tile stored as a string to be returned
    char* rotated = malloc(sizeof(char) * 30);
    for (int i = 0; i < 30; i++) {
        int currentRow = index_to_y_coordinate(5, 5, i);
        int currentCol = index_to_x_coordinate(5, 5, i);
        if (currentCol == -1) {
            rotated[i] = '\n';
        } else if (currentRow <= 4 && currentCol <= 4) {
            // the row (y-coordinate) of the "rotated" tile that corresponds to
            // the element at index i of "grid"
            int checkRow = currentCol;
            // same as above but the x-coordinate
            int checkCol = currentRow;
            checkCol = 4 - checkCol;
            // convert (checkCol, checkRow) to index in the string
            int checkIndex = coordinate_to_index(5, 5, checkRow, checkCol);
            if (grid[i] == '!') {
                rotated[checkIndex] = '!';
            } else {
                rotated[checkIndex] = ',';
            }
        }
    }
    return rotated;
}

/* Rotates a tile by 180 degrees. It is assumed that the input is valid (a char
     array of length 30 containing a tile with valid dimensions). */
char* rotate_tile_180(char* grid) {
    char* rotated = malloc(sizeof(char) * 30);
    for (int i = 0; i < 30; i++) {
        int currentRow = index_to_y_coordinate(5, 5, i);
        int currentCol = index_to_x_coordinate(5, 5, i);
        if (currentCol == -1) {
            rotated[i] = '\n';
        } else if (currentRow <= 4 && currentCol <= 4) {
            int checkRow = currentRow;
            int checkCol = currentCol;
            checkRow = 4 - checkRow;
            checkCol = 4 - checkCol;
            int checkIndex = coordinate_to_index(5, 5, checkRow, checkCol);
            if (grid[i] == '!') {
                rotated[checkIndex] = '!';
            } else {
                rotated[checkIndex] = ',';
            }
        }
    }
    return rotated;
}

/* Rotates a tile by 270 degrees. It is assumed that the input is valid (a char
     array of length 30 containing a tile with valid dimensions). */
char* rotate_tile_270(char* grid) {
    char* rotated = malloc(sizeof(char) * 30);
    for (int i = 0; i < 30; i++) {
        int currentRow = index_to_y_coordinate(5, 5, i);
        int currentCol = index_to_x_coordinate(5, 5, i);
        if (currentCol == -1) {
            rotated[i] = '\n';
        } else if (currentRow <= 4 && currentCol <= 4) {
            int checkRow = currentCol;
            int checkCol = currentRow;
            checkRow = 4 - checkRow;
            int checkIndex = coordinate_to_index(5, 5, checkRow, checkCol);
            if (grid[i] == '!') {
                rotated[checkIndex] = '!';
            } else {
                rotated[checkIndex] = ',';
            }
        }
    }
    return rotated;
}

/* Rotates a tile by the specified angle. It is assumed that the input is valid
   (a char array of length 30 containing a tile with valid dimensions). Returns
     NULL if an invalid angle is parsed.*/
char* rotate_tile(char* grid, int angle) {
    switch (angle) {
        case 0:
            {
                char* a = malloc(sizeof(char) * 30);
                memcpy(a, grid, sizeof(char) * 30);
                return a;
            }
        case 90:
            return rotate_tile_90(grid);
        case 180:
            return rotate_tile_180(grid);
        case 270:
            return rotate_tile_270(grid);
        default:
            return NULL;
    }
}

/* Given a file stream to the tile file, reads the tiles file for the tiles and
returns an array of char arrays with each char array holding one tile. */
char** get_tiles(FILE* tileFile) {
    if (check_tile_file(tileFile) == 1) {
        return NULL;
    }
    int next = 0;
    int tilesCount = get_tiles_count(tileFile);
    rewind(tileFile);
    char** tiles = malloc(sizeof(char*) * tilesCount);
    for(int i = 0; i < tilesCount; i++) {
        tiles[i] = malloc(sizeof(char) * 30);
    }
    int start = 0;
    for(int j = 0; j < tilesCount; j++) {
        for(int i = start; i < start + 30; i++) {
            next = fgetc(tileFile);
            tiles[j][i % 32] = next;
        }
        start += 32;
        next = fgetc(tileFile);
    }
    return tiles;
}

/* Given a file stream to the tile file, finds and returns the number of
tiles contained in the tile file. */
int get_tiles_count(FILE* tileFile) {
    rewind(tileFile);
    int next = 0;
    int countChars = 0;
    while (1) {
        next = fgetc(tileFile);
        if (next == EOF) {
            return countChars / 25;
        } else if (next == ',' || next == '!') {
            countChars++;
        }
    }
}

/* Given the array of char arrays which contains the tiles in the tile file and
the number of tiles, display to stdout the tiles with their rotations. */
void shape_display(char** tiles, int tilesCount) {
    for (int i = 0; i < tilesCount; i++) {
        char* rotated0 = rotate_tile(tiles[i], 0);
        char* rotated90 = rotate_tile(tiles[i], 90);
        char* rotated180 = rotate_tile(tiles[i], 180);
        char* rotated270 = rotate_tile(tiles[i], 270);

        rotated0[29] = '\0';
        rotated90[29] = '\0';
        rotated180[29] = '\0';
        rotated270[29] = '\0';

        // display the different rotations side by side
        for (int j = 0; j <= 24; j += 6) {
            char col1[6] = "";
            char col2[6] = "";
            char col3[6] = "";
            char col4[6] = "";
            strncpy(col1, rotated0 + j, 5);
            strncpy(col2, rotated90 + j, 5);
            strncpy(col3, rotated180 + j, 5);
            strncpy(col4, rotated270 + j, 5);
            printf("%s %s %s %s\n", col1, col2, col3, col4);
        }

        // if this is the last set of rotations, output a new line to stdout to
        // follow the specification
        if(i <= tilesCount - 2) {
            printf("\n");
        }

        free(rotated0);
        free(rotated90);
        free(rotated180);
        free(rotated270);
    }
}

/* A function that frees the "tiles" array in the main function. */
void free_tiles_mem(char** tiles, int tilesCount) {
    for (int i = 0; i < tilesCount; i++) {
        free(tiles[i]);
    }
    free(tiles);
}

/* Checks whether the tile file is correctly formatted. Returns 1 if no,
   otherwise return 0. */
Bool check_tile_file(FILE* tileFile) {
    rewind(tileFile);
    int next = fgetc(tileFile);
    int length = 0;
    while (next != EOF) {
        length++;
        next = fgetc(tileFile);
    }
    // check that the length is correct
    if (length < 30) {
        return 1;
    }
    int tilesCount = (length - 30) / 31 + 1;
    int currentLength = 0;
    rewind(tileFile);
    // check that the tiles are correctly formatted (5*5 for each tile)
    for (int i = 0; i < tilesCount; i++) {
        int rowCount = 0;
        int colCount = 0;
        for (int j = 0; j < 30; j++) {
            next = fgetc(tileFile);
            currentLength++;
            if (next == ',' || next == '!') {
                colCount++;
            } else if (next == '\n') {
                if (colCount != 5) {
                    return 1;
                }
                colCount = 0;
                rowCount++;
            }
        }
        if (rowCount != 5) {
            return 1;
        }
        next = fgetc(tileFile);
        currentLength++;
    }
    if (currentLength - 1 != length) {
        return 1;
    }
    return 0;
}

/* Attempts to place a tile. If invalid, return NULL. Otherwise, return the
     correctly updated board. */
char* place_tile(char* board, int boardLength, int boardHeight, char* tile,
        int player, int y, int x, int angle) {
    char marker = '*'; // the current player's marker on the board
    if (player == 1) {
        marker = '#';
    }
    char* rotatedTile = rotate_tile(tile, angle);
    if (rotatedTile == NULL) {
        return NULL;
    }
    int length = (boardLength + 1) * boardHeight; // length of the board
    char* returnBoard = malloc(sizeof(char) * length); // board to return
    memcpy(returnBoard, board, sizeof(char) * length);

    int markersCount = count_tile_markers(rotatedTile);
    int displacementY = y - 2;
    int displacementX = x - 2;
    int maxX = boardLength - 1;
    int maxY = boardHeight - 1;
    int replacementCount = 0;

    for (int i = 0; i < 30; i++) {
        if (!((i - 5) % 6 == 0)) {
            int row = index_to_y_coordinate(5, 5, i);
            int col = index_to_x_coordinate(5, 5, i);
            row += displacementY;
            col += displacementX;
            if (row >= 0 && row <= maxY && col >= 0 && col <= maxX) {
                if (rotatedTile[i] == '!') {
                    int index = coordinate_to_index(boardHeight, boardLength,
                            row, col);
                    // if the current position is already occupied on the board
                    if (returnBoard[index] != '.') {
                        free(returnBoard);
                        free(rotatedTile);
                        return NULL;
                    }
                    returnBoard[index] = marker;
                    replacementCount++;
                }
            }
        }
    }
    // check whether the tile goes off the board
    if (replacementCount != markersCount) {
        free(returnBoard);
        free(rotatedTile);
        return NULL;
    }
    free(rotatedTile);
    return returnBoard;
}

/* Given char arrays containing the board and the tile to be placed, and ints
containing the length of the board, height of the board, player number, x and y
coordinates of the point where the tile is to be placed and the rotation angle,
determine if the placement is valid. If valid, return 1; else return 0. This
function is simply an adaptation of the "place_tile" function above. */
Bool valid_tile_placement(char* board, int boardLength, int boardHeight,
        char* tile, int player, int y, int x, int angle) {
    char marker = '*';
    if (player == 1) {
        marker = '#';
    }
    char* rotatedTile = rotate_tile(tile, angle);
    int length = (boardLength + 1) * boardHeight;
    char* returnBoard = malloc(sizeof(char) * length);
    memcpy(returnBoard, board, sizeof(char) * length);

    int markersCount = count_tile_markers(rotatedTile);
    int displacementY = y - 2;
    int displacementX = x - 2;
    int maxX = boardLength - 1;
    int maxY = boardHeight - 1;
    int replacementCount = 0;

    for (int i = 0; i < 30; i++) {
        if (!((i - 5) % 6 == 0)) {
            int row = index_to_y_coordinate(5, 5, i);
            int col = index_to_x_coordinate(5, 5, i);
            row += displacementY;
            col += displacementX;
            if (row >= 0 && row <= maxY && col >= 0 && col <= maxX) {
                if (rotatedTile[i] == '!') {
                    int index = coordinate_to_index(boardHeight, boardLength,
                            row, col);
                    if (returnBoard[index] != '.') {
                        free(returnBoard);
                        free(rotatedTile);
                        return 0;
                    }
                    returnBoard[index] = marker;
                    replacementCount++;
                }
            }
        }
    }
    free(rotatedTile);
    free(returnBoard);
    if (replacementCount != markersCount) {
        return 0;
    }
    return 1;
}

/* Given a char array containing a single tile, determine the number of
'markers' (i.e. '!' as opposed to '.') in the tile. */
int count_tile_markers(char* tile) {
    int counter = 0;
    for (int i = 0; i < 30; i++) {
        if (tile[i] == '!') {
            counter++;
        }
    }
    return counter;
}

/* Reads a human player's input and returns a String of the player's input.
If EOF detected, return NULL. */
char* read_line(FILE* file) {
    int i = 0;
    int next = 0;
    char* result = malloc(sizeof(char) * 71);

    while (!feof(file)) {
        next = fgetc(file);
        if(next == '\n') {
            result[i] = '\0';
            return result;
        } else {
            result[i++] = (char)next;
        }
    }
    // EOF detected - check if line entered so far is a valid move
    result[i - 1] = '\0';
    int* check2 = get_human_input(result);
    if (check2 == NULL) {
        free(result);
        return NULL;
    }
    free(check2);
    return result;
}

/* Given char arrays each containing the player type the user has input as
arguments, checks whether the user's input player types are valid. If valid,
return 1; else return 0. */
Bool valid_player_types(char* pType1, char* pType2) {
    int pType1Cmp1 = strcmp(pType1, "1");
    int pType1Cmp2 = strcmp(pType1, "2");
    int pType1Cmph = strcmp(pType1, "h");
    int pType2Cmp1 = strcmp(pType2, "1");
    int pType2Cmp2 = strcmp(pType2, "2");
    int pType2Cmph = strcmp(pType2, "h");
    int pType1Valid = !pType1Cmp1 || !pType1Cmp2 || !pType1Cmph;
    int pType2Valid = !pType2Cmp1 || !pType2Cmp2 || !pType2Cmph;
    if (pType1Valid && pType2Valid) {
        return 1;
    }
    return 0;
}

/* A function that manages placement of tiles, given a pointer to a Game
struct that contains the current game, and arrays containing the player types,
the tiles in the game and the "recentPlays" array which contains the most
recent moves by player 1, player 2 and either. */
int move(Game* currentGameP, char* pType1, char* pType2, char** tiles,
        int** recentPlays) {
    Game currentGame = *currentGameP;
    int player = currentGame.nextPlayer;
    char* pType;
    if (player == 0) {
        pType = pType1;
    } else {
        pType = pType2;
    }
    if (strcmp(pType, "h") == 0) {
        while(1) {
            prompt_player(player);
            char* input = read_line(stdin);
            if (input == NULL) {
                return 2;
            }
            Bool unsuccessfulPlacement = human_move(currentGameP, tiles,
                    recentPlays, input);
            if (unsuccessfulPlacement == 0) {
                free(input);
                return 0;
            }
            free(input);
        }
    } else if (strcmp(pType, "1") == 0) {
        automated_move_1(currentGameP, tiles, recentPlays);
    } else {
        automated_move_2(currentGameP, tiles, recentPlays);
    }
    return 0;
}

/* Checks a user's input for whether they have entered a save command.
   Returns the output file's path if yes. */
char* check_save_command(char* input) {
    // excluding "save", the number of characters of the output path (i.e. the
    // user's input) is 66 characters; so 67 after adding '\0' ("outputFile" is
    // intended to be a string)
    char* outputFile = malloc(sizeof(char) * 67);
    if (input[0] == 's' && input[1] == 'a' && input[2] == 'v' &&
            input[3] == 'e') {
        for (int i = 4; i < 71; i++) {
            outputFile[i - 4] = input[i];
            if (input[i] == '\0') {
                return outputFile;
            }
        }
    }
    free(outputFile);
    return NULL;
}

/* Attempts to save the game. Returns 0 if game successfully saved. Otherwise
   return 1. */
int save_game(Game currentGame, char* outputPath) {
    FILE* output = fopen(outputPath, "w");
    if (output == NULL) {
        return 1;
    }
    int nextPlayer = currentGame.nextPlayer;
    int nextTile = currentGame.nextTile;
    int rows = currentGame.rows;
    int cols = currentGame.cols;
    char* grid = currentGame.grid;
    fprintf(output, "%d %d %d %d\n", nextTile, nextPlayer, rows, cols);
    fwrite(grid, sizeof(char), rows * (cols + 1), output);
    fclose(output);
    return 0;
}

/* Processes a human user's input. Returns 0 if a successful move is made; 1
if unsuccessful (either unsuccessful tile placement or invalid command); 3 if
a save file command is successfully processed. */
int human_move(Game* currentGameP, char** tiles, int** recentPlays,
        char* input) {
    Game currentGame = *currentGameP;
    char* board = currentGame.grid;
    int boardLength = currentGame.cols;
    int boardHeight = currentGame.rows;
    char* tile = tiles[currentGame.nextTile];
    int player = currentGame.nextPlayer;

    //in case the user has entered a save command instead of a move
    char* outputPath = check_save_command(input);
    if (outputPath != NULL) {
        int saveStatus = save_game(currentGame, outputPath);
        free(outputPath);
        if(saveStatus == 1) {
            fprintf(stderr, "Unable to save game\n");
            return 1;
        } else {
            return 3;
        }
    }

    int* result = get_human_input(input);
    if (result == NULL) {
        return 1;
    }
    char* updatedBoard = place_tile(board, boardLength, boardHeight, tile,
            player, result[0], result[1], result[2]);
    if (updatedBoard == NULL) {
        free(result);
        return 1;
    }
    // update the game's board
    memcpy(currentGameP->grid, updatedBoard,
            sizeof(char) * (boardLength + 1) * boardHeight);
    // update recentPlays
    recentPlays[player][0] = result[0];
    recentPlays[player][1] = result[1];
    recentPlays[2][0] = result[0];
    recentPlays[2][1] = result[1];
    free(result);
    free(updatedBoard);
    return 0;
}

/* Converts a human player's input into integers that represent a move and
returns this information as an int array. Returns NULL if invalid format. */
int* get_human_input(char* input) {
    int parameterCounter = 0;
    int length = strlen(input);
    // check for shorter than minimum length
    if (length < 5) {
        return NULL;
    }
    // check for trailing space
    if (input[length - 1] == ' ') {
        return NULL;
    }
    // a str array containing the human player's three numbers (but as a str)
    char** resultStrs = malloc(sizeof(char*) * 3);
    for (int i = 0; i < 3; i++) {
        resultStrs[i] = malloc(sizeof(char) * 71);
    }

    int j = 0;
    for(int i = 0; i <= length; i++) {
        if (input[i] == ' ' || input[i] == '\0') {
            resultStrs[parameterCounter][j] = '\0';
            parameterCounter++;
            j = 0;
        } else if (parameterCounter >= 3) {
            free_input_mem(resultStrs);
            return NULL;
        } else {
            if (isdigit(input[i]) || input[i] == '-') {
                resultStrs[parameterCounter][j] = input[i];
                j++;
            } else {
                free_input_mem(resultStrs);
                return NULL;
            }
        }
    }

    if (parameterCounter != 3) {
        free_input_mem(resultStrs);
        return NULL;
    }

    int* resultInts = malloc(sizeof(int) * 3);
    resultInts[0] = strtol(resultStrs[0], NULL, 10);
    resultInts[1] = strtol(resultStrs[1], NULL, 10);
    resultInts[2] = strtol(resultStrs[2], NULL, 10);
    free_input_mem(resultStrs);
    return resultInts;
}

/* Displays the player prompt for human players, given an int of the player
number. */
void prompt_player(int player) {
    if (player == 0) {
        printf("Player *] ");
    } else {
        printf("Player #] ");
    }
}

/* Frees the "input" array used in the function get_human_input().  */
void free_input_mem(char** resultStrs) {
    for (int i = 0; i < 3; i++) {
        free(resultStrs[i]);
    }
    free(resultStrs);
}

/* Assigns rStart and cStart values for automated player 1. */
void a1_assign_initial_values(int** recentPlays, int* r, int* c) {
    int* mostRecentPlay = recentPlays[2];
    int rStart;
    int cStart;
    if (mostRecentPlay[0] == -10 || mostRecentPlay[1] == -10) {
        rStart = -2;
        cStart = -2;
    } else {
        rStart = mostRecentPlay[0];
        cStart = mostRecentPlay[1];
    }
    *r = rStart;
    *c = cStart;
}

/* Processes automated player 1's moves.  */
int automated_move_1(Game* currentGame, char** tiles, int** recentPlays) {
    char* board = currentGame->grid;
    char* tile = tiles[currentGame->nextTile];
    int boardLength = currentGame->cols, boardHeight = currentGame->rows,
            player = currentGame->nextPlayer, rStart, cStart;
    a1_assign_initial_values(recentPlays, &rStart, &cStart);
    int r = rStart, c = cStart, theta = 0;
    do {
        do {
            char* updatedBoard = place_tile(board, boardLength, boardHeight,
                    tile, player, r, c, theta);
            if (updatedBoard != NULL) {
                memcpy(currentGame->grid, updatedBoard,
                        sizeof(char) * (boardLength + 1) * boardHeight);
                recentPlays[player][0] = r;
                recentPlays[player][1] = c;
                recentPlays[2][0] = r;
                recentPlays[2][1] = c;
                automated_display(player, r, c, theta);
                free(updatedBoard);
                return 0;
            }
            c += 1;
            if (c > boardLength + 1) {
                c = -2;
                r += 1;
            }
            if (r > boardHeight + 1) {
                r = -2;
            }
        } while (!(r == rStart && c == cStart));
        theta += 90;
    } while (theta <= 270);
    return 1;
}

/* Assigns rStart and cStart values for automated player 2. */
void a2_assign_initial_values(Game* currentGame, int** recentPlays, int* r,
        int* c) {
    int boardLength = currentGame->cols;
    int boardHeight = currentGame->rows;
    int player = currentGame->nextPlayer;
    int* mostRecentPlay = recentPlays[player];
    int rStart;
    int cStart;
    if (mostRecentPlay[0] == -10 || mostRecentPlay[1] == -10) {
        if (player == 0) {
            rStart = -2;
            cStart = -2;
        } else {
            rStart = boardHeight + 1;
            cStart = boardLength + 1;
        }
    } else {
        rStart = mostRecentPlay[0];
        cStart = mostRecentPlay[1];
    }
    *r = rStart;
    *c = cStart;
}

/* Processes automated player 2's moves.  */
int automated_move_2(Game* currentGame, char** tiles, int** recentPlays) {
    char* board = currentGame->grid;
    char* tile = tiles[currentGame->nextTile];
    int boardLength = currentGame->cols, boardHeight = currentGame->rows,
            player = currentGame->nextPlayer, rStart, cStart;
    a2_assign_initial_values(currentGame, recentPlays, &rStart, &cStart);
    int r = rStart, c = cStart;
    do {
        int theta = 0;
        do {
            char* updatedBoard = place_tile(board, boardLength, boardHeight,
                    tile, player, r, c, theta);
            if (updatedBoard != NULL) {
                memcpy(currentGame->grid, updatedBoard,
                        sizeof(char) * (boardLength + 1) * boardHeight);
                recentPlays[player][0] = r;
                recentPlays[player][1] = c;
                recentPlays[2][0] = r;
                recentPlays[2][1] = c;
                automated_display(player, r, c, theta);
                free(updatedBoard);
                return 0;
            }
            theta += 90;
        } while (theta <= 270);
        if (player == 0) {
            c++;
            if (c > boardLength + 1) {
                c = -2;
                r++;
            }
            if (r > boardHeight + 1) {
                r = -2;
            }
        } else {
            c--;
            if (c < -2) {
                c = boardLength + 1;
                r--;
            }
            if (r < -2) {
                r = boardHeight + 1;
            }
        }
    } while (!(r == rStart && c == cStart));
    return 1;
}

/* Print to stdout an automated player's move, given the player number,
x and y coordinstes of the tile placement position and the rotation angle. */
void automated_display(int player, int r, int c, int theta) {
    if (player == 0) {
        printf("Player * => %d %d rotated %d\n", r, c, theta);
    } else {
        printf("Player # => %d %d rotated %d\n", r, c, theta);
    }
}
