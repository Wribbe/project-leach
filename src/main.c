#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

const char * src_shader_vertex = \
"#version 450 core"
"void main(void)"
"{"
"	gl_Position = vec4(0.0f, 0.0f, 0.5f, 1.0f);"
"}";

const char * src_shader_fragment = \
"#version 450 core"
"out vec4 color;"
"void main(void)"
"{"
"	color = vec4(0.0f, 0.8f, 1.0f, 1.0f);"
"}";

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

	GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	while (!glfwWindowShouldClose(window))
	{
		glClearBufferfv(GL_COLOR, 0, red);
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
