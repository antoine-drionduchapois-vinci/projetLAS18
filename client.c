#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "messages.h"
#include "utils_v1.h"



int main(int argc, char const *argv[])
{
	char pseudo[MAX_CHAR];
	int sockfd;
	int ret;

	StructMessage msg;
	// char inputBuffer[3];

	printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
	printf("Pour participer entrez votre nom :\n");
	ret = sread(0, pseudo, MAX_CHAR);
	checkNeg(ret, "read client error");
	pseudo[ret - 1] = '\0';
	strcpy(msg.text, pseudo);
	msg.code = INSCRIPTION_REQUEST;

	sockfd = initSocketClient(SERVER_IP, SERVER_PORT);

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

	printf("%ld\n", sread(sockfd, &msg, sizeof(msg)));
}
