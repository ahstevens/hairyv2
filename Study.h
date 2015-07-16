#pragma once

#include <string>
#include "Camera.h"
#include "Light.h"
#include "Trial.h"

class Study
{
public:
	Study(void);
	~Study(void);

	void begin();
	void next();
	void end();

private:
	Camera camera;
	Light light;

	std::string participant;
	//Trial trial;
	int trialNum;
	float timeStart;

};

