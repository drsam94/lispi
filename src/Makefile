CC = g++
CFLAGS = -g --std=c++1z -Wall -Werror -Wextra -Wpedantic
OBJS = allObjs

all: release test debug
release: $(OBJS)
	$(CC) $(CFLAGS) -O3 main.cc *.o -o ../bin/lispi
debug: $(OBJS)
	$(CC) $(CFLAGS) -O0 main.cc *.o -o ../bin/lispi.debug
test: $(OBJS)
	$(CC) $(CFLAGS) -O0 test.cc *.o -o ../bin/test_runner

allObjs:
	$(CC) $(CFLAGS) -O3 -c Lexer.cc Parser.cc
clean:
	rm *.o
