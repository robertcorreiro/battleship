/*
  
  server.h
  ========

  Author: Robert Correiro

  Interface to server helpers.

*/

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

#define BOARD_LEN 10
#define MAX_BUFF_LEN 512

#define vprintf(format, ...) do {            \
  if (verbose)                               \
    fprintf(stderr, format, ##__VA_ARGS__);  \
} while (0);

extern int verbose;

// typedef struct {
//   server_state state;
// } server;

typedef enum {
  WAITING,
  SETUP,
  PLAY,
  FINAL
} game_state;

typedef enum {
  JOIN,
  INIT,
  MOVE,
  POLL
} message_type;

typedef struct {
  int uid;
} player;

typedef struct {
  int setup;
  int hits;
  char ships[BOARD_LEN][BOARD_LEN];
  char guesses[BOARD_LEN][BOARD_LEN];
} board;

typedef struct {
  game_state state;
  player p1;
  player p2;
  int sync;
  int turn;
  board p1_board;
  board p2_board;
  int last_guess_x;
  int last_guess_y;
} battleship;

#endif
