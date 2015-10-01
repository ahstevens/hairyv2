#pragma once
#include "Object.h"
#include <vector>

class Probe :
	public Object
{
public:
	Probe(void);
	~Probe(void);
	
	virtual void redraw();

private:
	std::vector<glm::vec2> circle(int segments);
	void generateProbe();
};

