#include "lib/utils.h"

char *
file_read(const char * filepath)
{
  FILE * handle_file = fopen(filepath, "r");
  if (handle_file == NULL) {
    fprintf(stderr, "Error: could not open file %s.\n", filepath);
    exit(EXIT_FAILURE);
  }
  printf("Could open file: %s\n", filepath);
  return NULL;
}


GLuint
program_create(const char * path_vert, const char * path_frag)
{
  UNUSED(path_vert);
  UNUSED(path_frag);
  return 0;
}

GLuint
obj_create(GLuint id_program)
{
  UNUSED(id_program);
  return 0;
}
