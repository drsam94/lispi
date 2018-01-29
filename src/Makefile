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
	# There is a bug with -Wundefined-func-template in clang5.0 with class template deduction
	# guides. This bug has been fixed in trunk, so we should be able to reenable that warning
	# with the next update of clang. See https://bugs.llvm.org/show_bug.cgi?id=34776
	WARNINGS = -Weverything -Wno-c++98-c++11-c++14-compat -Wno-c++98-compat \
				-Wno-exit-time-destructors -Wno-global-constructors -Wno-c++98-compat-pedantic \
				-Wno-padded -Wno-undefined-func-template -Wno-missing-variable-declarations \
				-Wno-missing-prototypes -Wno-weak-vtables
else
	STDLIB =
	STDLINK =
	WARNINGS = -Wstrict-null-sentinel -Wchkp -Wsuggest-override -Wlogical-op \
				-Wall -Wextra -Wpedantic -Wshadow -Wformat -Weffc++ \
				-Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wdouble-promotion \
				-Wimplicit-fallthrough -Wuninitialized
endif
CFLAGS = -g --std=c++17 -I. -Werror $(WARNINGS) $(SANITIZE) $(STDLIB) -O$(OPT)
OBJDIR = ../build/$(VARIANT).$(CC)
BINDIR = ../bin/$(VARIANT).$(CC)
OBJS = $(OBJDIR)/core/Lexer.o $(OBJDIR)/core/Parser.o $(OBJDIR)/core/Evaluator.o $(OBJDIR)/data/Data.o \
	   $(OBJDIR)/data/BigInt.o $(OBJDIR)/library/SpecialForms.o $(OBJDIR)/library/SystemMethods.o

all: debug

# This second expansion directive is necessary in order to get the change of variant
# to apply in the expansion of OBJS
.SECONDEXPANSION:
release: OPT = 3
release: VARIANT=rel
release: $$(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJS) -o $(BINDIR)/lispi $(STDLINK) -lreadline
	$(CC) $(CFLAGS) test.cc $(OBJS) -o $(BINDIR)/test_runner $(STDLINK)

debug: VARIANT = debug
debug: $$(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJS) -o $(BINDIR)/lispi $(STDLINK) -lreadline
	$(CC) $(CFLAGS) test.cc $(OBJS) -o $(BINDIR)/test_runner $(STDLINK)

# Note: there are some sanitizatio errors when running the REPL that come from GNU readline
sanitize: SANITIZE = -fsanitize=address -fsanitize=leak -fsanitize=undefined -fstack-check
sanitize: VARIANT = sanitize
sanitize: $$(OBJS)
	$(CC) $(CFLAGS) main.cc $(OBJS) -o $(BINDIR)/lispi $(STDLINK) -lreadline
	$(CC) $(CFLAGS) test.cc $(OBJS) -o $(BINDIR)/test_runner $(STDLINK)

# There is probably a way to do this in a patterned based way, but oh well
# also have to make the associated bin directories...
MAKE_DIR=@mkdir -p $(@D)
MAKE_OBJ=$(CC) $(CFLAGS) -c -o $@ $<
../build/rel.g++/%.o : %.cc %.h
	$(MAKE_DIR)
	$(MAKE_OBJ)
../build/debug.g++/%.o : %.cc %.h
	$(MAKE_DIR)
	$(MAKE_OBJ)
../build/sanitize.g++/%.o : %.cc %.h
	$(MAKE_DIR)
	$(MAKE_OBJ)
../build/rel.clang++/%.o : %.cc %.h
	$(MAKE_DIR)
	$(MAKE_OBJ)
../build/debug.clang++/%.o : %.cc %.h
	$(MAKE_DIR)
	$(MAKE_OBJ)
../build/sanitize.clang++/%.o : %.cc %.h
	$(MAKE_DIR)
	$(MAKE_OBJ)
clean:
	rm ../build/*/*.o ../build/*/*/*.o || true
