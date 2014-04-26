/*
  
  response.h
  ========

  Author: Robert Correiro

  Interface to server's response handling functions.

*/

#ifndef RESPONSE_H
#define RESPONSE_H

#include "server.h"

typedef struct message{
  char buf[MAX_BUFF_LEN];
  int len;
} message;

void build_response(battleship *game, message *msg_in, message *msg_out);

#endif
