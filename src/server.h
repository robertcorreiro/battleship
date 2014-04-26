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
  char ships[BOARD_LEN][BOARD_LEN];
  char guess[BOARD_LEN][BOARD_LEN];
} board;

typedef struct {
  game_state state;
  player p1;
  player p2;
  int num_players;
  board p1_board;
  board p2_board;
} battleship;

#endif
