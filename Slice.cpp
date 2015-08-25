#include "Slice.h"
#include "SweepSurface.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Slice::Slice(void)
	: width( 1.0f ), height( 1.0f )
{
}

Slice::Slice( float width, float height )
	: width( width ), height( height )
{
}

Slice::Slice( float width, float height, std::vector<Seed> seeds )
	: width( width ), height( height ), seeds( seeds )
{
}

Slice::~Slice(void)
{
}

void Slice::setWidth( float width )
{
	this->width = width;
}

float Slice::getWidth( void )
{
	return width;
}

void Slice::setHeight( float height )
{
	this->height = height;
}

float Slice::getHeight( void )
{
	return height;
}

void Slice::addSeed( Seed s )
{
	seeds.push_back( s );
}

void Slice::addSeeds( std::vector<Seed> seeds )
{
	std::vector<Seed> newSeeds;
	newSeeds.reserve( this->seeds.size() + seeds.size() );
	newSeeds.insert( newSeeds.end(), this->seeds.begin(), this->seeds.end() );
	newSeeds.insert( newSeeds.end(), seeds.begin(), seeds.end() );
	this->seeds = newSeeds;
}

void Slice::removeSeed( void )
{
	seeds.pop_back();
}

void Slice::removeSeeds( int n )
{
	for( int i = 0; i < n; ++i ) seeds.pop_back();
}

void Slice::clearSeeds( void )
{
	seeds.clear();
}

std::vector<vec2> Slice::circle(int segments)
{
    float angleIncrement = 2.0f * (float) M_PI / (float) segments;
    
    std::vector<vec2> circle;

    for( int i = segments - 1; i >= 0; --i )
        circle.push_back(vec2(float(sin(i * angleIncrement)) * 0.5f, 
                              float(cos(i * angleIncrement)) * 0.5f));

    return circle;
}

void Slice::generateTubes( int segments, float thickness, float lengthMultiplier )
{
	vertices.clear();
	indices.clear();
	indices_offsets.clear();
	counts.clear();

	std::vector<vec2> circle = this->circle( segments );
	GLuint offset = 0;

	std::vector<Seed>::iterator it;
	for( it = seeds.begin(); it != seeds.end(); ++it )
	{
		std::vector<vec3> path;
		path.push_back( vec3( it->x, it->y, 0.0f ) );
		path.push_back( vec3( it->x + ( it->dx * lengthMultiplier ),
							  it->y + ( it->dy * lengthMultiplier ),
							  it->dz * lengthMultiplier ) );

		SweepSurface s( circle, path );
		s.updateScales( thickness );

		std::vector<Vertex> tubeVerts = s.getVertices();
		vertices.insert( vertices.end(), tubeVerts.begin(), tubeVerts.end() );

		std::vector<GLuint> tubeIndices = s.getIndices();
		for(GLuint& i : tubeIndices)
			i += offset;
		indices.insert( indices.end(), tubeIndices.begin(), tubeIndices.end() );
		counts.push_back( tubeIndices.size() );
		indices_offsets.push_back( offset );
		offset += tubeVerts.size();
	}

	// set up VAO
	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);
		// Texture coordinate attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, texture)));
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void Slice::redraw( Shader shader )
{
	glUniform3f(glGetUniformLocation(shader.Program, "material.ambient"), 
		mat.getAmbientColor().r, mat.getAmbientColor().g, mat.getAmbientColor().b);
	glUniform3f(glGetUniformLocation(shader.Program, "material.diffuse"), 
		mat.getDiffuseColor().r, mat.getDiffuseColor().g, mat.getDiffuseColor().b);
	glUniform3f(glGetUniformLocation(shader.Program, "material.specular"), 
		mat.getSpecularColor().r, mat.getSpecularColor().g, mat.getSpecularColor().b);
	glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 
		mat.getShininess());
	glUniform1i(glGetUniformLocation(shader.Program, "use_texture"),
		use_texture);
		
	//glActiveTexture(GL_TEXTURE0);
	if (use_texture) tex.enable();
		
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 
					   1,
					   GL_FALSE,
					   glm::value_ptr(getModelMatrix())
					   );

	// Draw the container (using container's vertex attributes)
	glBindVertexArray(VAO);
	glMultiDrawElements(GL_TRIANGLES, &counts[0], GL_UNSIGNED_INT, (const GLvoid **) &indices_offsets[0], seeds.size() );
	glBindVertexArray(0);

	if (use_texture) tex.disable();
}