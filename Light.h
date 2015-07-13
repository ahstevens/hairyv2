#pragma once
#include "glm/glm.hpp"

enum LIGHT_TYPE {
	POINT, DIRECTIONAL
};

class Light
{
public:
	Light();
	~Light();

	void setPosition( float x, float y, float z );
	void setPosition( glm::vec3 loc );
	glm::vec4 getPosition();

	void setColor( float r, float g, float b );
	void setColor( glm::vec3 color );
	glm::vec3 getColor();

	void setAmbientRatio( float r );
	float getAmbientRatio();

	void setAmbientColor( float r, float g, float b );
	void setAmbientColor( glm::vec3 color );
	glm::vec3 getAmbientColor();

	void setDiffuseColor( float r, float g, float b );
	void setDiffuseColor( glm::vec3 color );
	glm::vec3 getDiffuseColor();

	void setSpecularColor( float r, float g, float b );
	void setSpecularColor( glm::vec3 color );
	glm::vec3 getSpecularColor();

	void setType( LIGHT_TYPE type );
	LIGHT_TYPE getType();

private:
	glm::vec3 position,
		ambient,
		diffuse,
		specular;

	float ambientRatio;

	LIGHT_TYPE type;

};

