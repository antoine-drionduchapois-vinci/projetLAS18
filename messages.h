#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define SERVER_PORT 9502
#define SERVER_IP "127.0.0.1"
#define MAX_CHAR 256

// TODO: use enum
#define INSCRIPTION_REQUEST 10
#define INSCRIPTION_OK 11
#define TILE 12
#define PLAYED 13
#define CANCEL_GAME 14
#define END_GAME 15
#define SCORE 16
#define RANKING 17

typedef struct
{
  int code;
  char text[MAX_CHAR];
  int value;
} StructMessage;
#endif
