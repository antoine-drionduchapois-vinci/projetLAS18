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
  PIPE_END_GAME,
  PIPE_SCORE,
  SCORE,
  RANKING,
  PIPE_TILE,
  PIPE_PLAYED,
} Code;

typedef struct
{
  Code code;
  char text[MAX_CHAR];
  int value;
} StructMessage;
#endif
