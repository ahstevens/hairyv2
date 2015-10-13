/**
 * Object.h - an abstract class representing an OpenGL graphical object 

 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "Material.h"

class Object
{
public:	
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texture;
	};

    Object();
    virtual ~Object();
    
    void setPosition( float x, float y, float z ); // set location
	void setPosition(glm::vec3 location);
	glm::vec3 getPosition();               // return location as a vec3

    void setSize( float xs, float ys, float zs );   // set object size
	void setSize( glm::vec3 size );
	glm::vec3 getSize();

    void rotate( float degrees, float x, float y, float z ); // additive rotation
	void rotate( float degrees, glm::vec3 axis );            // additive rotation
	void setRotation( float degrees, float x, float y, float z ); // absolute rotation
	void setRotation( float degrees, glm::vec3 axis );            // absolute rotation
	float getRotationAngle();
	glm::vec3 getRotationAxis();	
	
	void setOrientation( glm::quat &q );
	void setOrientation( glm::mat4 &m );

	void setColor( glm::vec3 color );
	void setColor( float r, float g, float b);
	Material getMaterial();

	void setShader( Shader *s );
	Shader* getShader();

	glm::mat4 getModelMatrix();
    
	virtual void redraw() = 0;
        
protected:
	void computeModelMatrix();

    glm::vec3 position;					   // location (origin) of the object
	glm::vec3 size;						   // size of the object
	glm::quat orientation;                 // orientation of the object

	glm::mat4 model;

	GLuint VAO, VBO, EBO, UBO;

	std::vector<Vertex> vertices;         // position/normal/tex_coords of each seed's geometry
	std::vector<GLuint> indices;          // indices for rendering each seed's geometry
	std::vector<GLvoid*> indices_offsets; // pointers to beginning of each seed's indices in the indices array
	std::vector<GLsizei> counts;          // holds the number of indices for each geometry primitive

	Material mat;

	Texture *tex;
	bool use_texture;

	Shader *shader;

	bool update_model_matrix;
};

#endif /*OBJECT_H_*/

