#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 instanceLocation;
layout (location = 4) in vec3 w;


out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float lengthMult;
uniform float thicknessMult;

void main()
{
	vec3 u = normalize(cross(vec3(0.f, 1.f, 0.f), w));
	vec3 v = normalize(cross(w, u));

	// build coordinate frame transformation matrix (CFTM) at seed point
	mat4 coordFrameTransNorm = mat4(vec4(u, 0.f),
									vec4(v, 0.f),
									vec4(normalize(w), 0.f),
									vec4(instanceLocation, 1.f));
		
	// build CFTM for scaling the tubes
	mat4 coordFrameTransScaled = mat4(vec4(u * thicknessMult, 0.f),
									  vec4(v * thicknessMult, 0.f),
									  vec4(w * lengthMult, 0.f),
									  vec4(instanceLocation, 1.f));

	gl_Position = projection * view * model * coordFrameTransScaled * vec4(position, 1.0f);
	FragPos = vec3(model * coordFrameTransScaled * vec4(position, 1.0f));
	Normal = mat3(transpose(inverse(model))) * mat3(coordFrameTransNorm) * normal;
	
	TexCoords = vec2(texCoord.x, 1.0 - texCoord.y);
} 