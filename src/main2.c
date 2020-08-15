#include "lib/utils.h"

GLFWwindow *
window_get(float * WINDOW_ASPECT)
{
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  int WINDOW_WIDTH = 1920;
  int WINDOW_HEIGHT = 1080;
  *WINDOW_ASPECT = (float)WINDOW_HEIGHT/(float)WINDOW_WIDTH;

  GLFWwindow * window = glfwCreateWindow(
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    "Project Leech",
    NULL,
    NULL
  );

  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);
  return window;
}

void
process_events()
{
}

void
render_objects(float window_aspect)
{
  glClear(GL_COLOR_BUFFER_BIT);
  UNUSED(window_aspect);
}

GLuint
setup_objects()
{
  // Create basic program.
  GLuint id_program = program_create(
    "src/shaders/shader.vert",
    "src/shaders/shader.frag"
  );

  // Create VAO from cube.
  GLuint id_vao = vao_create("res/cube.obj");

  // Setup camera object.
  GLuint id_camera = obj_create(id_program, id_vao);

  return id_camera;
}

int
main(void)
{
  float window_aspect = 0;
  GLFWwindow * window = window_get(&window_aspect);
  GLuint id_camera = setup_objects();
  UNUSED(id_camera);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    process_events();
    render_objects(window_aspect);
    glfwSwapBuffers(window);
  }
}
