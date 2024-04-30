#ifndef _IPC_H_
#define _IPC_H_

#define MAX_PLAYERS 5

#define PERM 0666
#define MAX 256

#define RAKING_SHM_KEY 1
#define RAKING_SEM_KEY 2

#define SHARED_MEMORY_SIZE (MAX_PLAYERS * sizeof(PlayerIpc))

typedef struct PlayerIpc
{
	char pseudo[MAX];
	int score;
} PlayerIpc;

void createIpc();

void detachIpc();

PlayerIpc *getSharedMemory();

#endif