#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 instanceLocation;
layout (location = 4) in vec3 w;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float lengthMult;
uniform float thicknessMult;

uniform bool directionalGeom;
uniform float directionalGeomScale;
uniform float haloSize;

void main()
{
	vec3 u = normalize(cross(vec3(0.f, 1.f, 0.f), w));
	vec3 v = normalize(cross(w, u));

	// build CFTM for scaling the tubes
	mat4 coordFrameTrans = model * mat4(vec4(u * (directionalGeom ? directionalGeomScale + haloSize : thicknessMult + haloSize * 2), 0.f),
										vec4(v * (directionalGeom ? directionalGeomScale + haloSize : thicknessMult + haloSize * 2), 0.f),
										vec4(normalize(w) * ((directionalGeom ? directionalGeomScale : length(w) * lengthMult) + haloSize), 0.f),
										vec4(instanceLocation, 1.f));
	
	gl_Position = projection * view * coordFrameTrans * vec4(position, 1.0f);
} 