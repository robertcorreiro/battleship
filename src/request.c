/*
  
  request_handler.c
  ========

  Author: Robert Correiro

  Implementation of server's request handling functionality.

*/

#include "server.h"  /* serv_state enum */
#include "response.h"  /* response struct, build_response(), send_response() */
#include <stdio.h>

int get_msg_length(message msg){
  return msg.buf[5];
}

int request_handler(int sockfd, battleship *game) {
  message msg_out, msg_in;
  int length;
  /* Read message into buffer */
  msg_in.len = read(sockfd, msg_in.buf, 6);
  length = get_msg_length(msg_in);
  msg_in.len += length;
  read(sockfd, msg_in.buf+6, length);
  
  /* Call to FSM */
  build_response(game, &msg_in, &msg_out);  

  /* SEND RESPONSE HERE */
  send(sockfd,msg_out.buf,msg_out.len,0);
  close(sockfd);
}

