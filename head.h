#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char* grid;
    int nextTile;
    int nextPlayer;
    int rows;
    int cols;
} Game;

typedef int Bool;

//function prototypes
void check_arg_count(int argc);
FILE* open_tile_file(char* filename);
char** process_get_tiles(FILE* tileFile);
void check_player_types(char* playerType1, char* playerType2, char** tiles,
        int tilesCount);
void process_shape_display(int argc, char** tiles, int tilesCount,
        FILE* tileFile);
void process_new_game(int argc, char** argv, Game* currentGame, char** tiles,
        int tilesCount, FILE* tileFile, char* playerType1, char* playerType2);
void process_saved_game(int argc, char** argv, Game* currentGame, char** tiles,
        int tilesCount, FILE* tileFile, char* playerType1, char* playerType2);
void print_winner(int nextPlayer);
void play_game(Game* currentGame, char** tiles, int tilesCount,
        char* playerType1, char* playerType2, int** recentPlays,
        FILE* tileFile);
Bool game_over(Game currentGame, char** tiles);
char* read_file(FILE* input, int tilesCount);
char* new_grid(int rows, int cols);
int* read_parameters(FILE* input);
int check_saved_game(int* parameters, char* grid, int gridLength, int
	tilesCount);
void print_grid(Game currentGame);
char* rotate_tile_90(char* grid);
int coordinate_to_index(int rows, int cols, int y, int x);
char* rotate_tile(char* grid, int angle);
char** get_tiles(FILE* tileFile);
int get_tiles_count(FILE* tileFile);
void shape_display(char** tiles, int tilesCount);
void free_tiles_mem(char** tiles, int tilesCount);
Bool check_tile_file(FILE* tileFile);
char* place_tile(char* board, int boardLength, int boardHeight, char* tile, int
        player, int y, int x, int angle);
char* read_line(FILE* file);
int count_tile_markers(char* tile);
Bool valid_player_types(char* pType1, char* pType2);
char* get_marker(int player);
int move(Game* currentGamePtr, char* pType1, char* pType2, char** tiles, int**
        recentPlays);
int human_move(Game* currentGamePtr, char** tiles, int** recentPlays,
        char* input);
int* get_human_input(char* input);
void prompt_player(int player);
void free_input_mem(char** resultStrs);
int automated_move_1(Game* currentGamePtr, char** tiles, int** recentPlays);
void automated_display(int player, int r, int c, int theta);
int automated_move_2(Game* currentGamePtr, char** tiles, int** recentPlays);
int index_to_x_coordinate(int rows, int cols, int index);
int index_to_y_coordinate(int rows, int cols, int index);
char* rotate_tile_270(char* grid);
Bool valid_tile_placement(char* board, int boardLength, int boardHeight,
        char* tile, int player, int y, int x, int angle);
char* check_save_command(char* input);
int save_game(Game currentGame, char* outputPath);
void free_mem_recent_plays(int** recentPlays);
void display_next_tile(char* pType1, char* pType2, int nextPlayer,
        int nextTile, char** tiles);
void a1_assign_initial_values(int** recentPlays, int* r, int* c);
void a2_assign_initial_values(Game* currentGame, int** recentPlays, int* r,
        int* c);
