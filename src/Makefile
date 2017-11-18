CC = g++
OPT = 0
WARNINGS = -Wall -Werror -Wextra -Wpedantic -Wshadow -Wformat
# workaround for https://bugs.llvm.org/show_bug.cgi?id=33222
# we need to compile against libc++, and we also need to make sure
# the linker knows how to do that; this assumes you have an environment
# variable CLANG pointing to a clang installation, it's possible that you can leave
# STDLINK undefined on your system. Or, easier solution, just don't use clang
ifeq ($(CC), clang++)
	STDLIB= --stdlib=libc++
	STDLINK= -L$(CLANG)/lib -Wl,-rpath,$(CLANG)/lib -lc++abi
else
	STDLIB=
	STDLINK=
endif
CFLAGS = -g --std=c++1z $(WARNINGS) $(STDLIB) -O$(OPT)
OBJDIR = ../build
OBJS = $(OBJDIR)/Lexer.o $(OBJDIR)/Parser.o $(OBJDIR)/Evaluator.o

all: release test debug

# When there is something of reasonable quality to "relase", we should turn optimizations
# for the release
#release: OPT = 0
release: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o ../bin/lispi $(STDLINK) -lreadline
debug: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o ../bin/lispi.debug $(STDLINK) -lreadline
test: $(OBJS)
	$(CC) $(CFLAGS) test.cc $(OBJDIR)/*.o -o ../bin/test_runner $(STDLINK)

$(OBJDIR)/%.o : %.cc %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(OBJDIR)/*.o
