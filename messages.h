#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define SERVER_PORT 9502
#define SERVER_IP "127.0.0.1"
#define MAX_CHAR 256

#define INSCRIPTION_REQUEST 10
#define INSCRIPTION_OK 11
#define INSCRIPTION_KO 12
#define START_GAME 13
#define CANCEL_GAME 14
#define SCORE 15
#define RANKING 16
#define TILE 17
#define END_GAME 18

typedef struct
{
  char messageText[MAX_CHAR];
  int value;
  int code;
} StructMessage;
#endif
