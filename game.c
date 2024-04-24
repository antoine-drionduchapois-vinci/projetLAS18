#include "game.h"
#include "utils_v1.h"
#include "messages.h"

StructMessage pipeMessage;
int turn = 0;

void sendTile(Player *players, int size, int tile)
{
    pipeMessage.code = PIPE_TILE;
    pipeMessage.value = tile;

    for (int i = 0; i < size; i++)
    {
        swrite(players[i].parentToChild[1], &pipeMessage, sizeof(pipeMessage));
    }
}

void waitForPlayed(Player *players, int size)
{
    for (int i = 0; i < size; i++)
    {
        sread(players[i].childToParent[0], &pipeMessage, sizeof(pipeMessage));
        printf("Joueur %s a joué\n", players[i].pseudo);
    }
}