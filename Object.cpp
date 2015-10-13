/**
 * Object3D.cpp - an abstract class representing an OpenGL graphical object
 *
 * Can Xiong
 * Sep 17, 2012
 *
 * 09/25/13 rdb: added some Color methods, other minor changes
 */
#include "Object.h"

#include <glm/gtc/matrix_transform.hpp>

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

using namespace glm;

//------------------ Constructors ------------------------------------
/**
 * Create a new object3D at position 0,0,0 of size 1,1,1
 */
Object::Object()
{
	glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &UBO);

    setPosition( 0.0f, 0.0f, 0.0f );
    setSize( 1.0f, 1.0f, 1.0f );
    orientation = glm::quat();

	use_texture = false;
	tex = NULL;
	shader = NULL;
}

//------------------ Destructor ------------------------------------
Object::~Object()
{
}

//<<<<<<<<<<<<<<<<<<<<<<<<<< public methods >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//---------------------- setLocation ------------------------------
/**
 * set the location of the object to the x,y,z position defined by the args
 */
void Object::setPosition(float x, float y, float z)
{
    position = vec3( x, y, z );
	update_model_matrix = true;
}

/**
 * set the location of the object to the x,y,z position defined by the arg
 */
void Object::setPosition(vec3 location)
{
	this->position = location;
	update_model_matrix = true;
}

//------------------ getLocation ----------------------------------------
/**
 * return the location as a Point3 object
 */
vec3 Object::getPosition()
{
	return position;
}

//------------------ setSize ----------------------------------------
/**
 * set the size of the shape to be scaled by xs, ys, zs
 *    That is, the shape has an internal fixed size, the shape parameters
 *    scale that internal size.
 */
void Object::setSize( float xs, float ys, float zs )
{
    size = vec3( xs, ys, zs );
	update_model_matrix = true;
}

void Object::setSize( vec3 size )
{
    this->size = size;
	update_model_matrix = true;
}

vec3 Object::getSize() { return size; }

//------------------ setRotate ---------------------------------------
/**
 * set the rotation parameters via quaternions
 */
void Object::rotate( float degrees, float x, float y, float z )
{   
    glm::vec3 axis = vec3( x, y, z );
	this->rotate( degrees, axis );
}

void Object::rotate( float degrees, vec3 axis )
{
	float angle = degrees * (float) M_PI / 180.f;
	glm::quat rotQuat = glm::angleAxis( angle, normalize( axis ) );
	orientation = orientation * rotQuat;

	update_model_matrix = true;
}

void Object::setRotation( float degrees, float x, float y, float z )
{
	glm::vec3 axis = vec3( x, y, z );
	this->setRotation( degrees, axis );
}

void Object::setRotation( float degrees, glm::vec3 axis )
{
	float angle = degrees * (float) M_PI / 180.f;
	glm::quat rotQuat = glm::angleAxis( angle, normalize( axis ) );
	orientation = rotQuat;

	update_model_matrix = true;
}

float Object::getRotationAngle() { return glm::angle( orientation ); }

vec3 Object::getRotationAxis() { return glm::axis( orientation ); }

void Object::setOrientation( glm::quat &q )
{
	orientation = q;
}

void Object::setOrientation( glm::mat4 &m )
{
	orientation = glm::toQuat( m );
}

void Object::computeModelMatrix()
{
	glm::mat4 trans = glm::translate(glm::mat4(), position); 
	glm::mat4 rot = glm::toMat4( orientation );
    glm::mat4 scl = glm::scale(glm::mat4(), size);

	model = trans * rot * scl;
}

void Object::setColor( glm::vec3 color )
{
	mat.setColor( color );
}

void Object::setColor( float r, float g, float b )
{
	mat.setColor( r, g, b );
}

Material Object::getMaterial()
{
	return mat;
}

void Object::setShader( Shader *shader )
{
	this->shader = shader;
}

Shader* Object::getShader()
{
	return shader;
}

mat4 Object::getModelMatrix()
{	
	if( update_model_matrix ) computeModelMatrix();
	return model; 
}
