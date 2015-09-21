#version 330 core
struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
	vec4 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 col;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool use_texture;
uniform sampler2D theTexture;

void main()
{

	color = col;

}