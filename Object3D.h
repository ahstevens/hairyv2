/**
 * Object3D.h - an abstract class representing an OpenGL graphical object 
 *
 * rdb 
 * 09/23/13
 *
 * Based on code created by Can Xiong, Fall 2012
 *   09/07/14 rdb include gl770.h
 * 
 */
#ifndef OBJECT3D_H_
#define OBJECT3D_H_

#include <vector>
#include <glm/glm.hpp>

class Object3D
{
public:
    Object3D();
    virtual ~Object3D();
    
    void setLocation( float x, float y, float z ); // set location
	void setLocation( glm::vec3 location );
    void setSize( float xs, float ys, float zs );   // set object size
	void setSize( glm::vec3 size );
    void setRotate( float angle, float dx, float dy, float dz ); // set rotate
	void setRotate( float angle, glm::vec3 axis );
    
    float getX();                          // return x location
    float getY();                          // return y location
    float getZ();                          // return z location
    glm::vec3 getLocation();               // return location as a vec3
    
    virtual void redraw() = 0;
        
protected:
    glm::vec3 location;					   // location (origin) of the object
	glm::vec3 size;						   // size of the object

    float angle;						   // rotation angle and axis
	glm::vec3 axis;
};

#endif /*OBJECT3D_H_*/

