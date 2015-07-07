/**
 * Object3D.cpp - an abstract class representing an OpenGL graphical object
 *
 * Can Xiong
 * Sep 17, 2012
 *
 * 09/25/13 rdb: added some Color methods, other minor changes
 */
#include "Object.h"

using namespace glm;

//------------------ Constructors ------------------------------------
/**
 * Create a new object3D at position 0,0,0 of size 1,1,1
 */
Object3D::Object3D()
{
	glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    setLocation( 0, 0, 0 );
    setSize( 1, 1, 1 );
    setRotate( 0, 0, 0, 0);
}

//------------------ Destructor ------------------------------------
Object3D::~Object3D()
{
}

//<<<<<<<<<<<<<<<<<<<<<<<<<< public methods >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//---------------------- setLocation ------------------------------
/**
 * set the location of the object to the x,y,z position defined by the args
 */
void Object3D::setLocation( float x, float y, float z )
{
    location = vec3( x, y, z );
}

/**
 * set the location of the object to the x,y,z position defined by the arg
 */
void Object3D::setLocation( vec3 location )
{
    this->location = location;
}

//------------------ getX, getY, getZ -----------------------------
/**
 * return the value of the x origin of the shape
 */
float Object3D::getX()
{
    return location.x;
}
/**
 * return the value of the y origin of the shape
 */
float Object3D::getY()
{
    return location.y;
}
/**
 * return the value of the z origin of the shape
 */
float Object3D::getZ()
{
    return location.z;
}

//------------------ getLocation ----------------------------------------
/**
 * return the location as a Point3 object
 */
vec3 Object3D::getLocation()
{
    return location;
}

//------------------ setSize ----------------------------------------
/**
 * set the size of the shape to be scaled by xs, ys, zs
 *    That is, the shape has an internal fixed size, the shape parameters
 *    scale that internal size.
 */
void Object3D::setSize( float xs, float ys, float zs )
{
    size = vec3( xs, ys, zs );
}

void Object3D::setSize( vec3 size )
{
    this->size = size;
}

//------------------ setRotate ---------------------------------------
/**
 * set the rotation parameters: angle, and axis specification
 */
void Object3D::setRotate( float angle, float dx, float dy, float dz )
{
    this->angle = angle;
    axis = vec3( dx, dy, dz );
}

void Object3D::setRotate( float angle, vec3 axis )
{
    this->angle = angle;
    this->axis = axis;
}

mat4 Object3D::getModelMatrix() { return model; }
