#include "client.h"
#include "server.h"

#include <stdio.h>

void print_ships_board(char board[BOARD_LEN][BOARD_LEN]) {
  int i, j;
  char row_header = 'A';

  printf("   1  2  3  4  5  6  7  8  9  10\n");
  for(i = 0; i < BOARD_LEN; i++) {
    for(j = 0; j < BOARD_LEN + 1; j++) {
      if (j == 0) {  /* 0th column needs headings */
        printf("%c  ", row_header++);
      } else if (j == BOARD_LEN) {
        printf("%c\n", board[i][j-1]);
      } else {
        printf("%c  ", board[i][j-1]);
      }
    }
  }
  printf("\n");
}

void add_ship_to_board(ship *s, char board[BOARD_LEN][BOARD_LEN]) {
  int i, row = s->row, col = s->col;

  if (s->ori > 0) {  /* +positive => horizontal */
    for (i = 0; i < s->len; i++) {
      board[s->row][col++] = '0' + s->len;
    }
  } else {  /* -negative => vertical */
    for (i = 0; i< s->len; i++) {
      board[row++][s->col] = '0' + s->len;
    }
  }
}

int set_ship_position(ship *s) {
  int i, row, col;

  printf("Enter initial position: ");
  for (i = 0; i < 3; i++) {
    int c = getchar();
    if (c == EOF) {
      if (i == 2) break;
      return -1;
    }
    if (i == 0) row = c - 65;
    if (i == 1) col = c - 49;
  }

  /* validation */
  if (row < 0 || row > 9 || col < 0 || col > 9) {
    return -1;
  }

  s->row = row;
  s->col = col;

  return 1;
}

int set_ship_orientation(ship *s) {
  int i, ori;

  printf("Enter Orientation (H/V): ");
  int c = getchar();
  getchar(); /* hack to get rid of EOF */
  
  switch(c) {
    case 'h':
    case 'H':
      s->ori = 1;
      break;
    case 'v':
    case 'V':
      s->ori = 0;
      break;
    default:
      return -1;
  }

  return 1;
}

int validate_ship_placement(ship *s, char board[BOARD_LEN][BOARD_LEN]) {
  int i;

  // vprintf("row=%d\ncol=%d\nori=%d\nlen=%d\n\n",s->row,s->col,s->ori,s->len);

  /* Horizontal validations */
  if (s->ori) {  
    /* Check running over the edge */
    if (s->col + (s->len) - 1 > 9) return -1;

    /* Horizontal collision? */
    int col = s->col;
    for (i = 0; i < s->len; i++, col++) {
      /* don't allow if something's already in the spot */
      /* TODO: change to WATER instead of 0 */
      if (board[s->row][col] != '0') return -1;
    }
  } 
  /* Vertical validations */
  else {  
    /* Check running over the edge */
    if (s->row + (s->len) - 1 > 9) return -1;

    /* Vertical collision? */
    int row = s->row;
    for (i = 0; i < s->len; i++, row++) {
      /* TODO: change to WATER instead of 0 */
      if (board[row][s->col] != '0') return -1;
    }
  }
  return 1;
}

void setup_game_ships(ship ships[5]) {
  int row, col, rv, i;
  int ship_lens[] = {2, 3, 3, 4, 5};
  char board[BOARD_LEN][BOARD_LEN];
  ship *s;

  /* Initialize ships board to empty */
  for (row = 0; row < BOARD_LEN; row++) {
    for (col = 0; col < BOARD_LEN; col++) {
      board[row][col] = '0';
    }
  }

  print_ships_board(board);

  for (i = 0; i < 5; i++) {
    s = &ships[i];
    s->sid = i;
    s->ori = 1;
    s->len = ship_lens[i];

    do {
      printf("SETUP: Ship %d - Length %d\n", i+1, s->len);
      
      rv = set_ship_position(s);
      while (rv == -1) {
        printf("Invalid position. Please try again.\n");
        rv = set_ship_position(s);
      }

      rv = set_ship_orientation(s);
      while (rv == -1) {
        printf("Invalid orientation. Please try again\n");
        rv = set_ship_orientation(s);
      }

      rv = validate_ship_placement(s, board);
      if (rv == -1) {
        print_ships_board(board);
        printf("Invalid placement. Please try again.\n");
      }
    } while (rv == -1);

    add_ship_to_board(s, board);
    print_ships_board(board);
  }
}