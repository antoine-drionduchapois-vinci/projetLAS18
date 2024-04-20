#ifndef _IPC_H_
#define _IPC_H_

#define MAX_PLAYERS 2

#define PERM 0666

#define RAKING_SHM_KEY 50
#define RAKING_SEM_KEY 60

void createIpc();

void detachIpc();

int* getSharedMemory();

#endif