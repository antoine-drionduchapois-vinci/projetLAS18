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

// pipe message codes
#define PIPE_TILE 10
#define PIPE_PLAYED 11
#define PIPE_SCORE 12

typedef struct Player
{
	char pseudo[MAX_CHAR];
	int sockfd;
	int pipefd[2];
	int pid;
	int score;
} Player;

typedef struct
{
	int code;
	int value;
} PipeMessage;

StructMessage msg;
Player players[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;

void restartInscriptions(int sig)
{
	end_inscriptions = 1;
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

void run_child(void *arg, void *arg1)
{
	// Retrieve pipe and socket
	int *pipefd = (int *)arg;
	int player_sockfd = *(int *)arg1;
	PipeMessage childPipeMessage;
	StructMessage childSocMessage;
	read(pipefd[1], &childPipeMessage, sizeof(childPipeMessage));

	// Game loop
	while (childPipeMessage.code == PIPE_TILE)
	{
		// Setting up msg
		childSocMessage.code = TILE;
		childSocMessage.value = childPipeMessage.value;
		strcpy(childSocMessage.text, "");

		// Send msg with tile to client
		swrite(player_sockfd, &childSocMessage, sizeof(childSocMessage));

		// Wait for code = PLAYED from client (sockfd)
		sread(player_sockfd, &childSocMessage, sizeof(childSocMessage));

		// Send code = PLAYED to parent (pipefd)
		childPipeMessage.code = PIPE_PLAYED;
		swrite(pipefd[1], &childPipeMessage, sizeof(childPipeMessage));

		sread(pipefd[0], &childPipeMessage, sizeof(childPipeMessage));
	}

	// Read Score
	sread(player_sockfd, &childSocMessage, sizeof(childSocMessage));

	// Send Score to Parent
	childPipeMessage.code = PIPE_SCORE;
	childPipeMessage.value = childSocMessage.value;
	swrite(pipefd[1], &childPipeMessage, sizeof(childPipeMessage));

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

	// int ret;

	ssigaction(SIGALRM, restartInscriptions);

	ssigaction(SIGUSR1, stopServer);
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

		printf("%d\n", tiles[0]); // TODO: remove

		for (int i = 0; i < nbPLayers; i++)
		{
			spipe(players[i].pipefd);
			// Creating child process with it's pipe and socket
			players[i].pid = fork_and_run2(run_child, players[i].pipefd, &players[i].sockfd);

			if (players[i].pid < 0)
			{
				perror("Fork error");
				exit(EXIT_FAILURE);
			}
		}

		PipeMessage pipeMessage;
		pipeMessage.code = PIPE_TILE;
		pipeMessage.value = 1;

		for (int i = 0; i < nbPLayers; i++)
		{
			swrite(players[i].pipefd[1], &pipeMessage, sizeof(pipeMessage));
		}

		for (int i = 0; i < nbPLayers; i++)
		{
			sread(players[i].pipefd[0], &pipeMessage, sizeof(pipeMessage));
			printf("Joueur %d a joué\n", pipeMessage.code);
		}
	}
}