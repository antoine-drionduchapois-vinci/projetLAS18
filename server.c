#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "messages.h"
#include "utils_v1.h"
#include "network.h"

#define MAX_PLAYERS 2
#define TIME_INSCRIPTION 15
#define FILE_NAME "random"
#define BUFFERSIZE 60

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

Player *trierTableau(Player *tableauPlayers, int sz)
{
	for (int i = 0; i < sz - 1; ++i)
	{
		for (int j = 0; j < sz - i - 1; ++j)
		{

			if (tableauPlayers[j + 1].score > tableauPlayers[j].score)
			{
				Player temp = tableauPlayers[j];
				tableauPlayers[j] = tableauPlayers[j + 1];
				tableauPlayers[j + 1] = temp;
			}
		}
	}
	return tableauPlayers;
}

// Signal to stop server
void stopServer(int sig)
{
	if (!end_inscriptions)
	{
		printf("Partie en cours, arrêt impossible.\n");
	}
	else
	{
		printf("Arrêt du serveur...\n");
		// Clean up resources (sockets, child processes, etc.)
		exit(0);
	}
}

void run_child(void *arg)
{
	// int *pipefd = (int *)arg;

	// Process client communication and handle player actions
	// Use pipefd[0] for communication with the parent process
}

int main(int argc, char const *argv[])
{
	if (argc < 2)
	{
		printf("Veuillez préciser le port en paramètres.\n");
		exit(1);
	}
	int port = atoi(argv[1]);
	int sockfd, newsockfd, i;
	StructMessage msg;
	// int ret;

	ssigaction(SIGALRM, restartInscriptions);

	ssigaction(SIGUSR1, stopServer);

	sockfd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

	while (true)
	{
		printf("Début des inscriptions :\n");
		end_inscriptions = 0;
		alarm(TIME_INSCRIPTION);

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
					if (nbPLayers == MAX_PLAYERS)
					{
						alarm(0);
						end_inscriptions = 1;
					}
					/*ret = */ swrite(newsockfd, &msg, sizeof(msg));
					printf("Nombre d' Inscriptions : %i\n", nbPLayers);
				}
			}
		}

		if (nbPLayers < 2)
		{
			msg.code = CANCEL_GAME;
			for (int i = 0; i < nbPLayers; i++)
			{
				swrite(players[i].sockfd, &msg, sizeof(msg));
				sclose(players[i].sockfd);
			}
			printf("Temps de connexion écoulé !\n");
			continue;
		}

		// read "random" file TODO: use utils
		int filefd = open(FILE_NAME, O_RDONLY);
		checkNeg(filefd, "Error opening file");

		char bufRd[BUFFERSIZE];
		sread(filefd, bufRd, BUFFERSIZE);

		int tiles[20];
		char *delim = "\n";
		char *token;
		int i = 0;

		token = strtok(bufRd, delim);
		while (token != NULL && i < 20)
		{
			tiles[i] = atoi(token);
			token = strtok(NULL, delim);
			i++;
		}

		// // Create a pipe and a child process for each player
		// int pipefd[MAX_PLAYERS][2];
		// pid_t pid[MAX_PLAYERS];

		// for (i = 0; i < MAX_PLAYERS; i++)
		// {
		// 	spipe(pipefd[i]);

		// 	pid[i] = fork_and_run1(run_child, pipefd[i]);

		// 	if (pid[i] < 0)
		// 	{
		// 		perror("Fork error");
		// 		exit(EXIT_FAILURE);
		// 	}
		// }
	}
}
