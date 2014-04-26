/*
  
  server.c
  ========

  Author: Robert Correiro

  Web server for Battleship protocol implementation.

*/

#include "server.h"  /* setup_server() */
#include "request.h"  /* request_handler() */

#include <stdio.h>  /* fprintf() */
#include <stdlib.h>  /* exit() */
#include <string.h>  /* memset() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>  /* sigaction() */

#define PORT "8080"
#define BACKLOG 5

int verbose;

// returns ptr to in_addr (IPv4) or in6_addr (IPv6) struct for inet_ntop()
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &( ((struct sockaddr_in *)sa) -> sin_addr );
  }
  return &( ((struct sockaddr_in6 *)sa) -> sin6_addr );
}

int main (int argc, char **argv) {
  int listen_fd, new_fd, status, reuse_val = 1;
  char in_addr[INET6_ADDRSTRLEN], *options = "v";
  struct addrinfo hints, *results, *p;
  struct sockaddr_storage cli_addr;
  socklen_t cli_addr_sz = sizeof cli_addr;

  char c;
  verbose = 0;
  while ((c = getopt(argc, argv, options)) != -1) {
    switch(c) {
      case 'v':
        verbose = 1;
        break;
      default:
        break;
    }
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  /* Create linked list results struct */
  if ((status = getaddrinfo(NULL, PORT, &hints, &results)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  /* Create socket */
  for (p = results; p != NULL; p = p->ai_next) {
    if ((listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    /* Prevent "Address already in use" error */
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, 
            &reuse_val, sizeof(int)) == -1) {
      perror("server: setsockopt");
      exit(1);
    }

    /* Assign socket address to socket */
    if (bind(listen_fd, results->ai_addr, results->ai_addrlen) == -1) {
      close(listen_fd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(results);

  /* Listen for connections */
  if (listen(listen_fd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  printf("Listening for connections...\n");

  /* Set initial game state */
  battleship game = {.state = WAITING};

  /* Main accept() loop */
  while (1) {
    /* Wait for connections */
    new_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_sz);
    if (new_fd == -1) {
      perror("server: accept");
      continue;
    }

    /* Convert connection's IP to printable form */
    inet_ntop(cli_addr.ss_family, get_in_addr((struct sockaddr *)&cli_addr),
              in_addr, sizeof in_addr);
    printf("Got connection from %s\n", in_addr);

    /* TODO: Check return val for error code */
    request_handler(new_fd, &game);
  }
  exit(EXIT_SUCCESS);
}