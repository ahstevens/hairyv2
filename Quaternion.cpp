/* Copyright (c)Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  cware@ccom.unh.edu 
*/

#include "Quaternion.h"
#include<math.h>
#include<iostream>

using namespace std;

Quaternion::Quaternion()
{
	Quaternion(0,0,0,0);
}

Quaternion::Quaternion(double S, double X, double Y, double Z)
{
	s = S;
	x = X;
	y = Y;
	z = Z;
}

Quaternion::Quaternion(double X, double Y, double Z)
{
	Quaternion(0,X,Y,Z);
}


void Quaternion::setAngleVector(double A, double X, double Y, double Z)
{
	s = cos(A/2.0);
	double temp = sin(A/2);
	x = X*temp;
	y = Y*temp;
	z = Z*temp;
}

void Quaternion::setDegreeAngleVector(double A, double X, double Y, double Z)
{
	A = (A*PI)/180;
	s = cos(A/2.0);
	double temp = sin(A/2);
	x = X*temp;
	y = Y*temp;
	z = Z*temp;
}


Quaternion Quaternion::operator=(Quaternion quat)
{
	this->s = quat.s;
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;

	return *this;
}

void Quaternion::set(double S, double X, double Y, double Z)
{
	s = S;
	x = X;
	y = Y;
	z = Z;
}

void Quaternion::print()
{
	cout << "[" << s << ",[" << x << "," << y << "," << z << "]] [" << getAngle() << ",[" << getVectorX() << "," << getVectorY() << "," << getVectorZ() << "]]\n";
	cout.flush();
}

double Quaternion::getAngle()
{
	return acos(s)*2.0;
}

double Quaternion::getVectorX()
{
	return x/(asin(s)*2.0);
}

double Quaternion::getVectorY()
{
	return y/(asin(s)*2.0);
}

double Quaternion::getVectorZ()
{
	return z/(asin(s)*2.0);
}

double Quaternion::getDegreeAngle()
{
	return (getAngle()/PI)*180.0;
}

Quaternion Quaternion::operator +(Quaternion quat)
{
	Quaternion retquat;
	retquat.s = this->s+quat.s;
	retquat.x = this->x+quat.x;
	retquat.y = this->y+quat.y;
	retquat.z = this->z+quat.z;
	return retquat;
}

Quaternion Quaternion::operator *(Quaternion quat)
{
	Quaternion retquat;

	retquat.s = this->s*quat.s-this->x*quat.x-this->y*quat.y-this->z*quat.z;
	retquat.x = this->s*quat.x+quat.s*this->x+this->y*quat.z-quat.y*this->z;
	retquat.y = this->s*quat.y+quat.s*this->y+this->z*quat.x-quat.z*this->x;
	retquat.z = this->s*quat.z+quat.s*this->z+this->x*quat.y-quat.x*this->y;

	return retquat;
}

Quaternion Quaternion::Conjugate()
{
	Quaternion retquat;

	retquat.s = s;
	retquat.x = -x;
	retquat.y = -y;
	retquat.z = -z;

	return retquat;
}

Quaternion Quaternion::Rotate(Quaternion rotquat)
{
	Quaternion retquat;

	//retquat = rotquat * *this * rotquat.Conjugate();
	retquat = *this * rotquat * this->Conjugate();

	return retquat;
}

void Quaternion::Rotate(Point3d in)
{
	Quaternion tmpquat;
	
	tmpquat.set(0,in[0],in[1],in[2]);
	tmpquat = Rotate(tmpquat);
	in[0] = (float)tmpquat.x;
	in[1] = (float)tmpquat.y;
	in[2] = (float)tmpquat.z;
}

void Quaternion::Rotate (double &x, double &y, double &z)
{
	Quaternion tmpquat;
	
	tmpquat.set(0,x,y,z);
	tmpquat = Rotate(tmpquat);
	x = (float)tmpquat.x;
	y = (float)tmpquat.y;
	z = (float)tmpquat.z;
}

