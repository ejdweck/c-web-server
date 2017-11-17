#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include "stats.h"
#include <pthread.h>


stats_t* stats_init(key_t key) {
  sem_t *mutex;
  if ((mutex = sem_open("EDEB", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("sem_open");
    return NULL;
  }
  sem_wait(mutex);
  size_t page_size = getpagesize();
  int id = shmget(key, page_size, 0666);
  stats_t* p;
  p = (stats_t*) shmat(id, NULL, 0);

  if (p == (stats_t*)-1) {
    sem_unlink("EDEB");
    sem_close(mutex);
    return NULL;
    printf("ERROR WITH SHMAT");
  }

  int index = -1;
  int i;
  inuse_t *inuse_ptr;
  inuse_ptr = (inuse_t*)p;
  for (i = 0; i < 16; i++) {
    if (inuse_ptr->array[i] == 0) {
      inuse_ptr->array[i] = 1;
      index = i;
      break;
    }
  }
  sem_post(mutex);
  if (index == -1) {
    // sem_unlink("EDEB");
    sem_close(mutex);
    return NULL;
  }
  inuse_ptr += 1;
  stats_t *new_p;
  new_p = (stats_t*)inuse_ptr;
  new_p += index;

  // sem_unlink("EDEB");
  sem_close(mutex);
  return new_p;
}

int stats_unlink(key_t key) {
  sem_t *mutex;
  if ((mutex = sem_open("EDEB", 0644, 0)) == SEM_FAILED) {
    perror("sem_open");
    return -1;
  }

  sem_wait(mutex);
  size_t page_size = getpagesize();
  int id = shmget(key, page_size, 0666);

  stats_t *p = (stats_t*) shmat(id, NULL, 0);

  int *array_ptr;
  array_ptr = (int*)p;
  int pid = getpid();
  inuse_t *np;
  np = (inuse_t*)p;
  // np = (void*)p;
  np += 1;
  stats_t *after_arr = (stats_t*)np;
  // p = (stats_t*)p;
  int i;
  for (i = 0; i < 16; i++) {
    if (after_arr->pid == pid) {
      array_ptr[i] = 0;
      after_arr->pid = -1;
      after_arr->counter = -1;
      after_arr->priority = -1;
      after_arr->cpu_secs = -1;
      if (shmdt(p) != 0) {
        printf("failed");
      }
      sem_post(mutex);
      // sem_unlink("EDEB");
      sem_close(mutex);
      return 0;
    }
    after_arr++;
  }
  shmdt(p);
  sem_post(mutex);
  // sem_unlink("EDEB");
  sem_close(mutex);
  return -1;
}

