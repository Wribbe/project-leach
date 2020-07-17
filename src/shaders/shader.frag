#version 450 core
out vec4 color;

uniform vec3 u_color;

void main(void)
{
	color = vec4(u_color, 1.0f);
};
