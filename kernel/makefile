include settings.mk

# Set compiler options
CC=gcc

# Set prerequisites
SRCS_C += $(shell find src/ -iname "*.c")
SRCS_H += $(shell find src/ -iname "*.h")
TEST_SRCS_C += $(shell find tests/ -iname "*.c")
TEST_OBJS = $(patsubst tests/%.c,obj/tests/%.o,$(TEST_SRCS_C))

DEPS = $(foreach STL,$(STATIC_LIBPATHS),$(STL:%=%/bin/lib$(notdir $(STL)).a))

# Set header paths to (-I)nclude
IDIRS += $(addsuffix /src,$(STATIC_LIBPATHS) .)

# Set library paths to (-L)ook
LIBDIRS = $(addsuffix /bin,$(STATIC_LIBPATHS))

# Set intermediate objects
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS_C))

# Set binary targets
BIN = bin/$(shell cd . && pwd | xargs basename)
TEST_BIN = bin/tests

# Configuration file path (can be overridden via command line)
config ?=

.PHONY: all
all: debug

.PHONY: debug
debug: CFLAGS = $(CDEBUG)
debug: $(BIN)

.PHONY: release
release: CFLAGS = $(CRELEASE)
release: $(BIN)

.PHONY: clean
clean:
	-rm -rfv obj bin

.PHONY: kernel
kernel: $(BIN)
	./$(BIN) $(config)

.PHONY: test
test: $(TEST_BIN)
	./$(TEST_BIN)

$(BIN): $(OBJS) | $(dir $(BIN))
	$(CC) $(CFLAGS) -o "$@" $^ $(IDIRS:%=-I%) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) 

obj/%.o: src/%.c $(SRCS_H) $(DEPS) | $(dir $(OBJS))
	$(CC) $(CFLAGS) -c -o "$@" $< $(IDIRS:%=-I%)

$(TEST_BIN): $(TEST_OBJS) | $(dir $(TEST_BIN))
	$(CC) $(CFLAGS) -o "$@" $^ $(IDIRS:%=-I%) $(LIBDIRS:%=-L%) $(LIBS:%=-l%) -lcspecs

obj/tests/%.o: tests/%.c $(SRCS_H) $(DEPS) | $(dir $(TEST_OBJS))
	$(CC) $(CFLAGS) -c -o "$@" $< $(IDIRS:%=-I%)

.SECONDEXPANSION:
$(DEPS): $$(shell find $$(patsubst %bin/,%src/,$$(dir $$@)) -iname "*.c" -or -iname "*.h")
	make --no-print-directory -C $(patsubst %bin/,%,$(dir $@)) 3>&1 1>&2 2>&3 | sed -E 's,(src/)[^ ]+\.(c|h)\:,$(patsubst %bin/,%,$(dir $@))&,' 3>&2 2>&1 1>&3

$(sort $(dir $(BIN) $(OBJS)) $(dir $(TEST_BIN) $(TEST_OBJS))):
	mkdir -pv $@

.PHONY: start
start: debug
	valgrind --tool=none ./$(BIN) $(config) $(ARGS)

.PHONY: memcheck
memcheck: debug
	valgrind --leak-check=full $(MEMCHECK_FLAGS) ./$(BIN) $(config) $(ARGS)

.PHONY: helgrind
helgrind: debug
	valgrind --tool=helgrind $(HELGRIND_FLAGS) ./$(BIN) $(config) $(ARGS)
