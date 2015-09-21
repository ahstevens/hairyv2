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

uniform vec4 lightPos;

uniform float lengthMult;
uniform float thicknessMult;

uniform bool directionalGeom;
uniform float directionalGeomScale;

uniform float offset;

void main()
{
	vec3 lightU = normalize(cross(vec3(0.f, 1.f, 0.f), lightPos.xyz));
	vec3 lightV = normalize(cross(lightPos.xyz, lightU));

	mat4 squish = mat4(vec4(lightU, 0.f),
					   vec4(lightV, 0.f),
					   vec4(0.f),
					   vec4(0.f, 0.f, 0.f, 1.f));

	vec3 u = normalize(cross(vec3(0.f, 1.f, 0.f), w));
	vec3 v = normalize(cross(w, u));

	// build CFTM for scaling the tubes
	mat4 coordFrameTrans = mat4(vec4(u * (directionalGeom ? directionalGeomScale : thicknessMult), 0.f),
								vec4(v * (directionalGeom ? directionalGeomScale : thicknessMult), 0.f),
								vec4(directionalGeom ? normalize(w) * directionalGeomScale : w * lengthMult, 0.f),
								vec4(0.f, 0.f, 0.f, 1.f));

	coordFrameTrans = squish * coordFrameTrans;

	mat4 trans = mat4(1.f);
	trans[3] = vec4(instanceLocation, 1.f);

	coordFrameTrans = model * trans * coordFrameTrans;
	
	gl_Position = projection * view * coordFrameTrans * vec4(position, 1.0f);
} 