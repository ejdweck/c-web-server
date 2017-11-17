#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include "stats.h"

int key;

static void catch_function(int signo) {
        stats_unlink(key);
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
    if (isdigit(argument[i]) == 0 && argument[i] != '.') {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[]) {
    int priority;
    long sleeptime_ns;
    double cputime_ns;
    int option = -1;
    int haveK = 0;
    int haveP = 0;
    int haveS = 0;
    int haveC = 0;
    if (argc > 9) {
      printf("invalid number of args");
      exit(1);
    }
    while ((option = getopt(argc, argv, "k:p:s:c:")) != -1) {
      switch (option) {
        case 'k':
          haveK = 1;
          if (isInt(optarg) != 1) {
            printf("Usage problem");
            exit(1);
          } else {
            key = atoi(optarg);
          }
          break;
        case 'p':
          haveP = 1;
          if (isInt(optarg) != 1) {
            printf("Usage problem");
            exit(1);
          } else {
            priority = atoi(optarg);
          }
          break;
        case 's':
          haveS = 1;
          if (isInt(optarg) != 1) {
            printf("Usage problem");
            exit(1);
          } else {
            sleeptime_ns = atoi(optarg);
          }
          break;
        case 'c':
          haveC = 1;
          if (isInt(optarg) != 1) {
            printf("Usage problem");
            exit(1);
          } else {
            cputime_ns = (double)atoi(optarg);
          }
          break;
        default:
          printf("Usage problem");
          exit(1);
      }
    }
    // how to handle if key isn't entered ?
    if (haveK == 0) {
        printf("Usage problem");
        exit(1);
    }
    // ask ta about reasonable values
    if (haveP == 0) {
      priority = 10;
    }
    if (haveS == 0) {
      sleeptime_ns = 1000;
    }
    if (haveC == 0) {
      cputime_ns = 1000000;
    }


    char *name;
    name = argv[0];

    if (signal(SIGINT, catch_function) == SIG_ERR) {
        printf("error handling signal");
        if (stats_unlink(key) == -1) {
          printf("unlink failed!");
          exit(1);
        }
        return 0;
    }

    stats_t* shared_addr;
    shared_addr = stats_init(key);
    printf("%p\n", shared_addr);

    if (shared_addr == NULL) {
      printf("init failed");
      exit(1);
    }

    // set priority
    int priority_check;
    if ((priority_check = setpriority(PRIO_PROCESS, 0, priority)) == -1) {
      printf("priority check FAILED");
    }
    shared_addr->pid = getpid();
    shared_addr->cpu_secs = 0;
    shared_addr->priority = priority;

    int j;
    for (j = 0; j < 15; j++) {
      shared_addr->client_name[j] = name[j];
    }

    shared_addr->counter++;  // does count start now or inside loop

    struct timespec first_time;
    struct timespec curr_time;
    struct timespec nanoS;
    nanoS.tv_sec = sleeptime_ns/1000000000;
    nanoS.tv_nsec = sleeptime_ns % 1000000000;
    while (1) {
      nanosleep(&nanoS, NULL);
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curr_time);
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &first_time);
      while ((curr_time.tv_nsec + (curr_time.tv_sec*1000000000)) -
          (first_time.tv_nsec + (first_time.tv_sec*1000000000)) < cputime_ns) {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curr_time);
      }
      shared_addr->cpu_secs += ((curr_time.tv_nsec +
            (curr_time.tv_sec*1000000000)) - (first_time.tv_nsec +
              (first_time.tv_sec*1000000000)))/1000000000.0;
      // update time thats passed
      shared_addr->counter++;
      shared_addr->priority = getpriority(PRIO_PROCESS, 0);
      // printf("priority: %d\n", shared_addr->priority);
    }
  return 0;
}
