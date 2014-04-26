/*
  
  request_handler.c
  ========

  Author: Robert Correiro

  Implementation of server's request handling functionality.

*/

#include "server.h"  /* serv_state enum */
#include "response.h"  /* response struct, build_response(), send_response() */
#include <stdio.h>

int request_handler(int sockfd, battleship *game) {
  unsigned type;
  response res;

  /* Parse first byte for message type */
  if (read(sockfd, &type, 1) != 1) {
    fprintf(stderr, "Failed to get message type\n");
    return -1;  /* TODO: SEND TRY AGAIN MESSAGE */
  }

  printf("%x\n", type);
  printf("%d\n", type);

  /* Call to FSM */
  res = build_response(sockfd, game, type);  

  /* SEND RESPONSE HERE */
}