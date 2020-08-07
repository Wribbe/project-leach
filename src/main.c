#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <cglm/cglm.h>

#define M_PI 3.14159265358979323846

#define NUM_OBJECTS 256
#define NUM_PROGRAMS 32
#define NUM_VAO 32

#define UNUSED(x) (void)(x)
typedef uint8_t ID_OBJ;
typedef GLuint TYPE_INDICES;

vec3 obj_pos[NUM_OBJECTS] = {0};
vec3 obj_dir_look[NUM_OBJECTS] = {0};
vec3 obj_dir_up[NUM_OBJECTS] = {0};

GLboolean OBJ_FLAG_GRAVITY[NUM_OBJECTS] = {GL_TRUE};
GLboolean key_down[GLFW_KEY_LAST] = {0};

GLboolean DEBUG = false;

GLuint programs[NUM_PROGRAMS] = {0};
GLuint vaos[NUM_VAO] = {0};

ID_OBJ ID_LAST_OBJ = 0;
GLuint ID_LAST_PROGRAM = 0;
GLuint ID_LAST_VAO = 0;

GLuint id_vao_debug = NUM_VAO+1;

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
  glGenVertexArrays(1, &vaos[id_vao]);
  return id_vao;
}

struct VAO {
  GLuint id;
  size_t num_indices;
};

struct VAO
vao_new(void)
{
  return (struct VAO){
    .id = id_vao_new()
  };
}

void
vao_bind_id(GLuint id_vao)
{
  glBindVertexArray(vaos[id_vao]);
}

void
vao_bind(struct VAO * vao)
{
  vao_bind_id(vao->id);
}

void
vao_unbind(void)
{
  glBindVertexArray(0);
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

  char * data = malloc(size_string+1);
  if (data == NULL) {
    fprintf(stderr, "COULD NOT ALLOCATE DATA\n.");
    return NULL;
  }

  fread(data, 1, size_string, fh);
  data[size_string] = '\0';
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
render(struct VAO * VAO, double time_current, GLuint program, mat4 mvp)
{
  UNUSED(time_current);

  GLfloat color[] = {0.3f, 0.3f, 0.3f, 1.0f};
  glClearBufferfv(GL_COLOR, 0, color);
  glClear(GL_DEPTH_BUFFER_BIT);

  vao_bind(VAO);

  program_use(program);
  uniform_set_mat4(program, "mvp", mvp);

//  glDrawElements(
//    GL_TRIANGLES,
//    VAO->num_indices,
//    GL_UNSIGNED_INT,
//    (void*)(0)
//  );

  for (size_t i=0; i<VAO->num_indices; i += 3) {
    uniform_set_vec3(program, "u_color", (vec3){
      (float)rand()/RAND_MAX,
      (float)rand()/RAND_MAX,
      (float)rand()/RAND_MAX
    });
    glDrawElements(
      GL_TRIANGLES,
      3,
      GL_UNSIGNED_INT,
      (void*)(i*sizeof(TYPE_INDICES))
    );
//    printf("%zu\n", i);
  }

  if (DEBUG) {
    if (id_vao_debug > NUM_VAO) {

      GLfloat verts[] = {
        // Triangle-top.
        0.0f, 1.0f, 0.0f,
       -0.3f, 0.7f, 0.0f,
        0.3f, 0.7f, 0.0f,
        // Line.
        0.0f, 0.7f, 0.0f,
        0.0f, 0.0f, 0.0f,
      };

      id_vao_debug = id_vao_new();
      vao_bind_id(id_vao_debug);
      GLuint VBO = 0;

      glGenBuffers(1, &VBO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

      glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3*sizeof(float),
        (void*)0
      );

      glEnableVertexAttribArray(0);
      vao_unbind();
    }

    vao_bind_id(id_vao_debug);

    uniform_set_vec3(program, "u_color", (vec3){1.0f, 1.0f, 1.0f});
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_LINES, 3, 2);

//    for (ID_OBJ i=0; i<ID_LAST_OBJ; i++) {
//    }

    vao_unbind();
  }
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


GLboolean
starts_with(const char * s, const char * match)
{
  for (size_t i=0; i<strlen(match); i++) {
    if (*(s+i) != *(match+i)) {
      return GL_FALSE;
    }
  }
  return GL_TRUE;
}

struct obj_data {
  size_t size_vertices;
  size_t size_indices;
  size_t num_indices;
  size_t num_vertices;
  float * vertices;
  TYPE_INDICES * indices;
};

void
obj_pos_forward(ID_OBJ id_obj, float speed, vec3 dir)
{
  vec3 look = {0};

  if (dir == NULL) {
    glm_vec3_copy(obj_dir_look_get(id_obj), look);
  } else {
    glm_vec3_copy(dir, look);
  }

  vec3 result = {0};

  glm_vec3_scale(look, speed, result);
  obj_pos_add(id_obj, result);
}

void
obj_pos_strafe(ID_OBJ id_obj, float speed)
{
  vec3 right = {0};
  vec3 result = {0};

  float * look = obj_dir_look_get(id_obj);
  float * up = obj_dir_up_get(id_obj);
  glm_vec3_cross(look, up, right);
  glm_vec3_normalize(right);
  glm_vec3_scale(right, speed, result);
  obj_pos_add(id_obj, result);
}

struct obj_data
obj_load(const char * path_obj)
{
  char * data = file_read(path_obj);
  char * p_data = data;

  if (data == NULL) {
    fprintf(stderr, "Could not open file %s, aborting.", path_obj);
    exit(EXIT_FAILURE);
  }

  #define TO_NEXT_CHAR(c) while (*p_data != c && *p_data != '\n'){p_data++;}
  #define TO_NEXT_LINE() while (*p_data != '\n'){p_data++;};p_data++;

  struct obj_data obj_data = {0};

  size_t NUM_VERTICES = 128;
  obj_data.vertices = malloc(sizeof(float)*NUM_VERTICES);
  if (obj_data.vertices == NULL) {
    fprintf(stderr, "Could not allocate memory for vertices in obj_load.\n");
    exit(EXIT_FAILURE);
  }
  float * p_vertices = obj_data.vertices;

  size_t NUM_INDICES = 128;
  obj_data.indices = malloc(sizeof(TYPE_INDICES)*NUM_INDICES);
  if (obj_data.indices == NULL) {
    fprintf(stderr, "Could not allocate memory for indices in obj_load.\n");
    exit(EXIT_FAILURE);
  }
  TYPE_INDICES * p_indices = obj_data.indices;

  for (;;) {
    switch(*p_data) {
      case('#'):
      case('o'):
      case('s'):
        TO_NEXT_LINE();
        continue;
        break;
    }
    if (*p_data == '\0') {
      break;
    }
    if (starts_with(p_data, "v ")) {
      for(;;) {
        TO_NEXT_CHAR(' ');
        if (*p_data++ == '\n') {
          break;
        }
        sscanf(p_data, "%f", p_vertices++);
        if ((size_t)(p_vertices - obj_data.vertices) == NUM_VERTICES) {
          size_t offset_current = NUM_VERTICES;
          NUM_VERTICES *= 2;
          printf("New num_vertice size: %zu\n", NUM_VERTICES);
          obj_data.vertices = realloc(
            obj_data.vertices,
            sizeof(float)*NUM_VERTICES
          );
          p_vertices = obj_data.vertices+offset_current;
        }
      }
    } else if (starts_with(p_data, "f ")) {
      for(;;) {
        TO_NEXT_CHAR(' ');
        if (*p_data++ == '\n') {
          break;
        }
        sscanf(p_data, "%u", p_indices);
        *p_indices -= 1;
        p_indices++;
        if ((size_t)(p_indices - obj_data.indices) == NUM_INDICES) {
          size_t offset_current = NUM_INDICES;
          NUM_INDICES *= 2;
          printf("New num_indices size: %zu\n", NUM_INDICES);
          obj_data.indices = realloc(
            obj_data.indices,
            sizeof(float)*NUM_INDICES
          );
          p_indices = obj_data.indices+offset_current;
        }
      }
    } else { // Ignore remaining.
      TO_NEXT_LINE();
    }
  }

  for (TYPE_INDICES * pi = obj_data.indices; pi<p_indices; pi++) {
    printf("%d ", *pi);
  }
  printf("\n");

  free(data);

  obj_data.num_vertices = p_vertices - obj_data.vertices;
  obj_data.size_vertices = obj_data.num_vertices * sizeof(float);
  obj_data.num_indices = p_indices - obj_data.indices;
  obj_data.size_indices =  obj_data.num_indices * sizeof(TYPE_INDICES);

  printf(
    "size_vertices: %zu size_indices: %zu num_vertices: %zu num_indices: %zu float: %zu\n",
    obj_data.size_vertices,
    obj_data.size_indices,
    obj_data.num_vertices,
    obj_data.num_indices,
    sizeof(float)
  );

  return obj_data;
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
    "Project Leech",
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

  struct VAO vao = vao_new();
  vao_bind(&vao);

//  struct obj_data obj_obj = obj_load("res/cube.obj");
  struct obj_data obj_obj = obj_load("res/monkey.obj");
//  struct obj_data obj_obj = obj_load("res/septacube.obj");
  vao.num_indices = obj_obj.num_indices;

  GLuint VBO = 0;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(
    GL_ARRAY_BUFFER,
    obj_obj.size_vertices,
    obj_obj.vertices,
    GL_STATIC_DRAW
  );

  GLuint EBO = 0;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    obj_obj.size_indices,
    obj_obj.indices,
    GL_STATIC_DRAW
  );

  free(obj_obj.vertices);
  free(obj_obj.indices);

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

  float speed_pitch_camera = speed_camera/2;
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
      obj_pos_strafe(id_camera_current, speed_camera);
    }
    if (key_down[GLFW_KEY_S]) {
      obj_pos_strafe(id_camera_current, -speed_camera);
    }

    if (key_down[GLFW_KEY_E]) {
      if (key_down[GLFW_KEY_LEFT_SHIFT]) {
        vec3 dir = {0};
        glm_vec3_copy(obj_dir_look_get(id_camera_current), dir);
        dir[1] = 0.0f;
        glm_vec3_normalize(dir);
        obj_pos_forward(id_camera_current, speed_camera, dir);
      } else {
        obj_pos_forward(id_camera_current, speed_camera, NULL);
      }
    }
    if (key_down[GLFW_KEY_D]) {
      if (key_down[GLFW_KEY_LEFT_SHIFT]) {
        vec3 dir = {0};
        glm_vec3_copy(obj_dir_look_get(id_camera_current), dir);
        dir[1] = 0.0f;
        glm_vec3_normalize(dir);
        obj_pos_forward(id_camera_current, -speed_camera, dir);
      } else {
        obj_pos_forward(id_camera_current, -speed_camera, NULL);
      }
    }

    if (key_down[GLFW_KEY_J]) {
      yaw_camera += -speed_yaw_camera;
    }
    if (key_down[GLFW_KEY_L]) {
      yaw_camera += speed_yaw_camera;
    }
    if (key_down[GLFW_KEY_I]) {
      if (key_down[GLFW_KEY_LEFT_SHIFT]) {
        obj_pos_forward(
          id_camera_current,
          speed_camera,
          (vec3){0.0f, 1.0f, 0.0f}
        );
      } else {
        float pitch_camera_new = pitch_camera + speed_pitch_camera;
        if (pitch_camera_new < M_PI/2-0.1f) {
          pitch_camera = pitch_camera_new;
        }
      }
    }
    if (key_down[GLFW_KEY_K]) {
      if (key_down[GLFW_KEY_LEFT_SHIFT]) {
        obj_pos_forward(
          id_camera_current,
          -speed_camera,
          (vec3){0.0f, 1.0f, 0.0f}
        );
      } else {
        float pitch_camera_new = pitch_camera - speed_pitch_camera;
        if (pitch_camera_new > -M_PI/2+0.1f) {
          pitch_camera = pitch_camera_new;
        }
      }
    }

    obj_dir_look_set(
      id_camera_current,
      (vec3){
        cosf(yaw_camera) * cosf(pitch_camera),
        sinf(pitch_camera),
        sinf(yaw_camera) * cosf(pitch_camera)
      }
    );

    glm_look(
      obj_pos_get(id_camera_current),
      obj_dir_look_get(id_camera_current),
      obj_dir_up_get(id_camera_current),
      m4_view
    );
    glm_mat4_mulN((mat4 *[]){&m4_perspective, &m4_view, &m4_model}, 3, m4_mvp);

    render(&vao, glfwGetTime(), id_program_render, m4_mvp);

    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
