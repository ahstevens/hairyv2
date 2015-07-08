#version 330 core
out vec4 color;

in vec3 vcolor;

void main()
{
    color = vec4(vcolor, 1.0f);
}
