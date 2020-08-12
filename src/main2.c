#include <stdlib.h>
#include <stdio.h>

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void)x

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
render_objects()
{
}

void
setup_objects(float window_aspect)
{
  UNUSED(window_aspect);
}

int
main(void)
{
  float window_aspect = 0;
  GLFWwindow * window = window_get(&window_aspect);
  setup_objects(window_aspect);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    process_events();
    render_objects();
    glfwSwapBuffers(window);
  }
}
