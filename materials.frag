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
  
out vec4 color;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool use_texture;
uniform sampler2D theTexture;

void main()
{
	vec3 tex = use_texture ? vec3(texture(theTexture, TexCoords)) : vec3(1.0);

    // Ambient
    vec3 ambient = light.ambient * material.ambient * tex;
  	
    // Diffuse 
    vec3 norm = normalize(Normal);

    vec3 lightDir;
	if(light.position.w == 0.0)
		lightDir = normalize(light.position).xyz;
	else
		lightDir = normalize(light.position.xyz - FragPos);

	float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse * tex;
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;
        
    color = vec4(ambient + diffuse + specular, 1.0f);

	//color = texture(theTexture, TexCoords);
	
} 