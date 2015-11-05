#pragma once
#include "Object.h"
#include <vector>



class Target : public Object
{
public:

	Target( glm::vec3 color );
	~Target( void );

	void init();

	virtual void redraw();

private:
	void generate();

	std::vector<glm::vec3> instances;
};