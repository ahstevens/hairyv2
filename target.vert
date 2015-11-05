#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 instanceLocation;
layout (location = 4) in vec3 w;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	
	vec3 up = vec3( 0.f, 1.f, 0.f );

	if( length( cross( up, w ) ) < 0.001 )
		up = vec3( 0.f, 0.f, -1.f );

	vec3 u = normalize( cross( up, w ) );

	vec3 v = normalize( cross( w, u ) );
		
	// build CFTM for scaling the tubes
	mat4 coordFrameTrans = mat4(vec4(u, 0.f),
								vec4(v, 0.f),
								vec4(w, 0.f),
								vec4(instanceLocation, 1.f));
	
	gl_Position = projection * view * model * coordFrameTrans * vec4(position, 1.0f);
} 