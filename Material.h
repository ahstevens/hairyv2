#pragma once

#include <glm/glm.hpp>

class Material
{
public:
	Material();
	Material(glm::vec3 color);
	~Material();

	void setColor(float r, float g, float b);
	void setColor(glm::vec3 color);
	glm::vec3 getColor();

	void setAmbientRatio(float r);
	float getAmbientRatio();

	void setAmbientColor(float r, float g, float b);
	void setAmbientColor(glm::vec3 color);
	glm::vec3 getAmbientColor();

	void setDiffuseColor(float r, float g, float b);
	void setDiffuseColor(glm::vec3 color);
	glm::vec3 getDiffuseColor();

	void setSpecularColor(float r, float g, float b);
	void setSpecularColor(glm::vec3 color);
	glm::vec3 getSpecularColor();

	void setShininess(float shininess);
	float getShininess();



private:
	float ambientRatio, shininess;
	glm::vec3 ambient, diffuse,	specular;
};

