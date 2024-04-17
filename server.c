#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "messages.h"
#include "utils_v1.h"
#include "network.h"

#define MAX_PLAYERS 2
#define TIME_INSCRIPTION 15
#define FILE_NAME "random"

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

void run_child(void *arg)
{
	int *pipefd = (int *)arg;

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

		// read "random" file
		int filefd = sopen(FILE_NAME, O_RDONLY);
		checkNeg(fd, "Error opening file");

		int randoms[20];

		// Create a pipe and a child process for each player
		int pipefd[nbPLayers][2];
		pid_t pid[nbPLayers];

		for (i = 0; i < nbPLayers; i++)
		{
			spipe(pipefd[i]);

			pid[i] = fork_and_run1(run_child, pipefd[i]);

			if (pid[i] < 0)
			{
				perror("Fork error");
				exit(EXIT_FAILURE);
			}
		}
	}
}
