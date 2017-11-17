#####################################################################
# mysh.c is the name of your source code; you may change this.
# However, you must keep the name of the executable as "mysh".
#
# Type "make" or "make mysh" to compile your code
#
# Type "make clean" to remove the executable (and any object files)
#
######################################################################

CC=gcc
CFLAGS=-Wall -g

library:
	      gcc -c -fpic stats.c -Wall -lpthread
				gcc -shared -o libstats.so stats.o -lpthread
				gcc -o stats_server stats_server.c -Wall -g -lstats -L. libstats.so -lpthread
				gcc -o stats_client stats_client.c -Wall -g -lstats -L. libstats.so -lpthread

clean:
				$(RM) stats_client
				$(RM) stats_server
				$(RM) libstats.so
				$(RM) stats.o

