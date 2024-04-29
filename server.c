#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "utils_v1.h"
#include "network.h"
#include "ipc.h"
#include "game.h"
#include "messages.h"

#define TIME_INSCRIPTION 15
#define BUFFERSIZE 60

StructMessage msg;
Player players[MAX_PLAYERS];
PlayerIpc ranking[MAX_PLAYERS];
int sockfd;
volatile sig_atomic_t end_inscriptions = 0;
char fileName[256];
bool end = false;
int tiles[20];
int nbPLayers = 0;

bool inscriptions(int sockfd)
{
	end_inscriptions = 0;
	alarm(TIME_INSCRIPTION);

	int newsockfd;
	int i = 0;

	while (!end_inscriptions)
	{
		newsockfd = accept(sockfd, NULL, NULL);

		if (newsockfd > 0)
		{
			sread(newsockfd, &msg, sizeof(msg));

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
				swrite(newsockfd, &msg, sizeof(msg));
				printf("Inscription acceptée.(nb. inscriptions : %i)\n", nbPLayers);
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
		return false;
	}
	return true;
}

void restartInscriptions(int sig)
{
	end_inscriptions = 1;
}

void run_child(void *arg, void *arg1, void *arg2)
{
	// Get pipes
	int *parentToChild = (int *)arg;
	int *childToParent = (int *)arg1;
	sclose(parentToChild[1]);
	sclose(childToParent[0]);

	// Get shared memory
	int sid = sem_get(RAKING_SEM_KEY, 1);
	int playerSockfd = *(int *)arg2;

	StructMessage childPipeMessage;
	StructMessage socketMessage;

	// Child game loop
	sread(parentToChild[0], &childPipeMessage, sizeof(childPipeMessage));
	while (childPipeMessage.code == PIPE_TILE)
	{
		// Setup socket message
		socketMessage.code = TILE;
		socketMessage.value = childPipeMessage.value;

		// Send tile to client
		swrite(playerSockfd, &socketMessage, sizeof(socketMessage));

		// Wait for client response
		sread(playerSockfd, &socketMessage, sizeof(socketMessage));

		// Setup pipe message
		childPipeMessage.code = PIPE_PLAYED;

		// Send pipe message to parent
		swrite(childToParent[1], &childPipeMessage, sizeof(childPipeMessage));

		// Wait for parent message
		sread(parentToChild[0], &childPipeMessage, sizeof(childPipeMessage));
	}

	// Send end game to client
	socketMessage.code = END_GAME;
	swrite(playerSockfd, &socketMessage, sizeof(socketMessage));

	// Get client score
	sread(playerSockfd, &socketMessage, sizeof(socketMessage));

	// Send score to parent
	childPipeMessage.code = PIPE_SCORE;
	childPipeMessage.value = socketMessage.value;
	swrite(childToParent[1], &childPipeMessage, sizeof(childPipeMessage));

	// Get shared memory
	sem_down0(sid);
	PlayerIpc *childRanking = getSharedMemory();

	// Write ranking in shared memory
	swrite(playerSockfd, childRanking, MAX_PLAYERS * sizeof(PlayerIpc));

	// Free access to shared memory
	sem_up0(sid);

	// Close socket
	sclose(playerSockfd);

	// Close pipes
	sclose(parentToChild[0]);
	sclose(childToParent[1]);

	exit(EXIT_SUCCESS);
}

void sortRanking(PlayerIpc *tableauPlayers, int sz)
{
	for (int i = 0; i < sz - 1; ++i)
	{
		for (int j = 0; j < sz - i - 1; ++j)
		{
			if (tableauPlayers[j + 1].score > tableauPlayers[j].score)
			{
				PlayerIpc temp = tableauPlayers[j];
				tableauPlayers[j] = tableauPlayers[j + 1];
				tableauPlayers[j + 1] = temp;
			}
		}
	}
}

void stopServer(int sig)
{

	printf(" : Partie en cours, arrêt à la fin de la partie.\n");
	end = true;
}

void killEverything()
{
	// Close listening socket
	printf("Fermeture du socket d'écoute...\n");
	sclose(sockfd);

	printf("Destruction IPC...\n");
	detachIpc();
}

int main(int argc, char const *argv[])
{
	// Get program params
	if (argc < 3)
	{
		printf("Veuillez préciser le port et le fichier tiles en paramètres.\n");
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
	strcpy(fileName, argv[2]);

	// Set interuption actions
	ssigaction(SIGALRM, restartInscriptions);
	ssigaction(SIGINT, stopServer);

	// Init IPC
	createIpc();

	// Init socket for listening
	sockfd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

	// Infinite server loop
	while (true)
	{
		// Check for kill server request
		if (end)
		{
			killEverything();
			printf("Arrêt du serveur...\n");
			exit(EXIT_SUCCESS);
		}

		// Begin inscription phase
		printf("Début des inscriptions :\n");
		if (!inscriptions(sockfd))
			continue;

		// Get and lock shared memory
		int sid = sem_get(RAKING_SEM_KEY, 1);
		sem_down0(sid);
		PlayerIpc *memory = getSharedMemory();

		// Clean shared memory
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			strcpy(memory[i].pseudo, "");
			memory[i].score = 0;
		}

		// Set tiles array from the file
		int filefd = sopen(fileName, O_RDONLY, 0666);

		char bufRd[BUFFERSIZE];
		sread(filefd, bufRd, BUFFERSIZE);
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

		// Setup pipes and fork for every players
		for (int i = 0; i < nbPLayers; i++)
		{
			spipe(players[i].parentToChild);
			spipe(players[i].childToParent);
			players[i].pid = fork_and_run3(run_child, players[i].parentToChild, players[i].childToParent, &players[i].sockfd);

			if (players[i].pid < 0)
			{
				perror("Fork error");
				exit(EXIT_FAILURE);
			}

			sclose(players[i].parentToChild[0]);
			sclose(players[i].childToParent[1]);
		}

		// Game loop
		printf("Début de la partie :\n");
		for (int i = 0; i < 20; i++)
		{
			sendTile(players, nbPLayers, tiles[i]);
			waitForPlayed(players, nbPLayers);
		}

		// End of the game
		endGame(players, nbPLayers);
		waitForScore(players, nbPLayers, ranking);

		// Set ranking in IPC
		sortRanking(ranking, nbPLayers);
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			strcpy(memory[i].pseudo, ranking[i].pseudo);
			memory[i].score = ranking[i].score;
		}

		// Free shared memory
		sem_up0(sid);

		// Close pipes
		for (int i = 0; i < nbPLayers; i++)
		{
			sclose(players[i].parentToChild[1]);
			sclose(players[i].childToParent[0]);
		}

		// Wait for children to end their task
		swait(0);

		// Reset players table
		nbPLayers = 0;
	}
}