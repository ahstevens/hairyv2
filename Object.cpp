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

    setPosition( 0.0f, 0.0f, 0.0f );
    setSize( 1.0f, 1.0f, 1.0f );
    setRotate( 0.0f, 0.0f, 0.0f, 1.0f);

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
 * set the rotation parameters: angle, and axis specification
 */
void Object::setRotate( float angle, float dx, float dy, float dz )
{
    this->angle = angle;
    axis = vec3( dx, dy, dz );
	update_model_matrix = true;
}

void Object::setRotate( float angle, vec3 axis )
{
    this->angle = angle;
    this->axis = axis;
	update_model_matrix = true;
}

float Object::getRotationAngle() { return angle; }

vec3 Object::getRotationAxis() { return axis; }

void Object::computeModelMatrix()
{
	model = glm::mat4();
	model = glm::translate(model, position); 
	if(angle < -0.0000001 || angle > 0.0000001) model = glm::rotate(model, angle, axis);
    model = glm::scale(model, size);	
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
