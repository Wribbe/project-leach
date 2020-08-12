#include "lib/utils.h"

char *
file_read(const char * filepath)
{
  FILE * handle_file = fopen(filepath, "r");
  if (handle_file == NULL) {
    fprintf(stderr, "Error: could not open file %s.\n", filepath);
    exit(EXIT_FAILURE);
  }
  fseek(handle_file, 0, SEEK_END);
  size_t size_file = ftell(handle_file);
  rewind(handle_file);
  char * data = malloc(size_file+1);
  if (data == NULL) {
    fprintf(
      stderr,
      "Error: Could not allocate enough data for reading file.\n"
    );
    exit(EXIT_FAILURE);
  }
  size_t size_read = fread(data, 1, size_file, handle_file);
  if (size_read != size_file) {
    fprintf(
      stderr,
      "Error: read-size: %zu, file-size %zu\n", size_read, size_file
    );
  }
  data[size_file] = '\0';
  return data;
}


GLuint
program_create(const char * path_vert, const char * path_frag)
{
  char * data_vert = file_read(path_vert);
  char * data_frag = file_read(path_frag);

  printf("vert: %s\n", data_vert);
  printf("frag: %s\n", data_frag);

  free(data_vert);
  free(data_frag);

  return 0;
}

GLuint
obj_create(GLuint id_program)
{
  UNUSED(id_program);
  return 0;
}
