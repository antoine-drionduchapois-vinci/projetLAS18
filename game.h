#ifndef _GAME_H_
#define _GAME_H

#define MAX_CHAR 256

#include "ipc.h"
#include <poll.h>

typedef struct Player
{
    char pseudo[MAX_CHAR];
    int sockfd;
    int parentToChild[2];
    int childToParent[2];
    int pid;
    int score;
} Player;

void sendTile(Player *players, int size, int tile);

void waitForPlayed(struct pollfd *fds, int size);

void endGame(Player *players, int size);

void waitForScore(Player *players, struct pollfd *fds, int size, PlayerIpc *playerIpcs);

#endif