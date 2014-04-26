/*
  
  response.h
  ========

  Author: Robert Correiro

  Interface to server's response handling functions.

*/

#ifndef RESPONSE_H
#define RESPONSE_H

typedef struct {

} response;

response build_response(int sockfd, battleship *game, unsigned type);

#endif