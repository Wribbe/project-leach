#include <stdlib.h>
#include <stdio.h>

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)x

#define NUM_OBJECTS 512

struct object {
  GLuint id_vao;
  GLuint id_program;
  GLuint id_model;
};

char *
file_read(const char * filepath);

GLuint
program_create(const char * path_vert, const char * path_frag);

void
program_use(GLuint id_program);

GLuint
obj_create(GLuint id_program);
