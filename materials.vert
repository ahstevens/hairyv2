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

uniform bool directionalGeom;
uniform bool glyphHead;
uniform float directionalGeomScale;

void main()
{
	
	vec3 up = vec3( 0.f, 1.f, 0.f );

	if( length( cross( up, w ) ) < 0.001 )
		up = vec3( 0.f, 0.f, -1.f );

	vec3 u = normalize( cross( up, w ) );

	vec3 v = normalize( cross( w, u ) );

	vec3 w_new, pos;

	if ( directionalGeom )
	{
		u *= directionalGeomScale;
		v *= directionalGeomScale;
		w_new = normalize( w ) * directionalGeomScale;
		if( glyphHead ) pos = instanceLocation + w * lengthMult;
		else pos = instanceLocation;
	}
	else
	{
		u *= thicknessMult;
		v *= thicknessMult;
		w_new = w * lengthMult;
		pos = instanceLocation;
	}
		
	// build CFTM for scaling the tubes
	mat4 coordFrameTrans = mat4(vec4(u, 0.f),
								vec4(v, 0.f),
								vec4(w_new, 0.f),
								vec4(pos, 1.f));
	
	gl_Position = projection * view * model * coordFrameTrans * vec4(position, 1.0f);
	FragPos = vec3( coordFrameTrans * vec4(position, 1.0f));
	Normal = mat3(transpose(inverse( model * coordFrameTrans ))) * normal;
	
	TexCoords = vec2(texCoord.x * length(w) * lengthMult / 10.f, 1.0 - texCoord.y * thicknessMult / 10.f);
} 