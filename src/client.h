/*
  
  client.h
  ========

  Author: Robert Correiro

  Client structs and helpers.

*/

#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"  /* game_state enum */

#define SERV_ADDR_LEN 15

typedef struct {
  int sid;
  int ori;
  int col;
  int row;
  int len;
} ship;

typedef struct {
  char row;
  char col;
} move_rc;

typedef struct {
  char *serv_ip;
  int uid;
} request;

typedef struct {
  char ready;
  char my_turn;
  char status;
  game_state state;
} response;

#endif