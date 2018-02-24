CFLAGS = -Wall -std=c99

smallsh : processArgs.o
	gcc -o smallsh smallsh.c processArgs.c -g $(CFLAGS)

clean :
	rm -rf smallsh *.o *.dSYM
