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



void endServerHandler(int sig)
{
	end_inscriptions = 1;
}

void run_child(void *arg)
{
    int *pipefd = (int *)arg;
    // Close the write end of the pipe in the child process
    sclose(pipefd[1]);

    // Process client communication and handle player actions
    // Use pipefd[0] for communication with the parent process
}

int main(int argc, char const *argv[])
{
	int sockfd, newsockfd, i;
	StructMessage msg;
	// int ret;

	ssigaction(SIGALRM, endServerHandler);

	sockfd = initSocketServer(SERVER_PORT);
	printf("Le serveur tourne sur le port : %i \n", SERVER_PORT);

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
				printf("Inscription demandée par le joueur : %s\n", msg.messageText);

				strcpy(players[i].pseudo, msg.messageText);
				players[i].sockfd = newsockfd;
				i++;

				if (nbPLayers < MAX_PLAYERS)
				{
					msg.code = INSCRIPTION_OK;
					nbPLayers++;
					if (nbPLayers == MAX_PLAYERS)
					{
						alarm(0);
						end_inscriptions = 1;
					}
				}
				else
				{
					msg.code = INSCRIPTION_KO;
				}
				/*ret = */ swrite(newsockfd, &msg, sizeof(msg));
				printf("Nombre d' Inscriptions : %i\n", nbPLayers);
			}
		}
	}
	// Create a pipe and a child process for each player
    int pipefd[MAX_PLAYERS][2];
    pid_t pid[MAX_PLAYERS];

    for (i = 0; i < MAX_PLAYERS; i++)
    {
        spipe(pipefd[i]);

        pid[i] = fork_and_run1(run_child, pipefd[i]);

        if (pid[i] < 0)
        {
            perror("Fork error");
            exit(EXIT_FAILURE);
        }

        // Close the read end of the pipe in the parent process
        sclose(pipefd[i][0]);
    }
}
