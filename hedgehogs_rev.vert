#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 instanceLocation;
layout (location = 4) in vec3 w;

#define M_PI 3.1415926535f

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 col;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct Light {
	vec4 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

uniform float lengthMult;
uniform float thicknessMult;

uniform bool directionalGeom;
uniform float directionalGeomScale;

uniform float offset;

uniform bool doShadows;

// Adapted from OpenGL Red Book Ch. 14, pg. 583-584
mat4 makeShadowMatrix(vec4 plane, vec4 L)
{
    float  dist;
    mat4  shadowMat;

	// plane is given as a normal vector (to the plane), so distance is simply
	// the dot product of the plane normal with the light vector
	// distance = ( light.x * plane.normal.x ) + 
	//            ( light.y * plane.normal.y ) + 
	//            ( light.z * plane.normal.z ) + 
	//            ( light.w * plane.distance )
    dist = dot( L, plane );

	float plane_normal_x = plane[ 0 ];
	float plane_normal_y = plane[ 1 ];
	float plane_normal_z = plane[ 2 ];
	float plane_distance = plane[ 3 ];
    
	// The following is a cleaned-up version of the matrix presented on pg. 584
	// of the OpenGL Red Book, 3rd Ed.

	// Column 1
    shadowMat[ 0 ].x = dist - L.x * plane_normal_x;
    shadowMat[ 0 ].y =      - L.y * plane_normal_x;
    shadowMat[ 0 ].z =      - L.z * plane_normal_x;
    shadowMat[ 0 ].w =      - L.w * plane_normal_x;

	// Column 2
    shadowMat[ 1 ].x =      - L.x * plane_normal_y;
    shadowMat[ 1 ].y = dist - L.y * plane_normal_y;
    shadowMat[ 1 ].z =      - L.z * plane_normal_y;
    shadowMat[ 1 ].w =      - L.w * plane_normal_y;

	// Column 3
    shadowMat[ 2 ].x =      - L.x * plane_normal_z;
    shadowMat[ 2 ].y =      - L.y * plane_normal_z;
    shadowMat[ 2 ].z = dist - L.z * plane_normal_z;
    shadowMat[ 2 ].w =      - L.w * plane_normal_z;

	// Column 4
    shadowMat[ 3 ].x =      - L.x * plane_distance;    
    shadowMat[ 3 ].y =      - L.y * plane_distance;    
    shadowMat[ 3 ].z =      - L.z * plane_distance;    
    shadowMat[ 3 ].w = dist - L.w * plane_distance;

    return shadowMat;
}

void main()
{		
	vec3 w_towards;
	bool glyphHeadAtPlane;

	// reverse vector if facing away from viewer
	if( w.z < 0.f )
	{
		w_towards = -w;
		glyphHeadAtPlane = true;
	}
	else
	{
		w_towards = w;
		glyphHeadAtPlane = false;
	}

	vec3 up = ( length( cross( vec3( 0.f, 1.f, 0.f ), w_towards ) ) < 0.001 ) ? vec3( 0.f, 0.f, -1.f ) : vec3( 0.f, 1.f, 0.f );

	vec3 u = normalize( cross( up, w_towards ) );

	vec3 v = normalize( cross( w_towards, u ) );

	vec3 planeNormal = vec3(0.f, 0.f, 1.f);

	float glyphOffset;
	float dp = dot( planeNormal, normalize( w_towards ) );
	
	// offset glyph to anchor tip of 3D shape to the plane
	if( glyphHeadAtPlane )
		glyphOffset = directionalGeomScale;
	else
		glyphOffset = sqrt( 1 - pow( dp, 2 ) ) * ( thicknessMult / 2 );
				
	vec3 w_new, pos;

	if ( directionalGeom )
	{
		u *= directionalGeomScale;
		v *= directionalGeomScale;
		w_new = normalize( w ) * directionalGeomScale;

		if( glyphHeadAtPlane ) 
			pos = instanceLocation + vec3( 0.f, 0.f, 1.f ) * glyphOffset;
		else 
			pos = instanceLocation + w_towards * lengthMult + normalize( w_towards ) * directionalGeomScale + vec3( 0.f, 0.f, 1.f ) * glyphOffset;
	}
	else
	{
		u *= thicknessMult;
		v *= thicknessMult;		
		w_new = w * lengthMult + normalize( w ) * directionalGeomScale;

		if( glyphHeadAtPlane )
			pos = instanceLocation + vec3( 0.f, 0.f, 1.f ) * glyphOffset - w_new;
		else
			pos = instanceLocation + vec3( 0.f, 0.f, 1.f ) * glyphOffset;
	}

	if( !glyphHeadAtPlane )
	{ 
		float alpha = acos( dp );
		float beta = M_PI / 2.f - alpha;

		float tipToPlane = sin( beta ) * length( w * lengthMult + normalize( w ) * directionalGeomScale );

		float delta = directionalGeomScale - tipToPlane - glyphOffset;

		if( delta > 0 ) pos += vec3( 0.f, 0.f, 1.f ) * delta;
	}
		
	// build CFTM for scaling the tubes
	mat4 coordFrameTrans = mat4(vec4(u, 0.f),
								vec4(v, 0.f),
								vec4(w_new, 0.f),
								vec4(pos, 1.f));

	if(doShadows)
	{
		mat4 shadow = makeShadowMatrix(vec4(0.f, 0.f, 1.f, offset), light.position);

		coordFrameTrans = shadow * coordFrameTrans;

		col = vec4(vec3(0.66f), 1.f); // light grey
	}
	else
		col = vec4(vec3(0.f), 1.f); // black	
	
	coordFrameTrans = model * coordFrameTrans;
	
	gl_Position = projection * view * coordFrameTrans * vec4(position, 1.0f);
} 