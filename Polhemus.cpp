#include "Polhemus.h"
#include <iostream>

// Initialize class variables
Polhemus* Polhemus::instance = NULL;

// Returns singleton Polhemus instance
Polhemus* Polhemus::getInstance( )
{
    if ( !instance )
        instance = new Polhemus( "Tracker0", "localhost" );
    return instance;
}

Polhemus::Polhemus(std::string trackerName, std::string serverLocation)
{
	vrpnTracker = new vrpn_Tracker_Remote( std::string( trackerName + "@" + serverLocation ).c_str() );
	vrpnTracker->register_change_handler( 0, handle_tracker_callback );
}

Polhemus::~Polhemus(void)
{
	delete vrpnTracker;
}

void VRPN_CALLBACK Polhemus::handle_tracker_callback( void* userData, const vrpn_TRACKERCB t )
{
	getInstance()->handle_tracker( userData, t );
}

void VRPN_CALLBACK Polhemus::handle_tracker( void* userData, const vrpn_TRACKERCB t )
{	
	quat.set(t.quat[0], t.quat[1], t.quat[2], t.quat[3]);
	pos.x = t.pos[0];
	pos.y = t.pos[1];
	pos.z = t.pos[2];
}

void Polhemus::update()
{
	vrpnTracker->mainloop();
}

void Polhemus::printInfo()
{
	std::cout << "Position = (" << pos.x << ", " <<  pos.y << ", " << pos.z << ")" << std::endl;
	std::cout << "Quaternion = (" << quat.s << ", " << quat.x << ", " << quat.y << ", " << quat.z << ")" << std::endl;
	std::cout << "Vector = (" << quat.getVectorX() << ", " << quat.getVectorY() << ", " << quat.getVectorZ() << ")" << std::endl;	
	std::cout << "Rotation = "<< quat.getDegreeAngle() << " degrees" << std::endl;
}

glm::vec3 Polhemus::getPosition()
{
	return pos;
}

glm::vec3 Polhemus::getVector()
{
	return glm::vec3(quat.getVectorX(), quat.getVectorY(), quat.getVectorZ());
}

float Polhemus::getRotation()
{
	return quat.getDegreeAngle();
}