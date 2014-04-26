#include "server.h"  /* server_state enum */
#include "response.h"
#include <stdio.h> /* printf() */

unsigned get_message_type(char *buff_in) {
  return (unsigned) (unsigned char) buff_in[0];
}

void build_response(battleship *game, char *buff_in, char *buff_out) {
  unsigned type = get_message_type(buff_in);

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