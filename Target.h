#pragma once
#include "Object.h"
#include "Slice.h"
#include <vector>



class Target : public Object
{
public:

	Target( glm::vec3 color );
	~Target( void );

	void init();

	void setSeed( Slice::Seed seed );
	Slice::Seed getSeed();

	virtual void redraw();

private:
	void generate();

	Slice::Seed seed;

	std::vector<glm::vec3> instances;
};