#include "Light.h"

using namespace glm;

Light::Light(glm::vec3 position) : ambientRatio(0.2f), ambient(vec3(0.2, 0.2, 0.2)), diffuse(vec3(1.0, 1.0, 1.0)), specular(vec3(0.5, 0.5, 0.5)), type(DIRECTIONAL_LT)
{
	this->position = position;
}

Light::~Light()
{
}

void Light::setPosition( float x, float y, float z )
{
	position = vec3(x, y, z);
}

void Light::setPosition( glm::vec3 loc )
{
	position = loc;
}

glm::vec4 Light::getPosition()
{
	return vec4(position, type == POINT_LT ? 1.0f : 0.0f);
}


void Light::setColor( float r, float g, float b )
{
	vec3 color = vec3( r, g, b );
	ambient = ambientRatio * color;
	diffuse = color;
	specular = vec3( 1.0, 1.0, 1.0 );
}

void Light::setColor( glm::vec3 color )
{
	ambient = ambientRatio * color;
	diffuse = color;
	specular = vec3( 1.0, 1.0, 1.0 );
}

glm::vec3 Light::getColor()
{ 
	return diffuse; 
}

void Light::setAmbientRatio( float r )
{
	ambientRatio = r;
}

float Light::getAmbientRatio()
{
	return ambientRatio;
}

void Light::setAmbientColor( float r, float g, float b )
{
	ambient = vec3( r, g, b );
}

void Light::setAmbientColor( glm::vec3 color )
{
	ambient = color;
}

glm::vec3 Light::getAmbientColor()
{ 
	return ambient; 
}


void Light::setDiffuseColor( float r, float g, float b )
{
	diffuse = vec3( r, g, b );
}

void Light::setDiffuseColor( glm::vec3 color )
{
	diffuse = color;
}

glm::vec3 Light::getDiffuseColor()
{
	return diffuse;
}


void Light::setSpecularColor( float r, float g, float b )
{
	specular = vec3( r, g, b );
}

void Light::setSpecularColor( glm::vec3 color )
{
	specular = color;
}

glm::vec3 Light::getSpecularColor()
{
	return specular;
}


void Light::setType( LIGHT_TYPE type )
{
	this->type = type;
}

LIGHT_TYPE Light::getType()
{
	return type;
}
