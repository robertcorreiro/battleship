#include "server.h"  /* server_state enum */
#include "helpers.h"
#include "response.h"
#include <stdio.h> /* printf() */
#include <string.h>
unsigned get_message_type(message *msg_in) {
  return (unsigned) (unsigned char) msg_in->buf[0];
}

unsigned get_player_id(message *msg_in) {
  return (msg_in->len >= 5) ? *(unsigned *)(msg_in->buf+1) : 0;
}

int setup_board(message *msg_in, board *bd){
  memset(bd,'\0',sizeof(*bd));
  int i,j;
  int offset = 5;
  int type_lengths[] = {2,3,3,4,5};
  int type;
  int dir;
  int length;
  int x, y;
  for(i=0;i<5;i++){
    type = msg_in->buf[offset];
    dir = msg_in->buf[offset+1];
    type &= ((~0)>>1);
    type &= (int)(~(char)0);
    if(type > 4 || type < 0){
      vprintf("ship %d: bad type: %x (%d)\n", i, type,type);
      return 1;
    }
    length = type_lengths[type];
    for(x=msg_in->buf[offset+2],y=msg_in->buf[offset+3],j=0;j<length;j++,(dir)?x++:y++){
      if(x < 0 || x > BOARD_LEN || y < 0 || y > BOARD_LEN){
        vprintf("ship %d: ran off board\n", i);
        return 2;
      }
      if(bd->ships[y][x] == SHIP){
        vprintf("ship %d: ship collision\n", i);
        return 3;
      }
      bd->ships[y][x] = SHIP;
    }
    offset += 4;
  }
  return 0;
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
      break;
    case SETUP:
      vprintf("game->state: SETUP\n");
      switch(type){
        case INIT:
          {
            board *new_board;
            int playerID = get_player_id(msg_in);
            if(playerID == 0){
              vprintf("bad packet\n");
              return;
            }else if(playerID == game->p1.uid){
              vprintf("got init from p1\n");
              new_board = &(game->p1_board);
            }else if(playerID == game->p2.uid){
              vprintf("got init from p2\n");
              new_board = &(game->p2_board);
            }else{
              vprintf("unknown uid\n");
              return;
            }
            if(setup_board(msg_in,new_board)){
              vprintf("error making board!\n");
            }else{
              vprintf("board successfully created!\n");
            }
            int i,j;
            char *icons = "_SF";
            for(i=0;i<BOARD_LEN;i++){
              for(j=0;j<BOARD_LEN;j++){
                vprintf("%c",icons[new_board->ships[i][j]]);
              }
              vprintf("\n");
            }
          }
          break;
        default:
          vprintf("invalid packet type\n");
      }    
      break;
    case PLAY:
      
      break;
    case FINAL:
      
      break;
    default:
      
      break;
  }
}
