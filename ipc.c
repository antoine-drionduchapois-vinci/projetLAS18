#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "utils_v1.h"
#include "ipc.h"

void createIpc() {
    // Mémoire Partagée
    // IPC creation and initialization
    sshmget(RAKING_SHM_KEY, MAX_PLAYERS * sizeof(int), IPC_CREAT | IPC_EXCL | PERM);
    sem_create(RAKING_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | PERM, 1);   
    printf("IPCs created.\n");
}

void detachIpc() {
    // IPC destruction
    printf("Destroying IPCs...\n");
    int shm_id = sshmget(RAKING_SHM_KEY, MAX_PLAYERS * sizeof(int), 0);
    sshmdelete(shm_id);
  
    int sem_id = sem_get(RAKING_SEM_KEY, 1);
    sem_delete(sem_id);

    printf("IPCs freed.\n");
}

int* getSharedMemory() {
    int shid = sshmget(RAKING_SHM_KEY, MAX_PLAYERS * sizeof(int), 0);
  
    int* memory = sshmat(shid);
   
    return memory;
}
