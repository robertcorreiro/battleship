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
  
  if ((rv = getaddrinfo("127.0.0.1", SERVER_PORT, &hints, &results)) != 0) {
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

  vprintf("before read: res->ready=%d\n", res->ready);

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
    case JOIN:
      //vprintf("Sending a JOIN\n");
      msg.buf[0] = 0;
      break;
    case POLL:
      //vprintf("Sending a POLL\n");
      msg.buf[0] = 3;
      break;
    default:
      break;
  }

  /* TODO: fix hardcoding */
  msg.len = 6;  
  intptr = (int*)&msg.buf[1];
  *intptr = req->uid; 
  // intptr = (int*)&msg.buf[5];
  // *intptr = msg.len; 
  msg.buf[5] = 0;

  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in send_req_to_server");
    return -1;
  }

  //vprintf("After write()\n");

  /* TODO: fix hardcoding */

  /* EXPAND THIS FOR DIFFERENT STATES */
  if (read(sockfd, &(res->ready), 1) != 1) {
    perror("client: failed to read 1 join/poll reponse");
    return -1;
  }

  if (res->state == SETUP || res->state == PLAY) {
    if (read(sockfd, &(res->my_turn), 1) != 1) {
      perror("client: failed to read 2 join/poll reponse");
      return -1;
    }    
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
    while (!res->my_turn) {
      printf(".");
      fflush(stdout);
      sleep(2);
      send_req_to_server(req, res, POLL);
      vprintf("\nPLAY:\n");
      vprintf("poll_server:res->ready=%d\n", res->ready);
      vprintf("poll_server:res->my_turn=%d\n", res->my_turn);
    }
  }
}

int get_move(move_rc *m) {
  int i, row, col;

  printf("Enter targeted location: ");
  for (i = 0; i < 3; i++) {
    int c = getchar();
    if (c == EOF) {
      if (i == 2) break;
      return -1;
    }
    if (i == 0) row = c - 65;
    if (i == 1) col = c - 49;
  }

  /* simple validation */
  if (col < 0 || col > 9 || row < 0 || row > 9) {
    return -1;
  }

  m->row = row;
  m->col = col;

  return 1;
}

int send_move_to_server(request *req, response *res, move_rc *m) {
  /* TODO: don't open new socket for each call to this function */
  int i, *intptr, sockfd = get_socket(req->serv_ip);
  message msg;

  /* TODO: fix hardcoding */
  msg.len = 8; 
  msg.buf[0] = 2;  /* sets first char of buf (msg_type) to MOVE */
  intptr = (int *)&msg.buf[1];  /* sets next 4 bytes to int uid */
  *intptr = req->uid;
  msg.buf[5] = 2;  /* payload length = 2 bytes */

  /* Because server parses using column-major... (oddly) */
  msg.buf[6] = m->col;
  msg.buf[7] = m->row;

  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in send_move_to_server");
    return -1;
  }

  if (read(sockfd, &(res->status), 1) != 1) {
    perror("client: read failed in send_move_to_server");
    return -1;
  }

  if (close(sockfd) == -1) {
    perror("client: failed to close socket in send_move_to_server");
    return -1;
  }
  
  return res->status;
}

int move(request *req, response *res, char guess_board[BOARD_LEN][BOARD_LEN]) {
  move_rc m;

  /* Get move input from user */
  if (get_move(&m) == -1) {
    printf("Invalid move. Try again.\n");
    return -1;
  }

  /* Check if they've already guessed this location */
  if (guess_board[m.row][m.col] != 'X') {
    printf("Already fired at this location. Try again.\n");
    return -1;
  }
  
  /* Trasmit user's move to the server */
  if (send_move_to_server(req, res, &m) == -1) {
    printf("Move failed.\n");
    return -1;
  }

  return 1;
}

void game_loop(request *req, response *res, char ships_board[BOARD_LEN][BOARD_LEN]) {
  char guess_board[BOARD_LEN][BOARD_LEN];
  empty_board(guess_board);

  while (1) {
    if (!res->my_turn) {
      printf("Waiting for other player");
      poll_server(req, res);
      printf("\n");
      // update_ships_board?
    }

    print_board(guess_board);
    print_board(ships_board);
    
    printf("YOUR TURN!\n");
    if (move(req, res, guess_board) == -1) continue;
    //update_guess_board(res->status, guess_board);
    res->my_turn = 0;
  }
}

int main(int argc,char **argv) {
  int sockfd, uid, ready, player = 0;
  int wait = 0;
  char c;

  srand(time(NULL));
  request req = {.uid = rand(), .serv_ip = argv[optind]};
  response res = {.state = WAITING};

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

  /* Verify server is up and working */
  printf("Connecting to server");
  do {
    printf(".");
    fflush(stdout);
    ready = send_req_to_server(&req, &res, JOIN); 
  } while (ready == -1);
  printf("\n");

  //vprintf("main:res.ready=%d - 1\n", res.ready);

  /* Poll until 2nd player joins */
  if (!res.ready) {
    printf("Waiting to join game.");
    poll_server(&req, &res);
  }
  printf("\nGame found!\n");

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

  printf("%d - Validating ships", (int) time(NULL));
  do {
    printf(".");
    fflush(stdout);
    sleep(wait);  /* testing */
    //vprintf("main:res.ready=%d - 1\n", res.ready);
    ready = send_ships_to_server(&req, &res, ships);
    //vprintf("main:res.ready=%d - 2\n", res.ready);
  } while (ready == -1);
  printf("..Success - %d\n", (int) time(NULL));
    
  vprintf("main:res.ready=%d\n", res.ready);
  vprintf("main:res.my_turn=%d\n", res.my_turn);

  if (!res.ready) {
    printf("Waiting for game to start");
    poll_server(&req, &res);
  }
  printf("\nGAME START!\n");

  //game_loop(&req, &res, board);
  res.state = PLAY;
  game_loop(&req, &res, board);

  return EXIT_SUCCESS;
}
