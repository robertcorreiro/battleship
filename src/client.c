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

int send_ships_to_server(char *serv_ip, int uid, ship ships[5]) {
  int i, *intptr, sockfd = get_socket(serv_ip);
  char rv;
  message msg;

  /* TODO: fix hardcoding */
  msg.len = 26; 
  msg.buf[0] = 1;  /* sets first char of buf (msg_type) to INIT */
  intptr = (int *)&msg.buf[1];  /* sets next 4 bytes to int uid */
  *intptr = uid;
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

  if (read(sockfd, &rv, 1) != 1) {
    perror("client: read failed in send_ships_to_server");
    return -1;
  }

  if (close(sockfd) == -1) {
    perror("client: failed to close socket in send_ships_to_server");
    return -1;
  }
  
  return (int) rv;
}

int send_req_to_server(message_type type, char *serv_ip, int uid) {
  int sockfd = get_socket(serv_ip), *intptr;
  char rv;
  message msg;

  /* TODO: fix hardcoding */
  switch (type) {
    case JOIN:
      vprintf("Sending a JOIN\n");
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
  *intptr = uid; 
  // intptr = (int*)&msg.buf[5];
  // *intptr = msg.len; 
  msg.buf[5] = 0;

  if (write(sockfd, msg.buf, msg.len) == -1) {
    perror("client: write failed in send_req_to_server");
    return -1;
  }

  //vprintf("After write()\n");

  /* TODO: fix hardcoding */
  if (read(sockfd, &rv, 1) != 1) {
    perror("client: failed to read join/poll reponse");
    return -1;
  }

  //vprintf("After read()\n");

  if (close(sockfd) == -1) {
    perror("client: failed to close join/poll socket");
    return -1;
  }

  //vprintf("After close()\n");

  return (int) rv;
}

int main(int argc,char **argv) {
  int sockfd, uid, ready, player;
  char *options = "v12", *serv_ip, c;
  verbose = 0;

  while((c = getopt(argc, argv, options)) != -1){
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
  uid = rand();
  serv_ip = argv[optind];

  /* Verify server is up and working */
  printf("Connecting to server");
  do {
    printf(".");
    ready = send_req_to_server(JOIN, serv_ip, uid); 
  } while (ready == -1);

  printf("\nWaiting to join game.");
  while (!ready) {
    printf(".");
    fflush(stdout);
    sleep(2);
    ready = send_req_to_server(POLL, serv_ip, uid);
  }
  printf("\nGame found!\n");

  /* Get ship input from user */
  // ship ships[5];
  // setup_game_ships(ships);  

  /* DEBUG */
  int wait;
  ship test_ships[5];
  switch (player) {
    case 1:
      setup_p1_ship_fixtures(test_ships);
      wait = 0;
      break;
    case 2:
      setup_p2_ship_fixtures(test_ships);
      wait = 3;
      break;
    default:
      break;
  }

  printf("Validating ships");
  do {
    printf(".");
    fflush(stdout);
    //ready = send_ships_to_server(serv_ip, uid, ships);
    sleep(wait);
    ready = send_ships_to_server(serv_ip, uid, test_ships);
  } while (ready == -1);
  printf("\n");
  
  printf("VALIDATED!\n");
  fflush(stdout);

  /* WAIT FOR GAME TO START OR PLAY IF P2 */

  return EXIT_SUCCESS;
}
