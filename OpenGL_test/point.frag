#version 150 core
in vec4 vertex_color;

//in vec3 Idiff;

//flat in vec4 vertex_color;
out vec4 fragment;

void main()
{
    fragment = vertex_color;

//    fragment = vec4(Idiff, 1.0);
}
