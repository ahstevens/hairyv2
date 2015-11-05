#pragma once
#include "Object.h"
#include <vector>



class Probe : public Object
{
public:

	Probe( float length, float width );
	~Probe( void );

	void init();

	virtual void redraw();

private:
	void generate();

	std::vector<glm::vec3> instances;

	GLsizei directionalIndicesCount;      // number of indices used to render the directionality geometry	

	float length, width;
};