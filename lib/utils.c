#include "lib/utils.h"

GLuint vaos[NUM_OBJECTS] = {0};
GLuint programs[NUM_OBJECTS] = {0};
GLuint mat4_models[NUM_OBJECTS] = {0};

GLuint id_last_vao = 0;
GLuint id_last_program = 0;
GLuint id_last_object = 0;

GLuint id_camera_current = 0;

struct object objects[NUM_OBJECTS] = {0};

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
program_create(const char * path_vertex, const char * path_fragment)
{
  const char * data_vertex = file_read(path_vertex);
  const char * data_fragment = file_read(path_fragment);

  GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(shader_vertex, 1, &data_vertex, NULL);
  glShaderSource(shader_fragment, 1, &data_fragment, NULL);

  int success = 0;
  size_t size_info = 512;
  char info[size_info];

  glCompileShader(shader_vertex);
  glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader_vertex, size_info, NULL, info);
    fprintf(stderr, "Failed to compile vertex shader: %s\n", info);
    exit(EXIT_FAILURE);
  }

  glCompileShader(shader_fragment);
  glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader_fragment, size_info, NULL, info);
    fprintf(stderr, "Failed to compile fragment shader: %s\n", info);
    exit(EXIT_FAILURE);
  }

  free((void *)data_vertex);
  free((void *)data_fragment);

  GLuint id_program = glCreateProgram();

  glAttachShader(id_program, shader_vertex);
  glAttachShader(id_program, shader_fragment);

  glLinkProgram(id_program);
  glGetProgramiv(id_program, GL_LINK_STATUS, &success);

  if (!success) {
    fprintf(stderr, "Failed to link program.\n");
    exit(EXIT_FAILURE);
  }

  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);

  programs[id_last_program] = id_program;

  return id_last_program++;
}

void
program_use(GLuint id_program) {
  glUseProgram(programs[id_program]);
}

GLuint
obj_create(GLuint id_program)
{
  UNUSED(id_program);
  return 0;
}
