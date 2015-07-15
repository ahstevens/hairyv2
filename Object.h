/**
 * Object.h - an abstract class representing an OpenGL graphical object 

 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Texture.h"
#include "Material.h"

class Object
{
public:
    Object();
    virtual ~Object();
    
    void setPosition( float x, float y, float z ); // set location
	void setPosition(glm::vec3 location);
	glm::vec3 getPosition();               // return location as a vec3

    void setSize( float xs, float ys, float zs );   // set object size
	void setSize( glm::vec3 size );
	glm::vec3 getSize();

    void setRotate( float angle, float dx, float dy, float dz ); // set rotate
	void setRotate( float angle, glm::vec3 axis );
	float getRotationAngle();
	glm::vec3 getRotationAxis();	

	void setColor( glm::vec3 color );
	void setColor( float r, float g, float b);
	Material getMaterial();

	glm::mat4 getModelMatrix();
    
	virtual void redraw( Shader shader ) = 0;
        
protected:
	void computeModelMatrix();

    glm::vec3 position;					   // location (origin) of the object
	glm::vec3 size;						   // size of the object

	glm::mat4 model;

    float angle;						   // rotation angle and axis
	glm::vec3 axis;

	GLuint VAO, VBO, EBO;

	Material mat;

	Texture tex;
	bool use_texture;

	bool update_model_matrix;
};

#endif /*OBJECT_H_*/

