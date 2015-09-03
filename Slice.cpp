#include "Slice.h"
#include "SweepSurface.h"
#include "IlluminatedLines.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

// glm::value_ptr
#include <glm/gtc/type_ptr.hpp>
// glm::translate
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

Slice::Slice(void) 
{
	width = height = 1.0f;
	doIL = ilInit = false;
	geometryChange = true;
	linesGenerated = tubesGenerated = false;

	il = NULL;
}

Slice::Slice( float width, float height )
{
	this->width = width;
	this->height = height;
	doIL = ilInit = false;
	geometryChange = true;
	linesGenerated = tubesGenerated = false;

	il = NULL;
}

Slice::Slice( float width, float height, std::vector<Seed> seeds )
	: width( width ), height( height ), seeds( seeds ), doIL( false ), ilInit( false ), geometryChange( true ), linesGenerated( false ), tubesGenerated( false )
{
	this->width = width;
	this->height = height;
	this->seeds = seeds;
	doIL = ilInit = false;
	geometryChange = true;
	linesGenerated = tubesGenerated = false;

	il = NULL;
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

	geometryChange = true;
}

void Slice::addSeeds( std::vector<Seed> seeds )
{
	std::vector<Seed> newSeeds;
	newSeeds.reserve( this->seeds.size() + seeds.size() );
	newSeeds.insert( newSeeds.end(), this->seeds.begin(), this->seeds.end() );
	newSeeds.insert( newSeeds.end(), seeds.begin(), seeds.end() );
	this->seeds = newSeeds;

	geometryChange = true;
}

void Slice::removeSeed( void )
{
	seeds.pop_back();
	
	geometryChange = true;
}

void Slice::removeSeeds( int n )
{
	for( int i = 0; i < n; ++i ) seeds.pop_back();
	
	geometryChange = true;
}

void Slice::clearSeeds( void )
{
	seeds.clear();
	
	geometryChange = true;
}

int Slice::seedCount()
{
	return seeds.size();
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

	mat4 trans = translate( mat4(1.f), vec3(-width/2, -height/2, 0.0));	
	
	std::cout << "Generating geometry for " << seedCount() << " tube glyphs... ";
	std::vector<Seed>::iterator it;
	for( it = seeds.begin(); it != seeds.end(); ++it )
	{
		std::vector<vec3> path;
		path.push_back( vec3( trans * vec4( it->x, it->y, 0.0f, 1.0f ) ) );
		path.push_back( vec3( trans * vec4( it->x + ( it->dx * lengthMultiplier ),
											it->y + ( it->dy * lengthMultiplier ),
											it->dz * lengthMultiplier, 1.0f ) ) );

		SweepSurface s( circle, path );
		s.updateScales( thickness );

		std::vector<Vertex> tubeVerts = s.getVertices();
		vertices.insert( vertices.end(), tubeVerts.begin(), tubeVerts.end() );

		std::vector<GLuint> tubeIndices = s.getIndices();
		for(GLuint& i : tubeIndices)
			i += offset;
		indices_offsets.push_back((GLvoid*)(indices.size() * sizeof(GLuint)));
		indices.insert( indices.end(), tubeIndices.begin(), tubeIndices.end() );
		counts.push_back( tubeIndices.size() );
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

	geometryChange = false;
	tubesGenerated = true;
	std::cout << "done (" << vertices.size() / 3 << " vertices generated)" << std::endl;
}

void Slice::generateHairs(float lengthMultiplier)
{
	std::vector<float> verts;
	std::vector<int> first;
	indices.clear();
	indices_offsets.clear();
	counts.clear();

	GLint offset = 0;

	mat4 trans = translate(mat4(1.f), vec3(-width / 2, -height / 2, 0.0));

	std::cout << "Generating geometry for " << seedCount() << " line glyphs... ";
	std::vector<Seed>::iterator it;
	for (it = seeds.begin(); it != seeds.end(); ++it)
	{
		vec4 base = trans * vec4( it->x, it->y, 0.0f, 1.0f );
		verts.push_back( base.x );
		verts.push_back( base.y );
		verts.push_back( base.z );

		vec4 tip = trans * vec4( it->x + ( it->dx * lengthMultiplier ),
								 it->y + ( it->dy * lengthMultiplier ),
								 it->dz * lengthMultiplier,
								 1.0f );

		verts.push_back( tip.x );
		verts.push_back( tip.y );
		verts.push_back( tip.z );
		
		first.push_back(offset);
		offset += 2;
		counts.push_back(2);
	}
	
	geometryChange = false;
	linesGenerated = true;
	std::cout << "done (" << verts.size() / 3 << " vertices generated)" << std::endl;

	this->il = new IlluminatedLines(seeds.size(), verts.size() / 3, first, counts, verts, NULL, ILines::ILLightingModel::IL_CYLINDER_BLINN);
	ilInit = false;
}

void Slice::renderIL( ILines::ILLightingModel::Model lightModel, float lengthMultiplier )
{	

	if( geometryChange || !linesGenerated ) 
		generateHairs( lengthMultiplier );	
		
	il->setLightingModel(lightModel);

	doIL = true;
}

void Slice::renderPL( float lengthMultiplier )
{
	if( geometryChange || !linesGenerated ) 
		generateHairs( lengthMultiplier );	

	doIL = true;
}

void Slice::renderPT( int segments, float thickness, float lengthMultiplier )
{
	if( geometryChange || !tubesGenerated )
		generateTubes( segments, thickness, lengthMultiplier );

	doIL = false;
}

void Slice::renderRT( int segments, float thickness, float lengthMultiplier )
{
	if( geometryChange || !tubesGenerated )
		generateTubes( segments, thickness, lengthMultiplier );

	doIL = false;
}

void Slice::renderSH( float lengthMultiplier )
{
	doIL = false;
}

void Slice::setILPVMatrix( float * pM, float *vM )
{
	il->setPVMatrix( pM, vM );
}

void Slice::redraw()
{
	if (doIL)
	{
		if (!ilInit)
		{
			std::cout << "Initializing Illuminated Streamlines..." << std::endl;
			ilInit = true;
			il->init();
		}
		il->redraw();
	}
	else {
		glUniform3f(glGetUniformLocation(shader->Program, "material.ambient"),
			mat.getAmbientColor().r, mat.getAmbientColor().g, mat.getAmbientColor().b);
		glUniform3f(glGetUniformLocation(shader->Program, "material.diffuse"),
			mat.getDiffuseColor().r, mat.getDiffuseColor().g, mat.getDiffuseColor().b);
		glUniform3f(glGetUniformLocation(shader->Program, "material.specular"),
			mat.getSpecularColor().r, mat.getSpecularColor().g, mat.getSpecularColor().b);
		glUniform1f(glGetUniformLocation(shader->Program, "material.shininess"),
			mat.getShininess());
		glUniform1i(glGetUniformLocation(shader->Program, "use_texture"),
			use_texture);

		//glActiveTexture(GL_TEXTURE0);
		if (use_texture) tex.enable();

		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"),
			1,
			GL_FALSE,
			glm::value_ptr(getModelMatrix())
			);

		// Draw the container (using container's vertex attributes)
		glBindVertexArray(VAO);
		glMultiDrawElements(GL_TRIANGLES, &counts[0], GL_UNSIGNED_INT, (const GLvoid **)&indices_offsets[0], seeds.size());
		glBindVertexArray(0);

		if (use_texture) tex.disable();
	}
}