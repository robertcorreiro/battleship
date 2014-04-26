#include "server.h"  /* server_state enum */
#include "response.h"
#include <stdio.h> /* printf() */

response build_response(int sockfd, battleship *game, unsigned type) {
  response res;

  switch(game->state) {
    case WAITING:
      printf("game->state: WAITING\n");
      switch(type) {
        case JOIN:
          printf("I want to join!\n");
          break;
        default:
          printf("Something else.\n");
          break;
      }

    case SETUP:

    case PLAY:

    case FINAL:

    default:
      break;
  }
}