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
  char buff_in[MAX_BUFF_LEN];
  char buff_out[MAX_BUFF_LEN];

  /* Read message into buffer */
  read(sockfd, &buff_in, MAX_BUFF_LEN);

  /* Call to FSM */
  build_response(game, buff_in, buff_out);  

  /* SEND RESPONSE HERE */
}