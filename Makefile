
DEPS := \
	glfw/src/libglfw3.a \
	obj/glad.o \
	cglm \

DEPS2 := lib/utils.c

INCLUDES := -Iglad/GL/include -Iglfw/include -Icglm/include -I.

#FLAGS_GLFW := $(shell pkg-config --static --libs glfw/src/glfw3.pc)
FLAGS_GLFW := -L/usr/local/lib -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp
FLAGS := -g -ldl -Wpedantic -Wall -Wextra ${INCLUDES} -std=c11 ${FLAGS_GLFW} -lm

SRCS := $(wildcard src/*.c)
BINS := $(foreach s,${SRCS},$(patsubst src/%.c,bin/%,$s))

all: ${DEPS} ${BINS}

bin/main2: ${DEPS2}

glad/GL/src/glad.c :
	git clone https://github.com/Dav1dde/glad
	cd glad && git checkout v0.1.33 && python -m glad --generator=c --out-path GL

glfw/src/libglfw3.a :
	git clone https://github.com/glfw/glfw
	cd glfw && git checkout latest && cmake . && make

obj/glad.o : glad/GL/src/glad.c | obj
	gcc -c $^ ${INCLUDES}
	mv glad.o $@

cglm:
	git clone https://github.com/recp/cglm

bin/% : src/%.c ${DEPS} | bin
	gcc $(filter %.o %.a %.c,$^) -o $@ ${FLAGS}

bin obj:
	mkdir $@
