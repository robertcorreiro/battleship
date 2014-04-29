#include "server.h"  /* server_state enum */
#include "helpers.h"
#include "response.h"
#include <stdio.h> /* printf() */
#include <string.h>
unsigned get_message_type(message *msg_in) {
  return (unsigned) (unsigned char) msg_in->buf[0];
}

unsigned get_player_id(message *msg_in) {
  return (msg_in->len >= 6) ? *(unsigned *)(msg_in->buf+1) : 0;
}

int setup_board(message *msg_in, board *bd){
  memset(bd,'\0',sizeof(*bd));
  int i,j;
  int offset = 6;
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
    if(type != i){
      vprintf("ship %d: out of order\n",i);
      return;
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
  int playerID = get_player_id(msg_in);
  switch(game->state) {
    case WAITING:
      vprintf("game->state: WAITING\n");
      switch(type) {
        case JOIN:
          if(game->sync == 2){
            vprintf("game is already full!\n");
          }
          if((playerID = get_player_id(msg_in)) != 0){
            if(game->sync){
              game->sync = 0;
              game->p2.uid = playerID;
            }else{
              game->sync = 1;
              game->p1.uid = playerID;
            }
          }
          vprintf("I want to join!\n");
          msg_out->buf[0] = 1 - game->sync;
          msg_out->len = 1;
          break;
        case POLL:
          vprintf("got a poll!\n");
          playerID = get_player_id(msg_in);
          if(game->p1.uid == playerID){
            msg_out->buf[0] = 1 - game->sync;
            msg_out->len = 1;
            if(!game->sync){
              vprintf("going to SETUP state\n");
              game->state = SETUP;
              game->turn = rand() % 2;
            }
          }
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
            playerID = get_player_id(msg_in);
            if(playerID == 0){
              vprintf("bad packet\n");
              return;
            }else if(playerID == game->p1.uid){
              if(!game->p1_board.setup){
                vprintf("got init from p1\n");
                new_board = &(game->p1_board);
              }else{
                vprintf("p1: no re-initialization!\n");
                return;
              }
            }else if(playerID == game->p2.uid){
              if(!game->p2_board.setup){
                vprintf("got init from p2\n");
                new_board = &(game->p2_board);
              }else{
                vprintf("p2: no re-initialization!\n");
                return;
              }
            }else{
              vprintf("unknown uid\n");
              return;
            }
            int error;
            if(error = setup_board(msg_in,new_board)){
              vprintf("error making board!\n");
            }else{
              vprintf("board successfully created!\n");
              int is_player1 = game->p1.uid == playerID;
              game->sync = !game->sync;
              msg_out->buf[0] = 1 - game->sync;
              msg_out->buf[1] = (game->turn - is_player1) == 0;
              msg_out->len = 2;
            }
            int i,j;
            char *icons = "~#@";
            for(i=0;i<BOARD_LEN;i++){
              for(j=0;j<BOARD_LEN;j++){
                vprintf("%c",icons[new_board->ships[i][j]]);
              }
              vprintf("\n");
            }
          }
          break;
        case POLL:
          vprintf("got a poll!\n");
          playerID = get_player_id(msg_in);
          if(game->p1.uid == playerID || game->p2.uid == playerID){
            int is_player1 = game->p1.uid == playerID;
            msg_out->buf[0] = 1 - game->sync;
            msg_out->buf[1] = (game->turn - is_player1) == 0;
            msg_out->len = 2;
            if(!game->sync){
              vprintf("going to PLAY state\n");
              game->state = PLAY;
            }
          }
          break;
        default:
          vprintf("invalid packet type\n");
      }    
      break;
    case PLAY:
      switch(type){
        case MOVE:
          {
            player p = (game->turn)?game->p1:game->p2;
            player other_p = (!(game->turn))?game->p1:game->p2;
            board *bd = (game->turn)?&(game->p1_board):&(game->p2_board);
            board *other_bd = (!(game->turn))?&(game->p1_board):&(game->p2_board);
            playerID = get_player_id(msg_in);
            if(playerID == p.uid){
              vprintf("got a move from player %d\n",1 + game->sync);
              int x, y;
              x = msg_in->buf[6];
              y = msg_in->buf[7];
              if(bd->guesses[x][y] == NOTHING){
                int hit = other_bd->ships[y][x] == SHIP;
                bd->guesses[x][y] = 1 + hit;
                other_bd->hits += hit;
                msg_out->buf[0] = other_bd->hits == 17;
                msg_out->buf[1] = hit;
                game->turn = !game->turn;
                game->last_guess_x = x;
                game->last_guess_y = y;
              }else{
                msg_out->buf[0] = 0;
                msg_out->buf[1] = -1;
              }
              msg_out->len = 2;
              int i,j;
              char *icons = "~#@";
              for(i=0;i<BOARD_LEN;i++){
                for(j=0;j<BOARD_LEN;j++){
                  vprintf("%c",icons[game->p1_board.ships[i][j]]);
                }
                vprintf(" ");
                for(j=0;j<BOARD_LEN;j++){
                  vprintf("%c",icons[game->p2_board.ships[i][j]]);
                }
                vprintf("\n");
              }
            }else if(playerID == other_p.uid){
              vprintf("got an out of turn move from player %d\n",1+game->turn);
              msg_out->buf[0] = -1;
              msg_out->len = 1;
            }else{
              vprintf("invalid uid\n");
            }
            break;
          }
        case POLL:
          playerID = get_player_id(msg_in);
          int done;
          if(playerID == game->p1.uid){
            vprintf("got a poll from player 1\n");
            done = game->p1_board.hits == 17;
            if(done) game->state = WAITING;
            msg_out->buf[0] = done;
            msg_out->buf[1] = game->turn;
            msg_out->buf[2] = game->last_guess_x;
            msg_out->buf[3] = game->last_guess_y;
            msg_out->len = 4;
          }else if(playerID == game->p2.uid){
            vprintf("got a poll from player 2\n");
            done = game->p2_board.hits == 17;
            if(done) game->state = WAITING;
            msg_out->buf[0] = done;
            msg_out->buf[1] = !game->turn;
            msg_out->buf[2] = game->last_guess_x;
            msg_out->buf[3] = game->last_guess_y;
            msg_out->len = 4;
          }else{
            vprintf("invalid uid\n");
          }
          if(msg_out->buf[0]){
            memset(game,'\0',sizeof(*game));
            game->state = WAITING;
          }
          break;
        default:
          
          break;
      }
      break;
    case FINAL:
      
      break;
    default:
      
      break;
  }
}
