#include "game.h"
#include "utils_v1.h"
#include "messages.h"
#include "ipc.h"
#include <string.h>

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

void endGame(Player *players, int size)
{
    pipeMessage.code = END_GAME;
    

    for (int i = 0; i < size; i++)
    {
        swrite(players[i].pipefd[1], &pipeMessage, sizeof(pipeMessage));
    }
}

void waitForScore(Player *players, int size, PlayerIpc  *playerIpcs) {
    // Assurez-vous que size ne dépasse pas MAX_PLAYERS
    if (size > MAX_PLAYERS) {
        printf("Erreur : Le nombre de joueurs dépasse la limite maximale.\n");
       
    }

    for (int i = 0; i < size; i++) {
        StructMessage pipeMessage;
        sread(players[i].pipefd[0], &pipeMessage, sizeof(StructMessage));

        // Stocker les informations dans PlayerIpc
        strcpy(playerIpcs[i].pseudo, players[i].pseudo);
        playerIpcs[i].score = pipeMessage.value;

        printf("Le joueur %s a joué avec un score de %d.\n", playerIpcs[i].pseudo, playerIpcs[i].score);
    }
}



