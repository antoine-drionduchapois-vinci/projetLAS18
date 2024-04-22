#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "ipc.h"
#include "utils_v1.h"

//*****************************************************************************
// USAGE
//*****************************************************************************
void checkUsage (int argc, char *argv[]) {
  if (argc != 2 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-d") && strcmp(argv[1], "-s"))) {
    printf("Usage:\n");
    printf("       %s -c   to create IPCs\n", argv[0]);
    printf("       %s -d   to destroy IPCs\n", argv[0]);
    printf("       %s -s   to destroy semaphore only\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}

//******************************************************************************
// MAIN FUNCTION
//******************************************************************************

int main (int argc, char *argv[]) {
  checkUsage(argc,argv);
  
  if (!strcmp(argv[1],"-c")) {
    // IPC creation and initialization
    sshmget(RAKING_SHM_KEY,SHARED_MEMORY_SIZE, IPC_CREAT | IPC_EXCL | PERM);
    sem_create(RAKING_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | PERM, 1);   
    printf("IPCs created.\n");
  } else if (!strcmp(argv[1], "-d")) {
    // IPC destruction
    printf("Destroying IPCs...\n");
    int shm_id = sshmget(RAKING_SHM_KEY, SHARED_MEMORY_SIZE, 0);
    sshmdelete(shm_id);
  
    int sem_id = sem_get(RAKING_SEM_KEY, 1);
    sem_delete(sem_id);

    printf("IPCs freed.\n");
  } else if (!strcmp(argv[1], "-s")) {
    // Semaphore destruction
    printf("Destroying Semaphore...\n");
    int sem_id = sem_get(RAKING_SEM_KEY, 1);
    sem_delete(sem_id);

    printf("Semaphore freed.\n");
  }
  
  exit(EXIT_SUCCESS);
}
