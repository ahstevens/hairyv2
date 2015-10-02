#pragma once
#include "Object.h"
#include <vector>

class Probe :
	public Object
{
public:
	Probe(void);
	~Probe(void);

	void setOrientation(glm::vec3 orientation);
	
	virtual void redraw();

private:
	std::vector<glm::vec2> circle(int segments);
	void generateProbe();
};

