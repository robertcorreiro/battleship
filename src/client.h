/*
  
  client.h
  ========

  Author: Robert Correiro

  Client structs and helpers.

*/

int ship_lens[] = {2, 3, 3, 4, 5};

typedef struct {
  int sid;
  int ori;
  int col;
  int row;
  int len;
} ship;