#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const GLchar * src_shader_vertex = \
"#version 450 core\n"
"void main(void)\n"
"{\n"
"	const vec4 vertices[3] = vec4[3] (\n"
"	 vec4( 0.25, -0.25, 0.5, 1.0),\n"
"	 vec4(-0.25, -0.25, 0.5, 1.0),\n"
"	 vec4( 0.25,  0.25, 0.5, 1.0)\n"
"	);\n"
"	gl_Position = vertices[gl_VertexID];\n"
"}\n";

const GLchar * src_shader_fragment = \
"#version 450 core\n"
"out vec4 color;\n"
"void main(void)\n"
"{\n"
"	color = vec4(0.0f, 0.8f, 1.0f, 1.0f);\n"
"}\n";

GLuint
shaders_compile(void)
{
	GLint success = GL_FALSE;

	// Create and compile vertex shader.
	GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_vertex, 1, &src_shader_vertex, NULL);
	glCompileShader(shader_vertex);
	glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		fprintf(stderr, "Vertex shader failed to compile.\n");
	}

	// Create and compile fragment shader.
	GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_fragment, 1, &src_shader_fragment, NULL);
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

	// Delete the shaders as the program has them now.
	glDeleteShader(shader_fragment);
	glDeleteShader(shader_vertex);

	return program;
}

void
render(double time_current, GLuint program)
{
	GLfloat color[] = {
		sinf(time_current) * 0.5f + 0.5f,
		cosf(time_current) * 0.5f + 0.5f,
		0.0f,
		1.0f,
	};
	glClearBufferfv(GL_COLOR, 0, color);
	glUseProgram(program);
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

	GLuint program_render = shaders_compile();
	GLuint VAO = 0;
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		render(glfwGetTime(), program_render);
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
