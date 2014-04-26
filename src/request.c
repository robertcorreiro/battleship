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
  message msg_out, msg_in;

  /* Read message into buffer */
  msg_in.len = read(sockfd, msg_in.buf, MAX_BUFF_LEN);

  /* Call to FSM */
  build_response(game, &msg_in, &msg_out);  

  /* SEND RESPONSE HERE */
  send(sockfd,msg_out.buf,msg_out.len,0);
  close(sockfd);
}

