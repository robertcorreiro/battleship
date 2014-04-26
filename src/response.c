#include "server.h"  /* server_state enum */
#include "response.h"
#include <stdio.h> /* printf() */
#include <string.h>
unsigned get_message_type(message *msg_in) {
  return (unsigned) (unsigned char) msg_in->buf[0];
}

unsigned get_player_id(message *msg_in) {
  return (msg_in->len >= 5) ? *(unsigned *)(msg_in->buf+1) : 0;
}
void build_response(battleship *game, message *msg_in, message *msg_out) {
  unsigned type = get_message_type(msg_in);

  switch(game->state) {
    case WAITING:
      vprintf("game->state: WAITING\n");
      switch(type) {
        case JOIN:
          if(game->num_players == 2){
            vprintf("game is already full!\n");
          }
          int playerID;
          if((playerID = get_player_id(msg_in)) != 0){
            if(game->num_players == 1){
              game->num_players++;
              game->p2.uid = playerID;
              vprintf("going to SETUP state\n");
              game->state = SETUP;
            }else{
              game->num_players = 1;
              game->p1.uid = playerID;
            }
          }
          vprintf("I want to join!\n");
          strncpy(msg_out->buf,"welcome!",10);
          msg_out->len = strlen(msg_out->buf);
          break;
        default:
          vprintf("Something else.\n");
          strncpy(msg_out->buf,"you are not welcome!",22);
          msg_out->len = strlen(msg_out->buf);
          break;
      }

    case SETUP:

    case PLAY:

    case FINAL:

    default:
      break;
  }
}
