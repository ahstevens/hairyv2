#pragma once
#include <vrpn/vrpn_Tracker.h>
#include "Quaternion.h"
#include <glm/glm.hpp>
#include <glm\gtc\quaternion.hpp>
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
	float getRotation();
	float getRotationDegrees();
	glm::mat4 getOrientationMatrix();

private:
	static Polhemus* instance;
	vrpn_Tracker_Remote* vrpnTracker;

	glm::quat quat;
	glm::vec3 pos;
	glm::mat4 orientation;
};

