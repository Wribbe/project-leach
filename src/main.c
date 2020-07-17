#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <signal.h>
#include <stdint.h>
#include <cglm/cglm.h>

#define M_PI 3.14159265358979323846

#define NUM_OBJECTS 256
#define NUM_PROGRAMS 32
#define NUM_VAO 32

#define UNUSED(x) (void)(x)
typedef uint8_t ID_OBJ;

vec3 obj_pos[NUM_OBJECTS] = {0};
vec3 obj_dir_look[NUM_OBJECTS] = {0};
vec3 obj_dir_up[NUM_OBJECTS] = {0};

GLboolean OBJ_FLAG_GRAVITY[NUM_OBJECTS] = {GL_TRUE};
GLboolean key_down[GLFW_KEY_LAST] = {0};

GLboolean DEBUG = false;

GLuint programs[NUM_PROGRAMS] = {0};
GLuint vao[NUM_VAO] = {0};

ID_OBJ ID_LAST_OBJ = 0;
GLuint ID_LAST_PROGRAM = 0;
GLuint ID_LAST_VAO = 0;

ID_OBJ
id_obj_new(void)
{
  return ID_LAST_OBJ++;
}

GLuint
id_program_new(void)
{
  return ID_LAST_PROGRAM++;
}

GLuint
id_vao_new(void)
{
  GLuint id_vao = ID_LAST_VAO++;
  glGenVertexArrays(1, &vao[id_vao]);
  return id_vao;
}

void
vao_bind(GLuint id_vao)
{
  glBindVertexArray(vao[id_vao]);
}

void
callback_key(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  UNUSED(window);
  UNUSED(scancode);
  UNUSED(mods);

  if (action == GLFW_REPEAT) {
    return;
  }

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
program(const char * path_src_vert, const char * path_src_frag)
{
  GLint success = GL_FALSE;

  char * src_shader_vertex = file_read(path_src_vert);
  char * src_shader_fragment = file_read(path_src_frag);

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

  GLuint id_program = id_program_new();
  programs[id_program] = program;

  return id_program;
}

void
uniform_set_vec3(GLuint id_program, const char * name, vec3 v3)
{
  glUniform3f(
    glGetUniformLocation(programs[id_program], name), v3[0], v3[1], v3[2]
  );
}

void
uniform_set_mat4(GLuint id_program, const char * name, mat4 m4)
{
  glUniformMatrix4fv(
    glGetUniformLocation(programs[id_program], name),
    1,
    GL_FALSE,
    m4[0]
  );
}


void
program_use(GLuint id_program)
{
  glUseProgram(programs[id_program]);
}


void
render(GLuint id_vao, double time_current, GLuint program, mat4 mvp)
{
  UNUSED(time_current);

  GLfloat color[] = {0.3f, 0.3f, 0.3f, 1.0f};
  glClearBufferfv(GL_COLOR, 0, color);
  glClear(GL_DEPTH_BUFFER_BIT);

  vao_bind(id_vao);

  program_use(program);
  uniform_set_vec3(program, "u_color", (vec3){1.0f, 0.0f, 0.0f});
  uniform_set_mat4(program, "mvp", mvp);

  glDrawArrays(GL_TRIANGLES, 0, 3);
  uniform_set_vec3(program, "u_color", (vec3){0.0f, 1.0f, 0.0f});
  glDrawArrays(GL_TRIANGLES, 3, 3);
}

void
obj_dir_look_set(ID_OBJ id_obj, vec3 v3)
{
  obj_dir_look[id_obj][0] = v3[0];
  obj_dir_look[id_obj][1] = v3[1];
  obj_dir_look[id_obj][2] = v3[2];
}

void
obj_dir_up_set(ID_OBJ id_obj, vec3 v3)
{
  obj_dir_up[id_obj][0] = v3[0];
  obj_dir_up[id_obj][1] = v3[1];
  obj_dir_up[id_obj][2] = v3[2];
}

void
obj_pos_set(ID_OBJ id_obj, vec3 v3)
{
  obj_pos[id_obj][0] = v3[0];
  obj_pos[id_obj][1] = v3[1];
  obj_pos[id_obj][2] = v3[2];
}

void
obj_pos_add(ID_OBJ id_obj, vec3 v3)
{
  obj_pos[id_obj][0] += v3[0];
  obj_pos[id_obj][1] += v3[1];
  obj_pos[id_obj][2] += v3[2];
}


float *
obj_pos_get(ID_OBJ id_obj) {
  return obj_pos[id_obj];
}

float *
obj_dir_up_get(ID_OBJ id_obj)
{
  return obj_dir_up[id_obj];
}

float *
obj_dir_look_get(ID_OBJ id_obj)
{
  return obj_dir_look[id_obj];
}

void
debug_draw_dir()
{
  for (ID_OBJ i=0; i<ID_LAST_OBJ; i++) {
  }
}


int main(void)
{
  GLFWwindow* window;

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  uint16_t HEIGH_WINDOW = 640;
  uint16_t WIDTH_WINDOW = 480;
  float ASPECT_WINDOW = (float)HEIGH_WINDOW/(float)WIDTH_WINDOW;

  window = glfwCreateWindow(
    HEIGH_WINDOW,
    WIDTH_WINDOW,
    "Simple example",
    NULL, NULL
  );

  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, callback_key);

  GLuint id_program_render = program(
    "src/shaders/shader.vert",
    "src/shaders/shader.frag"
  );

  GLuint id_vao = id_vao_new();
  vao_bind(id_vao);

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

  glm_perspective_default(ASPECT_WINDOW, m4_perspective);

  ID_OBJ id_camera_camera = id_obj_new();
  ID_OBJ id_camera_overhead = id_obj_new();
  ID_OBJ id_camera_current = id_camera_camera;

  obj_dir_look_set(id_camera_camera, (vec3){0.0f, 0.0f, -1.0f});
  obj_dir_look_set(id_camera_overhead, (vec3){0.0f, -1.0f, 0.0f});

  obj_dir_up_set(id_camera_camera, (vec3){0.0f, 1.0f, 0.0f});
  obj_dir_up_set(id_camera_overhead, (vec3){0.0f, 0.0f, -1.0f});

  obj_pos_set(id_camera_camera, (vec3){0.0f, 0.0f, 3.0f});
  obj_pos_set(id_camera_overhead, (vec3){0.0f, 3.0f, 0.0f});

  float speed_camera = 0.08f;
  float pitch_camera = 0.0f;
  float yaw_camera = -M_PI/2;

  float speed_pitch_camera = 0.01f;
  float speed_yaw_camera = speed_pitch_camera;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();

    if (key_down[GLFW_KEY_SPACE]) {
      id_camera_current++;
      if (id_camera_current == ID_LAST_OBJ) {
        id_camera_current = 0;
      }
      key_down[GLFW_KEY_SPACE] = false;
    }

    if (key_down[GLFW_KEY_Z]) {
      DEBUG = !DEBUG;
      printf("DEBUG: %s\n", DEBUG ? "True" : "False");
      key_down[GLFW_KEY_Z] = false;
    }

    if (key_down[GLFW_KEY_F]) {
      obj_pos_add(id_camera_current, (vec3){speed_camera, 0.0f, 0.0f});
    }
    if (key_down[GLFW_KEY_S]) {
      obj_pos_add(id_camera_current, (vec3){-speed_camera, 0.0f, 0.0f});
    }

    if (key_down[GLFW_KEY_E]) {
      obj_pos_add(id_camera_current, (vec3){0.0f, 0.0f, -speed_camera});
    }
    if (key_down[GLFW_KEY_D]) {
      obj_pos_add(id_camera_current, (vec3){0.0f, 0.0f, speed_camera});
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

//    obj_dir_look_set(
//      id_camera_current,
//      (vec3){
//        cosf(yaw_camera) * cosf(pitch_camera),
//        sinf(pitch_camera),
//        sinf(yaw_camera) * cosf(pitch_camera)
//      }
//    );

//    obj_dir_look_set(
//      id_camera_current,
//      (vec3){
//        cosf(yaw_camera) * cosf(pitch_camera),
//        sinf(pitch_camera),
//        sinf(yaw_camera) * cosf(pitch_camera)
//      }
//    );

    glm_look(
      obj_pos_get(id_camera_current),
      obj_dir_look_get(id_camera_current),
      obj_dir_up_get(id_camera_current),
      m4_view
    );
    glm_mat4_mulN((mat4 *[]){&m4_perspective, &m4_view, &m4_model}, 3, m4_mvp);

    render(id_vao, glfwGetTime(), id_program_render, m4_mvp);

    if (DEBUG) {
      for(ID_OBJ id_obj=0; id_obj<ID_LAST_OBJ; id_obj++) {
        debug_draw_dir(id_obj);
      }
    }

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
