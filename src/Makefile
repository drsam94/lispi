CC = g++
CFLAGS = --std=c++1z -Wall -Werror -Wextra -Wpedantic

all:
	$(CC) $(CFLAGS) -O3 -g main.cc -o ../bin/lispi
debug:
	$(CC) $(CFLAGS) -O0 -g main.cc -o ../bin/lispi.debug

clean:
	rm *.o
