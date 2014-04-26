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
  player players[2];
  board p1;
  board p2;
} battleship;

#endif