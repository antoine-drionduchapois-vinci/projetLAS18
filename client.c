#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"
#include "utils_v1.h"
#include "network.h"

void placeTile(int stream[20], int tile, int index)
{
	int i = index;
	while (stream[i] != 0)
	{
		if (i == 19)
			i = 0;
		else
			i++;
	}
	stream[i] = tile;
}

void printStream(int stream[20])
{
	for (int i = 0; i < 20; i++)
	{
		printf("%3d|", i + 1);
	}
	printf("\n");
	for (int i = 0; i < 20; i++)
	{
		printf("%3d|", stream[i]);
	}
	printf("\n");
}

int main(int argc, char const *argv[])
{
	if (argc < 2)
	{
		printf("Veuillez préciser le port en paramètres.\n");
		exit(1);
	}
	int port = atoi(argv[1]);
	char pseudo[MAX_CHAR];
	int sockfd;
	int ret;

	StructMessage msg;

	printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
	printf("Pour participer entrez votre nom :\n");
	ret = sread(0, pseudo, MAX_CHAR);
	checkNeg(ret, "read client error");
	pseudo[ret - 1] = '\0';
	strcpy(msg.text, pseudo);
	msg.code = INSCRIPTION_REQUEST;

	sockfd = initSocketClient(SERVER_IP, port);

	swrite(sockfd, &msg, sizeof(msg));

	sread(sockfd, &msg, sizeof(msg));

	switch (msg.code)
	{
	case INSCRIPTION_OK:
		printf("Réponse du serveur : Inscription acceptée\n");
		break;
	default:
		printf("Réponse du serveur non prévue %d\n", msg.code);
		break;
	}

	// Client Game
	int stream[20] = {0};
	sread(sockfd, &msg, sizeof(msg));

	while (msg.code == TILE)
	{
		printf("Placer la tuile '%d' : ", msg.value);

		// Place tile
		placeTile(stream, msg.value, atoi(readLine()) - 1);
		printStream(stream);

		// Send "Played" to server
		msg.code = PLAYED;
		swrite(sockfd, &msg, sizeof(msg));

		// Wait for new tile
		sread(sockfd, &msg, sizeof(msg));
	}

	if (msg.code == CANCEL_GAME)
	{
		printf("Partie annulée par le serveur!\n");
		sclose(sockfd);
		exit(EXIT_SUCCESS); // TODO
	}

	// TODO: calculate score

	// Send Score to server
	// msg.code = SCORE;
	// msg.value = 0; // TODO: set score
	// strcpy(msg.text, "");
	// swrite(sockfd, &msg, sizeof(msg));

	// READ RANKING
	// sread(sockfd, &msg, sizeof(msg));
	// printf("Ranking : %d", msg.value);

	// Game finished
	// Close socketFD
	sclose(sockfd);
}
