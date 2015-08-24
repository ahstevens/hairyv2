#pragma once

#include <vector>
#include "BiMap.h"
#include "Slice.h"

class Trial
{
public:
	Trial(int xSize = 10, int ySize = 10, float density = 1.0f, float jitter = 0.25f);
	~Trial();

	void init();

	void display( Shader shader );

private:
	int xSize, ySize;

	float jitter, density;

	BiMap* bimap;

	Slice* cp;
};

