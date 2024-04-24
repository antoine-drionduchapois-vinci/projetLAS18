#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils_v1.h"
#include "network.h"
#include "ipc.h"
#include "game.h"
#include "messages.h"

#define TIME_INSCRIPTION 15
#define FILE_NAME "random"
#define BUFFERSIZE 60

StructMessage msg;
Player players[MAX_PLAYERS];
PlayerIpc ranking[MAX_PLAYERS];
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

void run_child(void *arg, void *arg1, void *arg2)
{
	int *parentToChild = (int *)arg;
	int *childToParent = (int *)arg1;
	sclose(parentToChild[1]);
	sclose(childToParent[0]);
	int player_sockfd = *(int *)arg2;

	StructMessage childPipeMessage;
	StructMessage socketMessage;

	sread(parentToChild[0], &childPipeMessage, sizeof(childPipeMessage));

	while (childPipeMessage.code == PIPE_TILE)
	{
		printf("child %d got tile : %d\n", getpid(), childPipeMessage.value);

		// Setup socket message
		socketMessage.code = TILE;
		socketMessage.value = childPipeMessage.value;

		// Send tile to client
		swrite(player_sockfd, &socketMessage, sizeof(socketMessage));

		// Wait for client response
		sread(player_sockfd, &socketMessage, sizeof(socketMessage));

		// Setup pipe message
		childPipeMessage.code = PIPE_PLAYED;

		// Send pipe message to parent
		swrite(childToParent[1], &childPipeMessage, sizeof(childPipeMessage));

		// Wait for parent message
		sread(parentToChild[0], &childPipeMessage, sizeof(childPipeMessage));
	}

	printf("child exit");

	exit(EXIT_SUCCESS);
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

	sockfd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

	createIpc();

	while (true)
	{
		int sid = sem_get(RAKING_SEM_KEY, 1);
  		sem_down0(sid);
		PlayerIpc* memory = getSharedMemory();
		// Initialisation de la mémoire partagée avec un tableau vide
		for (int i = 0; i < MAX_PLAYERS; i++) {
			strcpy(memory[i].pseudo, "");
			memory[i].score = 0;
		}


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

		printf("%d\n", tiles[0]);

		for (int i = 0; i < nbPLayers; i++)
		{
			spipe(players[i].parentToChild);
			spipe(players[i].childToParent);
			// Creating child process with it's pipe and socket
			players[i].pid = fork_and_run3(run_child, players[i].parentToChild, players[i].childToParent, &players[i].sockfd);

			if (players[i].pid < 0)
			{
				perror("Fork error");
				exit(EXIT_FAILURE);
			}

			sclose(players[i].parentToChild[0]);
			sclose(players[i].childToParent[1]);
		}

		for (int i = 0; i < 20; i++)
		{
			sendTile(players, nbPLayers, tiles[i]);
			waitForPlayed(players, nbPLayers);
		}
		endGame(players,nbPLayers);
		waitForScore(players, nbPLayers,ranking);

		


		swait(0);
	}
}