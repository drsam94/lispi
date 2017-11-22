CC = g++
OPT = 0
# workaround for https://bugs.llvm.org/show_bug.cgi?id=33222
# we need to compile against libc++, and we also need to make sure
# the linker knows how to do that; this assumes you have an environment
# variable CLANG pointing to a clang installation, it's possible that you can leave
# STDLINK undefined on your system. Or, easier solution, just don't use clang
ifeq ($(CC), clang++)
	STDLIB = --stdlib=libc++
	STDLINK = -L$(CLANG)/lib -Wl,-rpath,$(CLANG)/lib -lc++abi
	# There is a bug with -Wundefined-func-template in clang5.0 with class tempalte deduction
	# guides. This bug has been fixed in trunk, so we should be able to reenable that warning
	# with the next update of clang. See https://bugs.llvm.org/show_bug.cgi?id=34776
	EXTRAWARN = -Weverything -Wno-c++98-c++11-c++14-compat -Wno-c++98-compat \
				-Wno-exit-time-destructors -Wno-global-constructors -Wno-c++98-compat-pedantic \
				-Wno-padded -Wno-undefined-func-template -Wno-missing-variable-declarations \
				-Wno-missing-prototypes
else
	STDLIB =
	STDLINK =
	EXTRAWARN = -Wstrict-null-sentinel -Wchkp -Wsuggest-override -Wlogical-op
endif
WARNINGS = -Wall -Werror -Wextra -Wpedantic -Wshadow -Wformat -Weffc++ \
		   -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wdouble-promotion \
		   -Wimplicit-fallthrough -Wuninitialized $(EXTRAWARN)
CFLAGS = -g --std=c++17 $(WARNINGS) $(SANITIZE) $(STDLIB) -O$(OPT)
OBJDIR = ../build
BINDIR = ../bin
OBJS = $(OBJDIR)/Lexer.o $(OBJDIR)/Parser.o $(OBJDIR)/Evaluator.o $(OBJDIR)/Data.o

all: debug

# TODO: generate different objects (different build dirs) fr the different variants
# so that a make clean isn't always necessary before building a new variant
release: OPT = 3
release: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o $(BINDIR)/lispi $(STDLINK) -lreadline
debug: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o -o $(BINDIR)/lispi.debug $(STDLINK) -lreadline
	$(CC) $(CFLAGS) test.cc $(OBJDIR)/*.o -o $(BINDIR)/test_runner $(STDLINK)

# Note: there are some sanitizatio errors when running the REPL that come from GNU readline
sanitize: SANITIZE = -fsanitize=address -fsanitize=leak -fsanitize=undefined -fstack-check
sanitize: $(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJDIR)/*.o \
		-o $(BINDIR)/lispi.sanitize $(STDLINK) -lreadline
	$(CC) $(CFLAGS) test.cc $(OBJDIR)/*.o \
		-o $(BINDIR)/test_runner.sanitize $(STDLINK)

$(OBJDIR)/%.o : %.cc %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(OBJDIR)/*.o $(BINDIR)/* || true
