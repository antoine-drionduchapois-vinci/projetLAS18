#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "messages.h"
#include "utils_v1.h"

#define MAX_PLAYERS 2
#define BACKLOG 5
#define TIME_INSCRIPTION 15

typedef struct Player
{
	char pseudo[MAX_CHAR];
	int sockfd;
	int score;
} Player;

Player players[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;



void restartInscriptions(int sig)
{
	end_inscriptions = 1;
}

int main(int argc, char const *argv[])
{
	int sockfd, newsockfd, i;
	StructMessage msg;
	// int ret;

	ssigaction(SIGALRM, restartInscriptions);

	sockfd = initSocketServer(SERVER_PORT);
	printf("Le serveur tourne sur le port : %i \n", SERVER_PORT);

	while (true)
	{
		end_inscriptions = 0;
		alarm(TIME_INSCRIPTION);
		printf("Début des inscriptions\n");

		i = 0;
		int nbPLayers = 0;

		while (!end_inscriptions)
		{
			newsockfd = accept(sockfd, NULL, NULL);

			if (newsockfd > 0)
			{

				/*ret = */ sread(newsockfd, &msg, sizeof(msg));

				if (msg.code == INSCRIPTION_REQUEST)
				{
					printf("Inscription demandée par le joueur : %s\n", msg.text);

					strcpy(players[i].pseudo, msg.text);
					players[i].sockfd = newsockfd;
					i++;

					msg.code = INSCRIPTION_OK;
					nbPLayers++;
					/*ret = */ swrite(newsockfd, &msg, sizeof(msg));
					printf("Nb Inscriptions : %i\n", nbPLayers);
				}
			}
		}
		if (nbPLayers < 2)
			printf("Temps d'inscription écoulé, redémarrage du serveur.\n");
	}
}
