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

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
          s, sizeof s);
  printf("client: connecting to %s\n", s);
  freeaddrinfo(results);

  return sockfd;
}

void print_ships_board(char board[BOARD_LEN][BOARD_LEN]) {
  int i, j;
  char row_header = 'A';

  printf("   1  2  3  4  5  6  7  8  9  10\n");
  for(i = 0; i < BOARD_LEN; i++) {
    for(j = 0; j < BOARD_LEN + 1; j++) {
      if (j == 0) {  /* 0th column needs headings */
        printf("%c  ", row_header++);
      } else if (j == BOARD_LEN) {
        printf("%c\n", board[i][j-1]);
      } else {
        printf("%c  ", board[i][j-1]);
      }
    }
  }
  printf("\n");
}


int join_game_queue(char *serv_ip, int uid) {
  message msg;
  int sockfd = get_socket(serv_ip);
  int *intptr;
  msg.buf[0] = 0;
  intptr = (int*)&msg.buf[1];
  *intptr = uid; 
  msg.len = 5;  /* TODO: fix hardcoding */

  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in join_game_queue");
    return -1;
  }
  
  if (close(sockfd) == -1) {
    perror("client: failed to close sockfd");
    return -1;
  }
}

void add_ship_to_board(ship *s, char board[BOARD_LEN][BOARD_LEN]) {
  int i, row = s->row, col = s->col;

  if (s->ori > 0) {  /* +positive => horizontal */
    for (i = 0; i < s->len; i++) {
      board[s->row][col++] = '0' + s->len;
    }
  } else {  /* -negative => vertical */
    for (i = 0; i< s->len; i++) {
      board[row++][s->col] = '0' + s->len;
    }
  }
}

int set_ship_position(ship *s) {
  int i, row, col;

  printf("Enter initial position: ");
  for (i = 0; i < 3; i++) {
    int c = getchar();
    if (c == EOF) {
      if (i == 2) break;
      return -1;
    }
    if (i == 0) row = c - 65;
    if (i == 1) col = c - 49;
  }

  /* validation */
  if (row < 0 || row > 9 || col < 0 || col > 9) {
    return -1;
  }

  s->row = row;
  s->col = col;

  return 1;
}

int set_ship_orientation(ship *s) {
  int i, ori;

  printf("Enter Orientation (H/V): ");
  int c = getchar();
  getchar(); /* hack to get rid of EOF */
  
  switch(c) {
    case 'h':
    case 'H':
      s->ori = 1;
      break;
    case 'v':
    case 'V':
      s->ori = 0;
      break;
    default:
      return -1;
  }

  return 1;
}

int validate_ship_placement(ship *s, char board[BOARD_LEN][BOARD_LEN]) {
  int i;

  // vprintf("row=%d\ncol=%d\nori=%d\nlen=%d\n\n",s->row,s->col,s->ori,s->len);

  /* Horizontal validations */
  if (s->ori) {  
    /* Check running over the edge */
    if (s->col + (s->len) - 1 > 9) return -1;

    /* Horizontal collision? */
    int col = s->col;
    for (i = 0; i < s->len; i++, col++) {
      /* don't allow if something's already in the spot */
      /* TODO: change to WATER instead of 0 */
      if (board[s->row][col] != '0') return -1;
    }
  } 
  /* Vertical validations */
  else {  
    /* Check running over the edge */
    if (s->row + (s->len) - 1 > 9) return -1;

    /* Vertical collision? */
    int row = s->row;
    for (i = 0; i < s->len; i++, row++) {
      /* TODO: change to WATER instead of 0 */
      if (board[row][s->col] != '0') return -1;
    }
  }
  return 1;
}

int send_ships_to_server(char *serv_ip, int uid, ship ships[5]) {
  int i, sockfd;
  message msg;
  int *intptr;

  msg.buf[0] = 1;
  intptr = (int *)&msg.buf[1];
  *intptr = uid;
  msg.len = 25; /* TODO: fix hardcoding */

  /* TODO: fix hardcoding */
  for (i = 0; i < 5; i++) {
    msg.buf[5 + i*4] = (char) ships[i].sid;
    msg.buf[6 + i*4] = (char) ships[i].ori;
    msg.buf[7 + i*4] = (char) ships[i].col;
    msg.buf[8 + i*4] = (char) ships[i].row;
  }

  sockfd = get_socket(serv_ip);
  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in join_game_queue");
    return -1;
  }
  
  if (close(sockfd) == -1) {
    perror("client: failed to close sockfd");
    return -1;
  }
  return 1;
}

int setup_game(char *serv_ip, int uid) {
  int row, col, rv, i;
  char board[BOARD_LEN][BOARD_LEN];
  ship ships[5], *s;

  /* Initialize ships board to empty */
  for (row = 0; row < BOARD_LEN; row++) {
    for (col = 0; col < BOARD_LEN; col++) {
      board[row][col] = '0';
    }
  }

  print_ships_board(board);

  for (i = 0; i < 5; i++) {
    s = &ships[i];
    s->sid = i;
    s->ori = 1;
    s->len = ship_lens[i];

    do {
      printf("SETUP: Ship %d - Length %d\n", i+1, s->len);
      
      rv = set_ship_position(s);
      while (rv == -1) {
        printf("Invalid position. Please try again.\n");
        rv = set_ship_position(s);
      }

      rv = set_ship_orientation(s);
      while (rv == -1) {
        printf("Invalid orientation. Please try again\n");
        rv = set_ship_orientation(s);
      }

      rv = validate_ship_placement(s, board);
      if (rv == -1) {
        print_ships_board(board);
        printf("Invalid placement. Please try again.\n");
      }
    } while (rv == -1);

    add_ship_to_board(s, board);
    print_ships_board(board);
  }

  /* TODO: check return val */
  send_ships_to_server(serv_ip, uid, ships);

}

int main(int argc,char **argv) {
  int sockfd, uid;
  char *options = "v", *serv_ip, c;
  verbose = 0;

  while((c = getopt(argc, argv, options)) != -1){
    switch(c){
      case 'v':
        //print all debug info
        verbose = 1;
        break;
      default:
        printf("invalid option\n"); 
        exit(EXIT_SUCCESS);
    }
  }

  serv_ip = argv[optind];
  srand(time(NULL));
  uid = rand();

  /* Verify server is up and working */
  if (join_game_queue(serv_ip, uid) == -1) {
    vprintf("Error: Failed to join game queue");
    exit(EXIT_FAILURE);
  }

  /* Get ship input from user */
  setup_game(serv_ip, uid);

  /* WAIT FOR GAME TO START OR PLAY IF P2 */

  return EXIT_SUCCESS;
}
