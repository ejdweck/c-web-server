# c-web-server

# Usage

0. clean  with
   > make clean

1. Compile with
   > make

2.  run stats_server executable
    TODO: document arugments for stats_server

3.  run stats_client executable
    TODO: document arugments for stats_client


IF RUNNING ON OSX
  It is a known error that unnamed semaphores are not supported.  Thus, we must
  use named semaphores instead.  To use named semaphores use sem_init instead of
  sem_open and sem_close and sem_unlink instead of sem_destroy

  see this stackoverflow post for more info if trying to run on OSX
  https://stackoverflow.com/questions/1413785/sem-init-on-os-x


