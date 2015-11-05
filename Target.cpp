#include "Target.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

// glm::value_ptr
#include <glm/gtc/type_ptr.hpp>
// glm::translate
#include <glm/gtc/matrix_transform.hpp>
// glm::gtx::quaternion
#include <glm\ext.hpp>

using namespace glm;

Target::Target( glm::vec3 color ) 
{
	this->setColor( color );
	init();
}

Target::~Target(void)
{
}

void Target::init()
{
	generate();

	use_texture = false;
}

void Target::generate()
{
	vertices.clear();
	indices.clear();
	
	//+++++++++++++++++++++++++++++++ GEOMETRY +++++++++++++++++++++++++++++

	Vertex tV; // temp Vertex

	// top triangle
	tV.position = vec3(0.25f, 1.f, 0.f);
	tV.normal = vec3(0.f, 0.f, 1.f);
	tV.texture = vec2(0.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.25f, 1.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(0.f, 0.1f, 0.f);
	vertices.push_back(tV);

	// left triangle
	tV.position = vec3(-1.f, 0.25f, 0.f);
	vertices.push_back(tV);
	
	tV.position = vec3(-1.f, -0.25f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.1f, 0.f, 0.f);
	vertices.push_back(tV);

	// bottom triangle
	tV.position = vec3(-0.25f, -1.f, 0.f);
	vertices.push_back(tV);
	
	tV.position = vec3(0.25f, -1.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(0.f, -0.1f, 0.f);
	vertices.push_back(tV);

	// right triangle
	tV.position = vec3(1.f, -0.25f, 0.f);
	vertices.push_back(tV);
	
	tV.position = vec3(1.f, 0.25f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(0.1f, 0.f, 0.f);
	vertices.push_back(tV);



	//+++++++++++++++++++++++++++++++ INDICES +++++++++++++++++++++++++++++

	int nFaces = 4;

	GLsizei offset = 0;

	for(int i = 0; i < nFaces; ++i)
	{
		indices.push_back(offset);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);

		offset += 3;
	}

	//+++++++++++++++++++++++++++++++ INSTANCE ATTRIBS +++++++++++++++++++++++++++++
	
	// store attribute info for each instance of the glyph
	instances.clear();

	vec3 basePoint = vec3( 0.f, 0.f, 0.f );
	instances.push_back( basePoint );
		
	// Forward vector
	glm::vec3 w = glm::vec3( 0.f, 0.f, 1.f );
	instances.push_back( w );

	//+++++++++++++++++++++++++++++++ DATA TRANSFER +++++++++++++++++++++++++++++

	// set up VAO
	glBindVertexArray(VAO);

		// Bind buffer for vertex info and fill it
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(0, 0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 0);
		// Texture coordinate attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, texture)));
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 0);


		// Bind buffer for instance info and fill it
		glBindBuffer(GL_ARRAY_BUFFER, UBO);
		glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(vec3), &instances[0], GL_DYNAMIC_DRAW);

		// Instance base location attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(sizeof(vec3) * 0));
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);

		// Instance w vector attribute
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(sizeof(vec3) * 1));
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);
}

void Target::redraw()
{
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(getModelMatrix())
		);

	// Draw the container (using container's vertex attributes)
	glBindVertexArray(VAO);

		glUniform1f(glGetUniformLocation(shader->Program, "opacity"), 0.1f);
		glDrawElementsInstanced(GL_TRIANGLES,					// rendering triangle primitives
								indices.size(),					// number of indices to be used in rendering
								GL_UNSIGNED_INT,				// indices array type is unsigned int
								(GLvoid*) (sizeof(GLuint) * 0),	// byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
								1);								// number of instances to render

		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glUniform1f(glGetUniformLocation(shader->Program, "opacity"), 0.25f);
		glDrawElementsInstanced(GL_TRIANGLES,					// rendering triangle primitives
								indices.size(),					// number of indices to be used in rendering
								GL_UNSIGNED_INT,				// indices array type is unsigned int
								(GLvoid*) (sizeof(GLuint) * 0),	// byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
								1);								// number of instances to render
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glBindVertexArray(0);
}