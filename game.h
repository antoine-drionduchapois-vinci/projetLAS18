#ifndef _GAME_H_
#define _GAME_H

#define MAX_CHAR 256

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

void waitForPlayed(Player *players, int size);

#endif