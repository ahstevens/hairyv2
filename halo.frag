#version 330 core
  
out vec4 color;
  
uniform vec3 haloColor;

void main()
{	
	color = vec4(haloColor, 1.f);
} 