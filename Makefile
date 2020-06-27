FLAGS := -g -ldl -Wpedantic -Wall -Wextra -std=c11 -lm

SRCS := $(wildcard src/*.c)
BINS := $(foreach s,${SRCS},$(patsubst src/%.c,bin/%,$s))

all: ${BINS}

bin/% : src/%.c | bin
	gcc $(filter %.o %.a %.c,$^) -o $@ ${FLAGS}

bin:
	mkdir $@
