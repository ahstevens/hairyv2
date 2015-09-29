/* Copyright (c)Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  cware@cisunix.unh.edu
*/

/* *** How to use quaternions for rotations ***
//
// As a rotation tool:
// Create a quaternion, set its angle/axis with set(Degree)AngleVector() 
// Rotate a vector by calling the quaternion's Rotate() method with either
// a Point3f or a Quaternion behaving like a vector. Such a quaternion is
// created by passing x,y,z into the constructer (in effect, leaving the
// angle 0)
//
// Adding rotations: multiplying two rotation quaternions will result in
// a quaternion representing the result of the two rotations.
//
// Interpolation: SLERP method of interpolating rotations still needs 
// to be implemented
*/

#ifndef QUATERNION_H
#define QUATERNION_H

#define PI 3.141592653589793f

//#include "SDV_types.h"

typedef double Point3d[3];


class Quaternion  
{
public:
	Quaternion();
	Quaternion(double X, double Y, double Z);
	Quaternion(double S, double X, double Y, double Z);
	
	void set(double S, double X, double Y, double Z);
	Quaternion operator= (Quaternion quat);
	void setAngleVector(double A, double X, double Y, double Z);
	void setDegreeAngleVector(double A, double X, double Y, double Z);
	
	Quaternion Rotate(Quaternion rotquat);
	void Rotate(Point3d in);
	void Rotate (double &x, double &y, double &z);
	
	Quaternion Conjugate();
	Quaternion operator* (Quaternion quat);
	Quaternion operator+ (Quaternion quat);
	
	double getDegreeAngle();
	double getVectorX();
	double getVectorY();
	double getVectorZ();
	double getAngle();
	
	void print();
	
	double s, x, y, z;
};

#endif
