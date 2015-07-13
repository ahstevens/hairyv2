#pragma once

#include <vector>
#include "Camera.h"
#include "Light.h"
#include "BiMap.h"
#include "Object.h"

class Trial
{
public:
	Trial(void);
	~Trial(void);

private:
	Camera camera;
	Light light;

	BiMap bimap;

	std::vector<Object> objects;
};

