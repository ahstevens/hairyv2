#pragma once
#include "vrpn/vrpn_Tracker.h"
#include "Quaternion.h"
#include "glm/glm.hpp"
#include <string>

class Polhemus
{
public:
	static Polhemus* getInstance( );
	Polhemus(std::string trackerName, std::string serverLocation);
	~Polhemus(void);

	static void VRPN_CALLBACK handle_tracker_callback( void* userData, const vrpn_TRACKERCB t );
	void VRPN_CALLBACK handle_tracker( void* userData, const vrpn_TRACKERCB t );

	void update();

	void printInfo();

	glm::vec3 getPosition();
	glm::vec3 getVector();

private:
	static Polhemus* instance;
	vrpn_Tracker_Remote* vrpnTracker;

	Quaternion quat;
	glm::vec3 pos;
};

