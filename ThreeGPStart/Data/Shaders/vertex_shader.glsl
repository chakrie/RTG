#version 330

layout(location = 0) in vec3 vertex_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 combined;

void main()
{
    gl_Position = projection * view * model * vec4(vertex_position, 1.0);
}