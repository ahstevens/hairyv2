#version 330 core

out vec4 color;
  
uniform vec3 col;
uniform float opacity = 1.f;

void main()
{
	color = vec4( col, opacity );	
} 