CFLAGS = -Wall 

buildrooms: lewikevi.buildrooms.c
	gcc -o lewikevi.buildrooms -g lewikevi.buildrooms.c $(CFLAGS)

adventure: lewikevi.adventure.c
	gcc -o lewikevi.adventure -g lewikevi.adventure.c -lpthread $(CFLAGS)

all: buildrooms adventure

clean:
	rm -f *.o *.gcov *.gcda *.gcno *.so *.out
