/*
  
  client.c
  ========

  Author: Robert Correiro

  Client program for communicating with battleship server.

*/

#include "client.h"  /* ship struct */
#include "server.h"  /* vprintf() */
#include "response.h"  /* message struct */

#include <stdio.h>  /* fprintf() */
#include <stdlib.h>  /* srand(), rand(), exit() */
#include <string.h>  /* memset() */
#include <unistd.h>  /* getopt() */
#include <time.h>  /* time() for rand seed */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_PORT "5050"  /* TODO: REMOVE THIS */

#define HIT_MARK 'X'
#define MISS_MARK 'O'

int verbose;

// void *get_in_addr(struct sockaddr *sa) {
//   if (sa->sa_family == AF_INET) {
//     return &(((struct sockaddr_in*)sa)->sin_addr);
//   }

//   return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

/* Given a server's IP, returns a socket CONNECTED to it. */
int get_socket(char *serv_ip) {
  int sockfd, rv;
  char s[INET6_ADDRSTRLEN];
  struct addrinfo hints, *results, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  if ((rv = getaddrinfo(serv_ip, SERVER_PORT, &hints, &results)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = results; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  freeaddrinfo(results);

  return sockfd;
}

int send_ships_to_server(request *req, response *res, ship ships[5]) {
  /* TODO: don't open new socket for each call to this function */
  int i, *intptr, sockfd = get_socket(req->serv_ip);
  char rv;
  message msg;

  /* TODO: fix hardcoding */
  msg.len = 26; 
  msg.buf[0] = 1;  /* sets first char of buf (msg_type) to INIT */
  intptr = (int *)&msg.buf[1];  /* sets next 4 bytes to int uid */
  *intptr = req->uid;
  // intptr = (int *)&msg.buf[5];  /* sets 6th byte to len of msg */
  // *intptr = msg.len;
  msg.buf[5] = 20;

  /* TODO: fix hardcoding */
  for (i = 0; i < 5; i++) {
    msg.buf[6 + i*4] = (char) ships[i].sid;
    msg.buf[7 + i*4] = (char) ships[i].ori;
    msg.buf[8 + i*4] = (char) ships[i].col;
    msg.buf[9 + i*4] = (char) ships[i].row;
  }

  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in send_ships_to_server");
    return -1;
  }

  //vprintf("before read: res->ready=%d\n", res->ready);

  if (read(sockfd, &(res->ready), 1) != 1) {
    perror("client: read 1 failed in send_ships_to_server");
    return -1;
  }

  if (read(sockfd, &(res->my_turn), 1) != 1) {
    perror("client: read 2 failed in send_ships_to_server");
    return -1;
  }

  //vprintf("send_ships_to_server:res->ready=%d, res->my_turn=%d\n", res->ready, res->my_turn);

  if (close(sockfd) == -1) {
    perror("client: failed to close socket in send_ships_to_server");
    return -1;
  }
  
  return 1;
}

int send_req_to_server(request *req, response *res, message_type type) {
  /* TODO: don't open new socket for each call to this function */
  int sockfd = get_socket(req->serv_ip), *intptr;
  message msg;

  /* TODO: fix hardcoding */
  switch (type) {
    case JOIN:  /* [JOIN][UID-4bytes][LEN=0] */
      //vprintf("Sending a JOIN\n");
      msg.len = 6;
      msg.buf[0] = 0;
      intptr = (int*)&msg.buf[1];
      *intptr = req->uid; 
      msg.buf[5] = 0;
      break;
    case POLL:  /* [POLL][UID-4bytes][LEN=0] */
      //vprintf("Sending a POLL\n");
      msg.len = 6;
      msg.buf[0] = 3;
      intptr = (int*)&msg.buf[1];
      *intptr = req->uid; 
      msg.buf[5] = 0;
      break;
    case MOVE:  /* [MOVE][UID-4bytes][LEN=2][X(COL)][Y[ROW]]*/
      msg.len = 8;
      msg.buf[0] = 2;
      intptr = (int*)&msg.buf[1];
      *intptr = req->uid; 
      msg.buf[5] = 2;
      msg.buf[6] = req->m.col;
      msg.buf[7] = req->m.row;
      break;
    default:
      break;
  }


  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in send_req_to_server");
    return -1;
  }

  //vprintf("After write()\n");

  /* TODO: fix hardcoding */
  switch(res->state) {
    /* JOIN/POLL => [GO INTO SETUP?] */
    case WAITING:  
      if (read(sockfd, &(res->ready), 1) != 1) {
        perror("client-waiting: failed to read reponse");
        return -1;
      }    
      break;
    
    /* INIT/POLL => [PLAY MODE?][MY TURN?] */
    case SETUP:  
      if (read(sockfd, &(res->ready), 1) != 1) {
        perror("client-setup: failed to read reponse 1");
        return -1;
      }
      
      if (read(sockfd, &(res->my_turn), 1) != 1) {
        perror("client-setup: failed to read reponse 1");
        return -1;
      }
      break;
    
    /* MOVE => [HIT, MISS, or ERROR] */
    /* POLL => [PLAY MODE?][MY TURN?][Opponent's X (col)][Opponent's Y (row)] */
    case PLAY:
      switch(type) {
        case MOVE:
          if (read(sockfd, &(res->ready), 1) != 1) {
            perror("client-move: failed to read reponse 1");
            return -1;
          }
          //vprintf("s: res->ready=%d\n", res->ready);

          if (read(sockfd, &(res->status), 1) != 1) {
            perror("client-move: failed to read reponse 2");
            return -1;
          }
          //vprintf("s: res->status=%d\n", res->status);
          break;

        case POLL:
          if (read(sockfd, &(res->ready), 1) != 1) {
            perror("client-play: failed to read reponse 1");
            return -1;
          }
      
          if (read(sockfd, &(res->my_turn), 1) != 1) {
            perror("client-play: failed to read reponse 2");
            return -1;
          }

          if (read(sockfd, &(res->col), 1) != 1) {
            perror("client-play: failed to read reponse 3");
            return -1;
          }
          //vprintf("s: res->col (x)=%d\n", res->col);
          
          if (read(sockfd, &(res->row), 1) != 1) {
            perror("client-play: failed to read reponse 4");
            return -1;
          }
          //vprintf("s: res->row (y)=%d\n", res->row);

          break;
        default:
          break;
      }
      break;

    default:
      break;
  }

  if (close(sockfd) == -1) {
    perror("client: failed to close join/poll socket");
    return -1;
  }

  return 1;
}

void poll_server(request *req, response *res) {
  res->ready = 0;
  res->my_turn = 0;

  if (res->state == WAITING || res->state == SETUP) {
    while (!res->ready) {
      printf(".");
      fflush(stdout);
      sleep(2);
      send_req_to_server(req, res, POLL);
      // vprintf("\nWAIT/SETUP:\n");
      // vprintf("poll_server:res->ready=%d\n", res->ready);
      // vprintf("poll_server:res->my_turn=%d\n", res->my_turn);
    }
  }

  if (res->state == PLAY) {
    while (!res->my_turn && !res->ready) {
      printf(".");
      fflush(stdout);
      sleep(2);
      send_req_to_server(req, res, POLL);
      // vprintf("\nPLAY:\n");
      // vprintf("poll_server:res->ready=%d\n", res->ready);
      // vprintf("poll_server:res->my_turn=%d\n", res->my_turn);
    }
  }
}

int get_move(move_rc *m) {
  int i, row, col;

  printf("Enter targeted location: ");
  for (i = 0; i < 4; i++) {
    int c = getchar();
    if (c == EOF || c == '\n') {
      if (i == 2 || i == 3) break;
      return -1;
    }
    if (i == 0) row = c - 65;
    if (i == 1) col = c - 49;
    if (i == 2) col += 9;
  }

  /* simple validation */
  if (col < 0 || col > 9 || row < 0 || row > 9) {
    return -1;
  }

  m->row = row;
  m->col = col;

  return 1;
}

int update_guess_board(char board[BOARD_LEN][BOARD_LEN], move_rc m, char status) {
  switch (status) {
    case -1:  /* error */
      return -1;
    case 0:  /* miss */
      board[m.row][m.col] = MISS_MARK;
      break;
    case 1:
      board[m.row][m.col] = HIT_MARK;
      break;
    default:
      return -1;
  }
  return 1;
}

int move(request *req, response *res, char guess_board[BOARD_LEN][BOARD_LEN]) {

  printf("Your Turn!\n");

  /* Get move input from user */
  if (get_move(&req->m) == -1) {
    printf("Invalid move. Try again.\n");
    return -1;
  }

  /* Check if they've already guessed this location */
  if (guess_board[req->m.row][req->m.col] != '~') {
    printf("Already fired at this location. Try again.\n");
    return -1;
  }
  
  /* Trasmit user's move to the server */
  if (send_req_to_server(req, res, MOVE) == -1) {
    printf("Move failed.\n");
    return -1;
  }

  if (update_guess_board(guess_board, req->m, res->status) == -1) {
    printf("Failed to update board.\n");
    return -1;
  }
    
  return 1;
}

void update_ships_board(response *res, char board[BOARD_LEN][BOARD_LEN]) {
  switch (board[res->row][res->col]) {
    case MISS_MARK:
    case HIT_MARK:
      break;
    case '~':
      board[res->row][res->col] = MISS_MARK;
      break;
    default:
      board[res->row][res->col] = HIT_MARK;
      printf("\n>>You've been HIT!<<\n\n");
      break;
  }
}

void game_loop(request *req, response *res, char ships_board[BOARD_LEN][BOARD_LEN]) {
  char guess_board[BOARD_LEN][BOARD_LEN];
  empty_board(guess_board);

  if (!res->my_turn) {
    print_board(guess_board);
    print_board(ships_board);
    printf("--------------------------------------------------\n\n");
  }

  res->ready = 0;
  while (1) {
    if (!res->my_turn) {
      printf("\nOpponent is firing a shot");
      poll_server(req, res);
      printf("\n");
      update_ships_board(res, ships_board);
      if (res->ready) {
        printf("GAME OVER! - YOU'VE LOST...\n");   
        break;
      }
    }

    print_board(guess_board);
    print_board(ships_board);

    printf("--------------------------------------------------\n\n");
    if (move(req, res, guess_board) == -1) continue;
    if (res->status == 1) printf("\tHIT!\n");
    if (res->status == 0) printf("\tMISS!\n");
    printf("--------------------------------------------------\n\n");
    
    if (res->ready) {
      printf("GAME OVER! - YOU'VE WON!\n"); 
      break;
    }
    // res: [HIT OR MISS OR ERROR]
    print_board(guess_board);
    print_board(ships_board);
    res->my_turn = 0;
  }
}

int main(int argc,char **argv) {
  int sockfd, uid, ready, player = 0;
  int wait = 0;
  char c;

  verbose = 0;
  while((c = getopt(argc, argv, "v12")) != -1){
    switch(c){
      case 'v':
        //print all debug info
        verbose = 1;
        break;
      case '1':
        vprintf("PLAYER ONE\n");
        player = 1;
        break;
      case '2':
        vprintf("PLAYER TWO\n");
        player = 2;
        break;
      default:
        printf("invalid option\n"); 
        exit(EXIT_SUCCESS);
    }
  }

  srand(time(NULL));
  request req = {.uid = rand(), .serv_ip = argv[optind]};
  response res = {.state = WAITING};

  /* Verify server is up and working */
  printf("Connecting..");
  do {
    printf(".");
    fflush(stdout);
    ready = send_req_to_server(&req, &res, JOIN); 
  } while (ready == -1);
  printf("\n");

  //vprintf("main:res.ready=%d - 1\n", res.ready);

  /* Poll until 2nd player joins */
  if (!res.ready) {
    printf("\nSearching for game..");
    poll_server(&req, &res);
  }
  printf("\n\nGame found!\n\n");

  /* Get ship input from user */
  ship ships[5];
  char board[BOARD_LEN][BOARD_LEN];
  res.state = SETUP;
  res.ready = 0;

  /* SET UP FIXTURES FOR TESTING */
  if (player) {
    switch (player) {
      case 1:
        setup_p1_fixtures(ships, board);
        break;
      case 2:
        setup_p2_fixtures(ships, board);
        wait = 3;
        break;
      default:
        break;
    }
  } else {
    setup_game(ships, board);  
  }

  printf("Validating ships");
  do {
    printf(".");
    fflush(stdout);
    sleep(wait);  /* testing */
    //vprintf("main:res.ready=%d - 1\n", res.ready);
    ready = send_ships_to_server(&req, &res, ships);
    //vprintf("main:res.ready=%d - 2\n", res.ready);
  } while (ready == -1);
  printf("..Success\n");
    
  // vprintf("main:res.ready=%d\n", res.ready);
  // vprintf("main:res.my_turn=%d\n", res.my_turn);

  if (!res.ready) {
    printf("Waiting for game to start..");
    poll_server(&req, &res);
  }
  printf("\n\nGAME START!\n\n");

  res.state = PLAY;
  game_loop(&req, &res, board);

  return EXIT_SUCCESS;
}
