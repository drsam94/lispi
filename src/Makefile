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
BINDIR = ../bin
OBJS = $(OBJDIR)/Lexer.o $(OBJDIR)/Parser.o $(OBJDIR)/Evaluator.o

all: debug

release: OPT = 3
release: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o $(BINDIR)/lispi $(STDLINK) -lreadline
debug: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o $(BINDIR)/lispi.debug $(STDLINK) -lreadline
	$(CC) $(CFLAGS) test.cc $(OBJDIR)/*.o -o $(BINDIR)/test_runner $(STDLINK)
sanitize: $(OBJS)
	$(CC) $(CFLAGS) -fsanitize=address main.cc $(OBJDIR)/*.o \
		-o $(BINDIR)/lispi.sanitize $(STDLINK) -lreadline
	$(CC) $(CFLAGS) -fsanitize=address test.cc $(OBJDIR)/*.o \
		-o $(BINDIR)/test_runner.sanitize $(STDLINK)

$(OBJDIR)/%.o : %.cc %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(OBJDIR)/*.o $(BINDIR)/* || true
