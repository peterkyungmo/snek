#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
  // initialize static variables
  unsigned int num_cols = 20;
  char* wall = "####################";
  char* space = "#                  #";
  char* snake_fruit = "# d>D    *         #";
  // allocate gamestate attribute pointers and initialize variabels
  game_state_t* state = (game_state_t*)malloc(sizeof(game_state_t));
  state->num_rows = 18;
  state->num_snakes = 1;
  state->board = (char**)malloc(state->num_rows * sizeof(char*));
  state->snakes = (snake_t*)malloc(state->num_snakes * sizeof(snake_t));
  // initialize board
  for (int i = 0; i < state->num_rows; i++) {
    state->board[i] = (char*)malloc(num_cols * sizeof(char));
    if (i == 0 || i == state->num_rows-1) {
      strcpy(state->board[i], wall);
    } else if (i == 2) {
      strcpy(state->board[i], snake_fruit);
    } else {
      strcpy(state->board[i], space);
    }
  }
  // initialize snake
  struct snake_t snake = {2, 2, 2, 4, true};
  state->snakes[0] = snake;
  return state;
}

/* Task 2 */
void free_state(game_state_t* state) {
  free(state->snakes);
  for (int r = 0; r < state->num_rows; r++) {
    free(state->board[r]);
  }
  free(state->board);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  // TODO: Implement this function.s
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s", strcat(state->board[i], "\n"));
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  if (c == '^' || c == '<' || c == 'v' || c == '>') {
    return true;
  } else if (is_head(c) || is_tail(c)) {
    return true;
  }
  return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  if (c == '^') {
    return 'w';
  } else if (c == '<') {
    return 'a';
  } else if (c == 'v') {
    return 's';
  }
  return 'd';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  if (c == 'w') {
    return '^';
  } else if (c == 'a') {
    return '<';
  } else if (c == 's') {
    return 'v';
  }
  return '>';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') {
    return cur_row + 1;
  } else if (c == '^' || c == 'w' || c == 'W') {
    return cur_row - 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') {
    return cur_col + 1;
  } else if (c == '<' || c == 'a' || c == 'A') {
    return cur_col - 1;
  } else {
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  unsigned int row = state->snakes[snum].head_row;
  unsigned int col = state->snakes[snum].head_col;
  unsigned int next_row = get_next_row(row, state->board[row][col]);
  unsigned int next_col = get_next_col(col, state->board[row][col]);
  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  // TODO: Implement this function.
  return;
}

/* Task 5 */
game_state_t* load_board(FILE* fp) {
  // TODO: Implement this function.
  return NULL;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  // TODO: Implement this function.
  return NULL;
}
