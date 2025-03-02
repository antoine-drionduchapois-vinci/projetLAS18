#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "utils_v1.h"
#include "ipc.h"

void createIpc()
{
    // IPC creation and initialization
    sshmget(RAKING_SHM_KEY, SHARED_MEMORY_SIZE, IPC_CREAT | IPC_EXCL | PERM);
    sem_create(RAKING_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | PERM, 1);
}

void detachIpc()
{
    // IPC destruction
    int shm_id = sshmget(RAKING_SHM_KEY, SHARED_MEMORY_SIZE, 0);
    sshmdelete(shm_id);

    int sem_id = sem_get(RAKING_SEM_KEY, 1);
    sem_delete(sem_id);
}

PlayerIpc *getSharedMemory()
{
    int shid = sshmget(RAKING_SHM_KEY, SHARED_MEMORY_SIZE, 0);

    PlayerIpc *memory = (PlayerIpc *)sshmat(shid);

    return memory;
}
