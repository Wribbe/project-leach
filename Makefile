
DEPS := \
	glfw/src/libglfw3.a \
	glad/GL/src/glad.c

INCLUDES := -Iglad/GL/include -Iglfw/include

FLAGS_GLFW := $(shell pkg-config --static --libs glfw/src/glfw3.pc)
FLAGS := -ldl -Wpedantic -Wall -Wextra ${INCLUDES} -std=c11 ${FLAGS_GLFW}

SRCS := $(wildcard src/*.c)
BINS := $(foreach s,${SRCS},$(patsubst src/%.c,bin/%,$s))

all: ${DEPS} ${BINS} | bin

glad/GL/src/glad.c :
	git clone https://github.com/Dav1dde/glad
	cd glad && git checkout v0.1.33 && python -m glad --generator=c --out-path GL

glfw/src/libglfw3.a :
	git clone https://github.com/glfw/glfw
	cd glfw && git checkout latest
	cmake . && make

bin/% : src/%.c ${DEPS}
	gcc $^ -o $@ ${FLAGS}

bin:
	mkdir $@
