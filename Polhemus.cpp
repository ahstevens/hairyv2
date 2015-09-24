#include "Polhemus.h"
#include <iostream>


Polhemus::Polhemus(std::string trackerName, std::string serverLocation)
{
	const char* device = (trackerName + "@" + serverLocation).c_str();
	vrpnTracker = new vrpn_Tracker_Remote(device);
	vrpnTracker->register_change_handler( 0, (vrpn_TRACKERCHANGEHANDLER) this->handle_tracker );
}


Polhemus::~Polhemus(void)
{
	delete vrpnTracker;
}

void Polhemus::printInfo()
{
	vrpnTracker->mainloop();
}

void VRPN_CALLBACK Polhemus::handle_tracker( void* userData, const vrpn_TRACKERCB t )
{	
	std::cout << "Sensor '" << t.sensor << "': Position = (" << t.pos[0] << ", " <<  t.pos[1] << ", " << t.pos[2] << "); Quaternion = (" << t.quat[0] << ", " << t.quat[1] << ", " << t.quat[2] << ", " << t.quat[3] << ")" << std::endl;
}