#pragma once
#include "Object.h"
#include "Trial.h"
#include <vector>



class Target : public Object
{
public:

	Target( glm::vec3 color );
	~Target( void );

	void init();

	void setSeed( Trial::Seed seed );
	Trial::Seed getSeed();

	glm::vec3 getFlowVector();

	virtual void redraw();

private:
	void generate();

	Trial::Seed seed;

	std::vector<glm::vec3> instances;
};