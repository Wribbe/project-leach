#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <signal.h>
#include <cglm/cglm.h>

#define M_PI 3.14159265358979323846

#define NUM_OBJECTS 256
#define UNUSED(x) (void)(x)

float OBJ_X[NUM_OBJECTS] = {0};
float OBJ_Y[NUM_OBJECTS] = {0};
float OBJ_Z[NUM_OBJECTS] = {0};

GLboolean OBJ_FLAG_GRAVITY[NUM_OBJECTS] = {GL_TRUE};

GLboolean key_down[GLFW_KEY_LAST] = {0};

void
callback_key(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  UNUSED(window);
  UNUSED(scancode);
  UNUSED(mods);

  if (action == GLFW_REPEAT) {
    return;
  }

//  printf("key: %d, mods %d, action: %d scancode:%d\n", key, mods, action, scancode);

  if (action == GLFW_PRESS) {
    key_down[key] = GL_TRUE;
  } else {
    key_down[key] = GL_FALSE;
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
  UNUSED(time_current);
  GLfloat color[] = {0.3f, 0.3f, 0.3f, 1.0f};
  glClearBufferfv(GL_COLOR, 0, color);

  glUseProgram(program);
  glBindVertexArray(VAO);

  GLuint uloc_mvp = glGetUniformLocation(program, "mvp");
  glUniformMatrix4fv(uloc_mvp, 1, GL_FALSE, mvp[0]);

  glDrawArrays(GL_TRIANGLES, 0, 6);
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

     0.0f, -0.5f,-0.5f,
     0.0f, -0.5f, 0.5f,
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

  vec3 dir_camera_up = {0.0f, 1.0f, 0.0f};
//  vec3 dir_camera_right = {1.0f, 0.0f, 0.0f};
  vec3 dir_camera = {0.0f, 0.0f, -1.0f};

  vec3 pos_camera = {0.0f, 0.0f, 3.0f};

  float speed_camera = 0.08f;
  float pitch_camera = 0.0f;
  float yaw_camera = -M_PI/2;

  float speed_pitch_camera = 0.01f;
  float speed_yaw_camera = speed_pitch_camera;

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();

    if (key_down[GLFW_KEY_F]) {
      pos_camera[0] -= speed_camera;
    }
    if (key_down[GLFW_KEY_S]) {
      pos_camera[0] += speed_camera;
    }

    if (key_down[GLFW_KEY_E]) {
      pos_camera[2] -= speed_camera;
    }
    if (key_down[GLFW_KEY_D]) {
      pos_camera[2] += speed_camera;
    }

    if (key_down[GLFW_KEY_J]) {
      yaw_camera += speed_yaw_camera;
    }
    if (key_down[GLFW_KEY_L]) {
      yaw_camera -= speed_yaw_camera;
    }
    if (key_down[GLFW_KEY_I]) {
      pitch_camera += speed_pitch_camera;
    }
    if (key_down[GLFW_KEY_K]) {
      pitch_camera -= speed_pitch_camera;
    }

    if (key_down[GLFW_KEY_SPACE]) {
      printf("SPAAACE\n");
      key_down[GLFW_KEY_SPACE] = false;
    }

    dir_camera[0] = cosf(yaw_camera) * cosf(pitch_camera);
    dir_camera[1] = sinf(pitch_camera);
    dir_camera[2] = sinf(yaw_camera) * cosf(pitch_camera);

    glm_look(pos_camera, dir_camera, dir_camera_up, m4_view);
    glm_mat4_mulN((mat4 *[]){&m4_perspective, &m4_view, &m4_model}, 3, m4_mvp);

    render(VAO, glfwGetTime(), program_render, m4_mvp);

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
