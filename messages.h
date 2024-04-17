#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define MAX_CHAR 256

typedef enum
{
  INSCRIPTION_REQUEST,
  INSCRIPTION_OK,
  TILE,
  PLAYED,
  CANCEL_GAME,
  END_GAME,
  SCORE,
  RANKING
} Code;

typedef struct
{
  Code code;
  char text[MAX_CHAR];
  int value;
} StructMessage;
#endif
