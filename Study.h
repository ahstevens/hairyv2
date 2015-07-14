#pragma once

#include <string>
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
	std::string participant;
	//Trial trial;
	int trialNum;
	float timeStart;

};

