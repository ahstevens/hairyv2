#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 instanceLocation;
layout (location = 4) in vec3 w;


out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 col;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 lightPos;

uniform float lengthMult;
uniform float thicknessMult;

uniform bool directionalGeom;
uniform float directionalGeomScale;

uniform float offset;

uniform bool doShadows;

mat4 myShadowMatrix(vec4 ground, vec4 light)
{
    float  dotProd;
    mat4  shadowMat;

    dotProd = dot(ground, light);
    
    shadowMat[0][0] = dotProd - light[0] * ground[0];
    shadowMat[1][0] = 0.0 - light[0] * ground[1];
    shadowMat[2][0] = 0.0 - light[0] * ground[2];
    shadowMat[3][0] = 0.0 - light[0] * ground[3];
    
    shadowMat[0][1] = 0.0 - light[1] * ground[0];
    shadowMat[1][1] = dotProd - light[1] * ground[1];
    shadowMat[2][1] = 0.0 - light[1] * ground[2];
    shadowMat[3][1] = 0.0 - light[1] * ground[3];
    
    shadowMat[0][2] = 0.0 - light[2] * ground[0];
    shadowMat[1][2] = 0.0 - light[2] * ground[1];
    shadowMat[2][2] = dotProd - light[2] * ground[2];
    shadowMat[3][2] = 0.0 - light[2] * ground[3];
    
    shadowMat[0][3] = 0.0 - light[3] * ground[0];
    shadowMat[1][3] = 0.0 - light[3] * ground[1];
    shadowMat[2][3] = 0.0 - light[3] * ground[2];
    shadowMat[3][3] = dotProd - light[3] * ground[3];

    return shadowMat;
}

void main()
{
	vec3 lightU = normalize(cross(vec3(0.f, 1.f, 0.f), lightPos.xyz));
	vec3 lightV = normalize(cross(lightPos.xyz, lightU));
			

	vec3 u = normalize(cross(vec3(0.f, 1.f, 0.f), w));
	vec3 v = normalize(cross(w, u));

	vec4 offsetLoc = vec4(0.f, 0.f, offset, 1.f);

	// build CFTM for scaling the tubes
	mat4 coordFrameTrans = mat4(vec4(u * (directionalGeom ? directionalGeomScale : thicknessMult), 0.f),
								vec4(v * (directionalGeom ? directionalGeomScale : thicknessMult), 0.f),
								vec4(directionalGeom ? normalize(w) * directionalGeomScale : w * lengthMult, 0.f),
								vec4(0.f, 0.f, 0.f, 1.f));
								
	mat4 trans = mat4(1.f);

	if(doShadows)
	{
		mat4 shadow = myShadowMatrix( vec4(0.f, 0.f, 1.f, 0.f), lightPos);

		// Calculate vector from offset point to plane along light vector
		vec4 lightVec = vec4(normalize(offsetLoc.xyz - lightPos.xyz), 0.f);
		float scaleAmount = offset / lightVec.z;
		lightVec = scaleAmount * lightVec;

		trans[3] = vec4(instanceLocation.x + lightVec.x, instanceLocation.y + lightVec.y, instanceLocation.z, 1.f);
		
		coordFrameTrans = trans * shadow * coordFrameTrans;

		col = vec4(vec3(0.5f), 1.f);
	}
	else
	{
		mat4 squish = mat4(1.f);
		squish[2] = vec4(0.f);

		trans[3] = vec4(instanceLocation.x, instanceLocation.y, instanceLocation.z + offset, 1.f);

		coordFrameTrans = trans * squish * coordFrameTrans;

		col = vec4(0.8f, 0.1f, 0.1f, 1.f);
	}	
	
	coordFrameTrans = model * coordFrameTrans;
	
	gl_Position = projection * view * coordFrameTrans * vec4(position, 1.0f);
} 