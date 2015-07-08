/**
 * Object.h - an abstract class representing an OpenGL graphical object 

 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Texture.h"

class Object
{
public:
    Object();
    virtual ~Object();
    
    void setLocation( float x, float y, float z ); // set location
	void setLocation( glm::vec3 location );
    void setSize( float xs, float ys, float zs );   // set object size
	void setSize( glm::vec3 size );
    void setRotate( float angle, float dx, float dy, float dz ); // set rotate
	void setRotate( float angle, glm::vec3 axis );

	glm::mat4 getModelMatrix();
    
    float getX();                          // return x location
    float getY();                          // return y location
    float getZ();                          // return z location
    glm::vec3 getLocation();               // return location as a vec3

	float getRotationAngle();
	glm::vec3 getRotationAxis();

	glm::vec3 getSize();
    
    virtual void redraw( Shader shader ) = 0;
        
protected:
    glm::vec3 location;					   // location (origin) of the object
	glm::vec3 size;						   // size of the object

	glm::mat4 model;

    float angle;						   // rotation angle and axis
	glm::vec3 axis;

	GLuint VAO, VBO, EBO;

	Texture tex;
};

#endif /*OBJECT_H_*/

