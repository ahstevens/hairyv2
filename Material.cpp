#include "Material.h"

#define MAT_AR  0.5f
#define MAT_SHN 10.0f

using namespace glm;

Material::Material() : ambientRatio(MAT_AR), ambient(vec3(1.0, 1.0, 1.0)), diffuse(vec3(1.0, 1.0, 1.0)), specular(vec3(0.5, 0.5, 0.5)), shininess(MAT_SHN)
{
}

Material::Material(vec3 color) : ambientRatio(MAT_AR), shininess(MAT_SHN)
{
	setColor(color);
}

Material::~Material()
{
}

void Material::setColor(float r, float g, float b)
{
	vec3 color = vec3(r, g, b);
	ambient = ambientRatio * color;
	diffuse = color;
	specular = vec3(0.5, 0.5, 0.5);
}

void Material::setColor(glm::vec3 color)
{
	ambient = ambientRatio * color;
	diffuse = color;
	specular = vec3(0.5, 0.5, 0.5);
}

glm::vec3 Material::getColor()
{
	return diffuse;
}

void Material::setAmbientRatio(float r)
{
	ambientRatio = r;
}

float Material::getAmbientRatio()
{
	return ambientRatio;
}

void Material::setAmbientColor(float r, float g, float b)
{
	ambient = vec3(r, g, b);
}

void Material::setAmbientColor(glm::vec3 color)
{
	ambient = color;
}

glm::vec3 Material::getAmbientColor()
{
	return ambient;
}


void Material::setDiffuseColor(float r, float g, float b)
{
	diffuse = vec3(r, g, b);
}

void Material::setDiffuseColor(glm::vec3 color)
{
	diffuse = color;
}

glm::vec3 Material::getDiffuseColor()
{
	return diffuse;
}

void Material::setSpecularColor(float r, float g, float b)
{
	specular = vec3(r, g, b);
}

void Material::setSpecularColor(glm::vec3 color)
{
	specular = color;
}

glm::vec3 Material::getSpecularColor()
{
	return specular;
}

void Material::setShininess(float shininess)
{
	this->shininess = shininess;
}

float Material::getShininess()
{
	return shininess;
}