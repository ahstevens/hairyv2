#include "Polhemus.h"
#include <iostream>

#include <glm\gtx\quaternion.hpp>

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

// Initialize class variables
Polhemus* Polhemus::instance = NULL;

// Returns singleton Polhemus instance
Polhemus* Polhemus::getInstance( )
{
    if ( !instance )
        instance = new Polhemus( "Tracker0", "192.168.8.149" );//"localhost" );
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
	glm::quat temp_quat;
	temp_quat.x = t.quat[0];
	temp_quat.y = t.quat[1];
	temp_quat.z = t.quat[2];
	temp_quat.w = t.quat[3];
	pos.x = t.pos[0];
	pos.y = t.pos[1];
	pos.z = t.pos[2];

	quat = calibration * temp_quat;
	orientation = glm::toMat4( quat );
}

void Polhemus::update()
{
	vrpnTracker->mainloop();
}

void Polhemus::printInfo()
{
	glm::vec3 axis = getAxis();
	std::cout << "Position = (" << pos.x << ", " <<  pos.y << ", " << pos.z << ")" << std::endl;
	std::cout << "Quaternion = (" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << ")" << std::endl;
	std::cout << "Vector = (" << axis.x << ", " << axis.y << ", " << axis.z << ")" << std::endl;	
	std::cout << "Rotation = "<< getRotationDegrees() << " degrees, " << getRotation() << " radians" << std::endl;
	std::cout << std::endl;
}

glm::vec3 Polhemus::getPosition()
{
	return pos;
}

glm::vec3 Polhemus::getAxis()
{
	return glm::axis( quat );
}

float Polhemus::getRotation()
{
	return glm::angle( quat );
}

float Polhemus::getRotationDegrees()
{
	return glm::angle( quat ) * 180.f / M_PI;
}

glm::quat Polhemus::getQuaternion()
{
	return quat;
}

// the following function assumes that the probe is aligned along its origin
// (i.e., points down the +x axis with the top of the probe pointing along +z)
void Polhemus::calibrate()
{
	calibrateReset();

	// get the raw orientation from the Polhemus into quat variable
	this->update();

	// the calibration is the inverse of the current orientation, since we
	// would expect an aligned probe to return the identity quaternion
	calibration = glm::inverse(quat);

	// now recalculate current position using new calibration
	this->update();
}

void Polhemus::calibrateReset()
{
	// reset the calibration to the identity quaternion
	calibration = glm::quat();
}

void Polhemus::calibrateRollUp( float degree )
{
	this->calibration *= glm::angleAxis( glm::radians( degree ), glm::vec3( 1.f, 0.f, 0.f ) );
}

void Polhemus::calibrateRollDown( float degree )
{
	this->calibration *= glm::angleAxis( -glm::radians( degree ), glm::vec3(1.f, 0.f, 0.f));
}

void Polhemus::calibratePitchUp( float degree )
{
	this->calibration *= glm::angleAxis( glm::radians( degree ), glm::vec3( 0.f, -1.f, 0.f ) );
}

void Polhemus::calibratePitchDown( float degree )
{
	this->calibration *= glm::angleAxis( -glm::radians( degree ), glm::vec3( 0.f, -1.f, 0.f ) );
}

void Polhemus::calibrateYawUp( float degree )
{
	this->calibration *= glm::angleAxis( glm::radians( degree ), glm::vec3( 0.f, 0.f, 1.f ) );
}

void Polhemus::calibrateYawDown( float degree )
{
	this->calibration *= glm::angleAxis( -glm::radians( degree ), glm::vec3( 0.f, 0.f, 1.f ) );
}