#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <signal.h>
#include <cglm/cglm.h>

#define NUM_OBJECTS 256
#define UNUSED(x) (void)(x)

float OBJ_X[NUM_OBJECTS] = {0};
float OBJ_Y[NUM_OBJECTS] = {0};
float OBJ_Z[NUM_OBJECTS] = {0};

GLboolean OBJ_FLAG_GRAVITY[NUM_OBJECTS] = {GL_TRUE};

struct key {
  GLboolean down;
};

struct key keys[GLFW_KEY_LAST] = {0};

void
callback_key(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  UNUSED(window);

  if (action == GLFW_REPEAT) {
    return;
  }

  printf("key: %d, mods %d, action: %d scancode:%d\n", key, mods, action, scancode);

  if (action == GLFW_PRESS) {
    keys[key].down = GL_TRUE;
  } else {
    keys[key].down = GL_FALSE;
  }
}

char *
file_read(const char * path)
{
  FILE * fh = fopen(path, "r");
  if (fh == NULL) {
    fprintf(stderr, "COULD NOT OPEN FILE %s\n.", path);
    return NULL;
  }

  fseek(fh, 0, SEEK_END);

  size_t size_string = ftell(fh);
  rewind(fh);

  char * data = malloc(size_string);
  if (data == NULL) {
    fprintf(stderr, "COULD NOT ALLOCATE DATA\n.");
    return NULL;
  }

  fread(data, 1, size_string, fh);
  fclose(fh);
  return data;
}

GLuint
shaders_compile(void)
{
  GLint success = GL_FALSE;

  char * src_shader_vertex = file_read("src/shaders/shader.vert");
  char * src_shader_fragment = file_read("src/shaders/shader.frag");

  // Create and compile vertex shader.
  GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader_vertex, 1, (const char **)(&src_shader_vertex), NULL);
  glCompileShader(shader_vertex);
  glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    fprintf(stderr, "Vertex shader failed to compile.\n");
  }

  // Create and compile fragment shader.
  GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader_fragment, 1, (const char **)(&src_shader_fragment), NULL);
  glCompileShader(shader_fragment);
  glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    fprintf(stderr, "Fragment shader failed to compile.\n");
  }

  // Create program, attatch shaders to it, ad link it.
  GLuint program = glCreateProgram();
  glAttachShader(program, shader_vertex);
  glAttachShader(program, shader_fragment);
  glLinkProgram(program);

  // Did it link?
  glGetShaderiv(shader_fragment, GL_LINK_STATUS, &success);
  if (!success) {
    fprintf(stderr, "Program failed to link.\n");
  }

  // Delete the shaders as the program has them now.
  glDeleteShader(shader_fragment);
  glDeleteShader(shader_vertex);

  free(src_shader_vertex);
  free(src_shader_fragment);

  return program;
}

void
render(GLuint VAO, double time_current, GLuint program, mat4 mvp)
{
  GLfloat color[] = {
    sinf(time_current) * 0.5f + 0.5f,
    cosf(time_current) * 0.5f + 0.5f,
    0.0f,
    1.0f,
  };
  glClearBufferfv(GL_COLOR, 0, color);

  glUseProgram(program);
  glBindVertexArray(VAO);

  GLuint uloc_mvp = glGetUniformLocation(program, "mvp");
  glUniformMatrix4fv(uloc_mvp, 1, GL_TRUE, mvp[0]);

  glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(void)
{
  GLFWwindow* window;

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, callback_key);

  GLuint program_render = shaders_compile();
  GLuint VAO = 0;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f,
  };

  GLuint VBO = 0;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  mat4 m4_model = GLM_MAT4_IDENTITY_INIT;
  mat4 m4_view = GLM_MAT4_IDENTITY_INIT;
  mat4 m4_perspective = GLM_MAT4_IDENTITY_INIT;
  mat4 m4_mvp = GLM_MAT4_IDENTITY_INIT;

  glm_perspective(40.0f, 1.0f, 1.0f, 100.0f, m4_perspective);
  vec3 up = {0.0f, 1.0f, 0.0f};
  float camera_z = 3.0f;


  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    glm_lookat((vec3){0.0f, 0.0f, camera_z}, (vec3){0.0f, 0.0f, 0.0f}, up, m4_view);
    camera_z-= 0.01f;
    glm_mat4_mulN((mat4 *[]){&m4_perspective, &m4_view, &m4_model}, 3, m4_mvp);
    render(VAO, glfwGetTime(), program_render, m4_mvp);
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
