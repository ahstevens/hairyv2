#pragma once

#include <vector>
#include "Camera.h"
#include "Light.h"
#include "BiMap.h"
#include "SweepSurface.h"

struct gridpoint_t {
	float x, y, x_jitter, y_jitter, dx, dy, dz;
};

class Trial
{
public:
	Trial(int xsz, int ysz, float jitter);
	~Trial();

	void init(float jitter);

	std::vector<SweepSurface> getObjects();

private:	
	std::vector<glm::vec2> circle(int segments);

	int xSize, ySize;

	Camera camera;
	Light light;

	BiMap bimap;

	std::vector< std::vector< gridpoint_t > > grid;

	std::vector<SweepSurface> objects;
};

