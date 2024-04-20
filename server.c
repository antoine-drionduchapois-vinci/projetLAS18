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

typedef struct Player
{
	char pseudo[MAX_CHAR];
	int sockfd;
	int score;
} Player;

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
    if (!end_inscriptions) {
        printf("Partie en cours, arrêt impossible.\n");
    } else {
        printf("Arrêt du serveur...\n");
        // Clean up resources (sockets, child processes, etc.)
        exit(0);
    }
}

void run_child(void *arg, int *arg1)
{
    // Retrieve pipe and socket
        int *pipefd = (int *)arg;
        int player_sockfd = *arg1;

        // Game loop
        while(true){
            // retrieve tile from parent (pipe)
            sread(pipefd[0], &msg, sizeof(msg));

            // stop loop if client sends END_GAME
            if (msg.code == END_GAME){
                break;
            }
            // Send msg with tile to client (socket)
            swrite(player_sockfd, &msg, sizeof(msg));

            // Wait for code = PLAYED from client (sockfd)
            sread(player_sockfd, &msg, sizeof(msg));

            // Send code = PLAYED to parent (pipefd)
            sread(pipefd[1], &msg, sizeof(msg));

            // Continue Loop as long as code != END_GAME
        }

        // Read Score
        sread(player_sockfd, &msg, sizeof(msg));

        // Send Score to Parent
        swrite(pipefd[1], &msg, sizeof(msg));

        // TODO : read ranking from shard memory

        // TODO : Setting up msg with ranking

        // Sends ranking to client
        swrite(player_sockfd, &msg, sizeof(msg));
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
		msg.code = CANCEL_GAME;
		for (int i = 0; i < nbPLayers; i++)
		{
			swrite(players[i].sockfd, &msg, sizeof(msg));
			sclose(players[i].sockfd);
		}
		printf("Temps de connexion écoulé !\n");
	}
	// Create a pipe and a child process for each player
	int pipefd[MAX_PLAYERS][2];
	pid_t pid[MAX_PLAYERS];

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		spipe(pipefd[i]);
        // Creating child process with it's pipe and socket
		pid[i] = fork_and_run2(run_child, pipefd[i], players[i].sockfd);

		if (pid[i] < 0)
		{
			perror("Fork error");
			exit(EXIT_FAILURE);
		}

		// Close the read end of the pipe in the parent process
		sclose(pipefd[i][0]);
	}
}
