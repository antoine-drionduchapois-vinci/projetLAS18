#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "messages.h"
#include "utils_v1.h"
#include "network.h"
#include "ipc.h"

int scoreValues[] = {0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300};

char pseudo[MAX_CHAR];
int sockfd;
StructMessage msg;
int stream[20] = {0};

void endProgram(int code)
{
	printf("Fermeture du socket...");
	sclose(sockfd);
	printf("Arrêt du jeu...");
	exit(code);
}

void placeTile(int tile, int index)
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

void printStream(int remaining)
{
	printf("Tuiles restantes : %d\n", remaining);
	printf("*********************************************************************************\n|");
	for (int i = 0; i < 20; i++)
	{
		printf("%3d|", i + 1);
	}
	printf("\n|");
	for (int i = 0; i < 20; i++)
	{
		printf("%3d|", stream[i]);
	}
	printf("\n*********************************************************************************\n");
}

int calculateScore(int arr[], int size)
{
	int totalScore = 0;
	int currentStreak = 1;
	for (int i = 1; i < size; i++)
	{
		if (arr[i] >= arr[i - 1])
		{
			currentStreak++;
		}
		else
		{
			totalScore += scoreValues[currentStreak - 1];
			currentStreak = 1;
		}
	}

	totalScore += scoreValues[currentStreak - 1];

	return totalScore;
}

int main(int argc, char const *argv[])
{
	// Get port in program params
	if (argc < 2)
	{
		printf("Veuillez préciser le port en paramètres.\n");
		exit(1);
	}
	int port = atoi(argv[1]);

	// Print welcome message
	printf("   _____ _                                \n"
		   "  / ____| |                               \n"
		   " | (___ | |_ _ __ ___  __ _ _ __ ___  ___ \n"
		   "  ___ | __| '__/ _  |/ _` | '_ ` _  |/ __|\n"
		   " ____) | |_| | |  __/ (_| | | | | |  __ \n"
		   " |_____/ |__|_||____|__,_|__| |_| |_|___/\n");
	printf("Bienvenue dans le programe d'inscription au serveur de jeu.\n");

	// Read player pseudo
	printf("Pour participer entrez votre nom :\n");
	int lenght = sread(0, pseudo, MAX_CHAR);
	pseudo[lenght - 1] = '\0';
	strcpy(msg.text, pseudo);

	// Init client socket
	sockfd = initSocketClient(SERVER_IP, port);

	// Send inscription request
	msg.code = INSCRIPTION_REQUEST;
	swrite(sockfd, &msg, sizeof(msg));

	// Wait for server response
	sread(sockfd, &msg, sizeof(msg));
	if (msg.code == INSCRIPTION_OK)
		printf("Inscription acceptée !\n");
	else
	{
		printf("Inscription refusée !\n");
		endProgram(EXIT_SUCCESS);
	}

	// Wait for server tile
	sread(sockfd, &msg, sizeof(msg));

	// Game loop
	int turn = 0;
	printStream(20 - turn);
	while (msg.code == TILE)
	{
		turn++;
		printf("Placer la tuile '%d' : ", msg.value);

		// Place tile
		placeTile(msg.value, atoi(readLine()) - 1);
		printStream(20 - turn);

		// Send "Played" to server
		msg.code = PLAYED;
		swrite(sockfd, &msg, sizeof(msg));

		// Wait for new tile from server
		printf("Attente des autres joueurs...\n");
		sread(sockfd, &msg, sizeof(msg));
	}

	// Inscriptions timed out
	if (msg.code == CANCEL_GAME)
	{
		printf("Partie annulée par le serveur!\n");
		sclose(sockfd);
		endProgram(EXIT_SUCCESS);
	}

	// End of the game
	if (msg.code == END_GAME)
	{
		// Send score to server
		msg.code = SCORE;
		msg.value = calculateScore(stream, 20);
		swrite(sockfd, &msg, sizeof(msg));

		// Wait for server to send ranking
		PlayerIpc ranking[MAX_PLAYERS];
		sread(sockfd, &ranking, MAX_PLAYERS * sizeof(PlayerIpc));

		// Print ranking
		printf("*********************************************************************************\n");
		printf("           \n"
			   "      _    __      __  __           __            __      _______      ______\n"
			   "||   / |  / /       / /      /|    / /     /|    / /     /______ /    //   ) )\n"
			   "||  /  | / /       / /      //|   / /     //|   / /     //_____      //___/ /\n"
			   "|| / /||/ /       / /      // |  / /     // |  / /     / _____ /    / ___ (\n"
			   "||/ / |  /       / /      //  | / /     //  | / /     //_____      //   | |\n"
			   "|  /  | /     __/ /__    //   |/ /     //   |/ /     /______ /    //    | |\n");

		printf("\t\t\t\t%14s\n\n", ranking[0].pseudo);
		printf("*********************************************************************************\n");

		printf("Classement :\n");
		printf("--------------------------------\n");
		printf("| pos |     Joueur     | score |\n");
		printf("|-----|----------------|-------|\n");
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			printf("| %3d | %14s |  %3d  |\n", i, ranking[i].pseudo, ranking[i].score);
		}
		printf("--------------------------------\n");
	}

	// End program
	endProgram(EXIT_SUCCESS);
}
