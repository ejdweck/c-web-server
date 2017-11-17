#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <ctype.h>
#include "stats.h"

#define MAX_STRUCTS 16

key_t key;
void *p;
int id;
sem_t *mutex;

static void catch_function(int signo) {
  printf("%p\n", p);
  if (shmdt(p) == -1) {
    printf("failed shmdt");
  }

/*  int nattach = shmctl(key, IPC_STAT, 0);
  printf("nattach val: %d\n", nattach);
*/
  if (shmctl(id, IPC_RMID, NULL) == -1) {
    printf("failed shmctl\n");
  }
  sem_unlink("EDEB");
  sem_close(mutex);
  printf("Interactive attention signal caught\n");
  exit(1);
}

/**
 * returns 0 if the argument is NOT an int
 *
 * returns 1 if the argument IS an int
 * */
int isInt(char *argument) {
  int length = strlen(argument);
  int i;
  for (i = 0; i < length; i++) {
    if (isdigit(argument[i]) == 0) {
      return 0;
    }
  }
  return 1;
}


int main(int argc, char *argv[]) {
  key = -1;
  if (signal(SIGINT, catch_function) == SIG_ERR) {
    printf("error handling signal");
    return 1;
  }

  if (argc != 3) {
    printf("too many args");
    return 1;
  }
  int k_present = 0;  // set to 1 if k is present in args
  int opt;
  while ((opt = getopt(argc, argv, "k:")) != -1) {
    switch (opt) {
      case 'k':
        k_present = 1;
        break;
      default:
      printf("Usage: stat_server -k [integer]");
    }
  }

  if (k_present != 1) {
    printf("Usage: stat_server -k [integer]");
    return 1;  // ask a Ta
  } else {
      if (isInt(argv[2]) == 1) {
        key = atoi(argv[2]);
      } else {
        printf("Usage: stat_server -k [integer]");
      }
  }

  if (key == -1) {
    // perror("key is null");
    exit(1);
  }


  if ((mutex = sem_open("EDEB", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }

  int sem_check = sem_init(mutex, 1, 1);
  // printf("sem value: %d\n", *mutex);

  // initalize semaphore

  if (sem_check == -1) {
    printf("sem_init failed!");
  }

  size_t page_size = getpagesize();
  id = shmget(key, page_size, IPC_CREAT | IPC_EXCL | 0666);
  if (id < 0) {
    printf("shmget");
    exit(1);
  }

  p = (stats_t*) shmat(id, NULL, 0);
  memset(p, '\0', page_size);

  if (p == (void*)-1) {
    printf("shmat");
    exit(1);
  }

  inuse_t* array_inuse;
  array_inuse = p;

  int i;
  // initalize the array to all 0
  for (i = 0; i < 16; i++) {
    array_inuse->array[i] = 0;
  }


  array_inuse += 1;
  stats_t* np;
  np = (stats_t*)array_inuse;
  stats_t* printing_ptr;
  stats_t* reset_p = np;
  for (i = 0; i < 16; i++) {
    np->pid = -1;
    np->counter = 0;
    np->priority = 0;
    np->cpu_secs = 0;
    // p->client_name = -1;
    np++;
  }

  int server_iteration = 0;
  while (1) {
    printf("\n");
    printing_ptr = reset_p;
    sleep(1);
    for (i = 0; i < MAX_STRUCTS; i++) {
      if (printing_ptr->pid != -1) {
        // print the necessary data
        printf("%d ", server_iteration);
        printf("%d ", printing_ptr->pid);
        printf("%s ", printing_ptr->client_name);
        printf("%d ", printing_ptr->counter);
        printf("%.2f ", printing_ptr->cpu_secs);
        printf("%d\n", printing_ptr->priority);
      }
      printing_ptr++;
    }
    server_iteration++;
  }
  return 0;
}




