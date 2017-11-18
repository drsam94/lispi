CC = g++
OPT = 3
WARNINGS = -Wall -Werror -Wextra -Wpedantic -Wshadow -Wformat
CFLAGS = -g --std=c++1z $(WARNINGS) -O$(OPT)
OBJDIR = ../build
OBJS = $(OBJDIR)/Lexer.o $(OBJDIR)/Parser.o $(OBJDIR)/Evaluator.o

all: release test debug

release: OPT = 0
release: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o ../bin/lispi -lreadline
debug: OPT = 0
debug: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o ../bin/lispi.debug -lreadline
test: OPT = 0
test: $(OBJS)
	$(CC) $(CFLAGS) test.cc $(OBJDIR)/*.o -o ../bin/test_runner

$(OBJDIR)/%.o : %.cc %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(OBJDIR)/*.o
