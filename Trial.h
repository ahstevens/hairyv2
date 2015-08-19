#pragma once

#include <vector>
#include "BiMap.h"
#include "SweepSurface.h"

struct gridpoint_t {
	float x_jitter, y_jitter, dx, dy, dz;
};

class Trial
{
public:
	Trial(int xSize, int ySize, float density, float jitter);
	~Trial();

	void init();

	std::vector<SweepSurface> getObjects();

private:	
	std::vector<glm::vec2> circle(int segments);

	int xSize, ySize;

	float jitter, density;

	BiMap* bimap;

	std::vector< std::vector< gridpoint_t > > grid;

	std::vector<SweepSurface> objects;
};

