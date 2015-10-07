#include "Slice.h"
#include "IlluminatedLines.h"
#include "Icosphere.h"

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
	geometryChange = directionality = true;
	linesGenerated = tubesGenerated = false;

	il = NULL;
}

Slice::Slice( float width, float height )
{
	this->width = width;
	this->height = height;
	doIL = ilInit = false;
	geometryChange = directionality = true;
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
	geometryChange = directionality = true;
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

void Slice::updateOrientation(glm::vec3 orientation)
{
	std::vector<glm::vec3>::iterator it;

	for (it = instances.begin() + 1; it != instances.end() - 1; it += 2)
	{
		it->x = orientation.x;
		it->y = orientation.y;
		it->z = orientation.z;
	}

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, UBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(vec3), &instances[0]);
	glBindVertexArray(0);
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

void Slice::generateTubes(int segments)
{
	vertices.clear();
	indices.clear();

	std::cout << "Generating geometry for tube glyphs... ";
	
	//+++++++++++++++++++++++++++++++ GEOMETRY +++++++++++++++++++++++++++++

	Vertex tV; // temp Vertex
	GLsizei offset = 0;

	// create directionality geometry
	if (directionality)
	{
		Icosphere sphere;

		sphere.create(3);

		for (auto &vert : sphere.getVertices())
		{
			tV.position = vert;
			tV.normal = normalize(vert); // normal for a vertex on a unit sphere is just the vertex position
			tV.texture = vec2(0.f, 0.f);
			vertices.push_back(tV);
		}

		directionalIndicesCount = 0;

		for (auto &i : sphere.getIndices())
		{
			indices.push_back(offset + i);
			directionalIndicesCount++;
		}

		offset += sphere.getVertices().size();
	}

	// make a 2D circle to generate the "ribs" of the tube
	std::vector<vec2> circle = this->circle(segments);

	// push origin
	tV.position = vec3(0.f, 0.f, 0.f);
	tV.normal = vec3(0.f, 0.f, -1.f);
	tV.texture = vec2(0.f, 0.f);

	vertices.push_back(tV);

	// push first rib for base endcap
	for (std::vector<vec2>::iterator iter = circle.begin(); iter != circle.end(); ++iter)
	{
		tV.position = vec3(*iter, 0.f);
		tV.normal = vec3(0.f, 0.f, -1.f);
		tV.texture = vec2(0.f, 0.f);
		vertices.push_back(tV);
	}

	// push base rib for tube		
	for (std::vector<vec2>::iterator iter = circle.begin(); iter != circle.end(); ++iter)
	{
		tV.position = vec3(*iter, 0.f);
		tV.normal = normalize(vec3(*iter, 0.f));
		tV.texture = vec2(0.f, 0.f);
		vertices.push_back(tV);
	}

	// push tip rib for tube		
	for (std::vector<vec2>::iterator iter = circle.begin(); iter != circle.end(); ++iter)
	{
		tV.position = vec3(*iter, 1.f);
		tV.normal = normalize(vec3(*iter, 0.f));
		tV.texture = vec2(1.f, 0.f);
		vertices.push_back(tV);
	}

	// push tip rib for tip endcap		
	for (std::vector<vec2>::iterator iter = circle.begin(); iter != circle.end(); ++iter)
	{
		tV.position = vec3(*iter, 1.f);
		tV.normal = vec3(0.f, 0.f, 1.f);
		tV.texture = vec2(0.f, 0.f);
		vertices.push_back(tV);
	}

	// push tip centerpoint
	tV.position = vec3(0.f, 0.f, 1.f);
	tV.normal = vec3(0.f, 0.f, 1.f);
	tV.texture = vec2(0.f, 0.f);

	vertices.push_back(tV);

	//+++++++++++++++++++++++++++++++ INDICES +++++++++++++++++++++++++++++

	// triangles for front endcap
	for (GLsizei i = 1; i < segments + 1; ++i)
	{
		indices.push_back(offset);
		indices.push_back(offset + i % segments + 1);
		indices.push_back(offset + i);
	}

	offset += segments + 1;

	// create strip of triangles connecting ribs together
	for (GLsizei i = 0; i < segments; ++i) {
		//triangle 1
		indices.push_back(offset + i);
		indices.push_back(offset + (i + 1) % segments);
		indices.push_back(offset + (i + 1) % segments + segments);
		//triangle 2
		indices.push_back(offset + (i + 1) % segments + segments);
		indices.push_back(offset + i + segments);
		indices.push_back(offset + i);
	}

	offset += segments * 2;

	// triangles for back endcap
	GLsizei end = offset + segments;
	for (GLsizei i = 0; i < segments; ++i)
	{
		indices.push_back(end);
		indices.push_back(offset + i);
		indices.push_back(offset + (i + 1) % segments);
	}

	//+++++++++++++++++++++++++++++++ INSTANCE ATTRIBS +++++++++++++++++++++++++++++

	// a translation to center the slice at the origin
	vec3 trans(-width / 2, -height / 2, 0.f);

	// store attribute info for each instance of the glyph
	instances.clear();
	instances.reserve(seeds.size() * 2);

	for (std::vector<Seed>::iterator it = seeds.begin(); it != seeds.end(); ++it)
	{
		vec3 basePoint = vec3(it->x, it->y, 0.f) + trans;
		instances.push_back(basePoint);
		vec3 w(it->dx, it->dy, it->dz);
		instances.push_back(w);				
	}

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
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)0);
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);

		// Instance w vector attribute
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)sizeof(vec3));
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	geometryChange = false;
	tubesGenerated = true;
	std::cout << "done." << std::endl;
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

void Slice::renderPT( int segments )
{
	if (geometryChange || !tubesGenerated)
		generateTubes( segments );

	doIL = false;
	use_texture = false;
}

void Slice::renderRT( int segments, float stripe_pairs_per_mm, vec3 stripe_color1, vec3 stripe_color2 )
{
	if( geometryChange || !tubesGenerated )
		generateTubes( segments );

	doIL = false;
	use_texture = true;

	tex = new Texture();
	
	int nStripes = (int) (stripe_pairs_per_mm * 2 * 10);
	tex->stripes1D(nStripes, stripe_color1, stripe_color2);
	tex->setMinFilter(GL_NEAREST);
	tex->setMagFilter(GL_NEAREST);
}

void Slice::renderSH( float lengthMultiplier )
{
	if (geometryChange || !tubesGenerated)
		generateTubes(8);

	doIL = false;
	use_texture = false;
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
		if (use_texture) tex->enable();

		glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"),
			1,
			GL_FALSE,
			glm::value_ptr(getModelMatrix())
			);

		// Draw the container (using container's vertex attributes)
		glBindVertexArray(VAO);
		//glMultiDrawElements(GL_TRIANGLES, &counts[0], GL_UNSIGNED_INT, (const GLvoid **)&indices_offsets[0], directionality ? seeds.size() * 2 : seeds.size());
		if(directionality)
		{
			glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), true);
			glDrawElementsInstanced(GL_TRIANGLES,			 // rendering triangle primitives
									directionalIndicesCount, // number of indices to be used in rendering
									GL_UNSIGNED_INT,		 // indices array type is unsigned int
									0,						 // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
									seeds.size());			 // number of instances to render
		}
		
		glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), false);

		glDrawElementsInstanced(GL_TRIANGLES,											// rendering triangle primitives
								indices.size() - directionalIndicesCount,				// number of indices to be used in rendering
								GL_UNSIGNED_INT,										// indices array type is unsigned int
								(GLvoid*) (sizeof(GLuint) * directionalIndicesCount),   // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
								seeds.size());											// number of instances to render
		glBindVertexArray(0);

		if (use_texture) tex->disable();
	}
}