#pragma once
#include "vrpn/vrpn_Tracker.h"
#include <string>

class Polhemus
{
public:
	Polhemus(std::string trackerName, std::string serverLocation);
	~Polhemus(void);

	void printInfo();

private:
	void VRPN_CALLBACK handle_tracker( void* userData, const vrpn_TRACKERCB t );

	vrpn_Tracker_Remote* vrpnTracker;
};

