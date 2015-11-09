#include "Trial.h"
#include "Icosphere.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

#include <random>
#include <time.h> // time() for srand()

// glm::value_ptr
#include <glm/gtc/type_ptr.hpp>
// glm::translate
#include <glm/gtc/matrix_transform.hpp>

//#define BIMAP_BASE_SIZE 1000.f
#define EPSILON            0.001f

using namespace glm;

Trial::Trial(float xSize, float ySize, float density, float jitter, RenderMode renderMode) : xSize(xSize), ySize(ySize), density(density), jitter(jitter), renderMode(renderMode)
{
	doIL = ilInit = doLines = false;
	geometryChange = directionality = true;
	linesGenerated = ilGenerated = tubesGenerated = false;

	il = NULL;
	bimap = NULL;
}


Trial::~Trial()
{
	if ( bimap != NULL )
		delete bimap;
}


void Trial::init()
{
	// seed the rand function for use in BiMap
	srand((unsigned int) time(NULL));
	
	makeBiMap();
	sampleBiMap();

	//testPattern();

	setRenderMode(renderMode);
}

void Trial::makeBiMap()
{
	// aspect ratio
	//float ar = xSize / ySize;
	//float bmMaxX, bmMaxY;

	//if( ar > 1.0f ) {
	//	bmMaxX = BIMAP_BASE_SIZE;
	//	bmMaxY = BIMAP_BASE_SIZE / ar;
	//}
	//else {
	//	bmMaxX = BIMAP_BASE_SIZE * ar;
	//	bmMaxY = BIMAP_BASE_SIZE;
	//}

	// free memory held by any existing BiMap
	if(bimap != NULL)
		delete bimap;

	//std::cout << "Generating " << bmMaxX << " x " << bmMaxY << " bimap (AR = " << ar << ")... ";
	//bimap = new BiMap( bmMaxX, bmMaxY );
	bimap = new BiMap( xSize, ySize );
	//std::cout << "done" << std::endl;	
}

void Trial::sampleBiMap()
{
	shadowOffset = 0.f;

	maxLength = 0.f;

	float xStep = 1 / density;
	float yStep = 1 / density;

	//std::cout << "Seeding the " << xSize << " x " << ySize << " cutting plane at a density of " << density << " glyphs/mm using the BiMap... ";

	clearSeeds();
	
    for( float i = fmod( ( xSize / 2 ), xStep ); i < ( xSize + EPSILON ); i += xStep )
	{
        for( float j = fmod( ( ySize / 2 ), yStep ); j < ( ySize + EPSILON ); j += yStep )
        {
			float x_jitter;
            if( i < EPSILON )
                x_jitter = ( rand() / (float) RAND_MAX ) * jitter;                  // +jitter
            else if( abs( i - ( xSize - xStep ) ) < EPSILON )
                x_jitter = ( rand() / (float) RAND_MAX ) * -jitter;                 // -jitter
            else
                x_jitter = ( rand() / (float) RAND_MAX ) * ( 2 * jitter ) - jitter; // +/- jitter

			float y_jitter;
            if( j < EPSILON )
                y_jitter = ( rand() / (float) RAND_MAX ) * jitter;                  // +jitter
            else if( abs( j - ( ySize - yStep ) ) < EPSILON )
                y_jitter = ( rand() / (float) RAND_MAX ) * -jitter;                 // -jitter
            else
                y_jitter = ( rand() / (float) RAND_MAX ) * ( 2 * jitter ) - jitter; // +/- jitter

			Seed seed;
            seed.x = (float) i + ( x_jitter * xStep );
            seed.y = (float) j + ( y_jitter * yStep );

            bimap->getVecValues(seed.x, seed.y, seed.dx, seed.dy, seed.dz);

			seed.twist = 0.f;

			addSeed( seed );

			if (seed.dz < shadowOffset) shadowOffset = seed.dz;

			if (seed.length() > maxLength) maxLength = seed.length();
        }
	}

	//std::cout << "done" << std::endl;

	//std::cout << "Max Length: " << maxLength << std::endl;
}

Trial::Seed Trial::getRandomSeed()
{
	Seed seed;

	std::random_device rand_seed;  // random seed
	std::mt19937 gen(rand_seed()); // Mersenne Twister RNG
	std::uniform_real_distribution<float> x_dist(0.f, xSize);
	std::uniform_real_distribution<float> y_dist(0.f, ySize);

	seed.x = x_dist( gen );
	seed.y = y_dist( gen );

	bimap->getVecValues(seed.x, seed.y, seed.dx, seed.dy, seed.dz);

	return seed;
}

void Trial::setRenderMode(RenderMode renderMode, float lengthMultiplier)
{
	this->renderMode = renderMode;

	if (renderMode == SHADOWED_HEDGEHOGS)
	{
		setJitter(0.f);
		sampleBiMap();
	}
	else
	{
		if (jitter < 0.24)
		{
			setJitter(0.25f);
			sampleBiMap();
		}
	}

	switch (renderMode)
	{
		case LINES_PLAIN:
			if (geometryChange || !linesGenerated)
				generateHairs();

			doIL = false;
			doLines = true;
			use_texture = false;
			break;	
		case TUBES_PLAIN:
		case TUBES_RINGED:
		case SHADOWED_HEDGEHOGS:
		{
			if (geometryChange || !tubesGenerated)
				generateTubes(16);

			if (renderMode == TUBES_RINGED)
			{
				use_texture = true;
				tex = new Texture();

				int nStripes = (int)(1 * 2 * 10); // first number is stripes per mm
				tex->stripes1D(nStripes, vec3(1.f, 1.f, 1.f), vec3(0.5f, 0.5f, 0.5f));
				tex->setMinFilter(GL_NEAREST);
				tex->setMagFilter(GL_NEAREST);
			}
			else
				use_texture = false;

			doIL = doLines = false;
			break;
		}
		case LINES_ILLUMINATED_CYLINDER_BLINN:
		case LINES_ILLUMINATED_CYLINDER_PHONG:
		case LINES_ILLUMINATED_MAXIMUM_PHONG:
		{
			generateHairs();

			vertices.clear();
			indices.clear();
			indices_offsets.clear();
			counts.clear();

			std::vector<GLfloat> vertices_flat;
			std::vector<GLsizei> first;

			GLsizei offset = 0;

			GLsizei nVerts = 2;

			directionalIndicesCount = insertDirectionalGeometry(vertices, indices, offset);

			offset = 0;

			mat4 trans = translate(mat4(1.f), vec3(-xSize / 2.f, -ySize / 2.f, 0.f));

			std::vector<Seed>::iterator it;
			for (it = seeds.begin(); it != seeds.end(); ++it)
			{
				vec4 base = trans * vec4(it->x, it->y, 0.0f, 1.0f);
				vertices_flat.push_back(base.x);
				vertices_flat.push_back(base.y);
				vertices_flat.push_back(base.z);

				vec4 tip = trans * vec4(it->x + (it->dx * lengthMultiplier),
					it->y + (it->dy * lengthMultiplier),
					it->dz * lengthMultiplier,
					1.0f);

				vertices_flat.push_back(tip.x);
				vertices_flat.push_back(tip.y);
				vertices_flat.push_back(tip.z);

				first.push_back(offset);
				offset += nVerts;
				counts.push_back(nVerts);
			}

			if (this->il != NULL) delete this->il;

			switch (renderMode)
			{
				case LINES_ILLUMINATED_CYLINDER_BLINN:
					this->il = new IlluminatedLines(seeds.size(), vertices_flat.size() / 3, first, counts, vertices_flat, NULL, ILines::ILLightingModel::IL_CYLINDER_BLINN);
					break;
				case LINES_ILLUMINATED_CYLINDER_PHONG:
					this->il = new IlluminatedLines(seeds.size(), vertices_flat.size() / 3, first, counts, vertices_flat, NULL, ILines::ILLightingModel::IL_CYLINDER_PHONG);
					break;
				case LINES_ILLUMINATED_MAXIMUM_PHONG:
					this->il = new IlluminatedLines(seeds.size(), vertices_flat.size() / 3, first, counts, vertices_flat, NULL, ILines::ILLightingModel::IL_MAXIMUM_PHONG);
					break;
			}

			doIL = ilGenerated = true;
			doLines = ilInit = geometryChange = linesGenerated = false;
		}
	}
}

Trial::RenderMode Trial::getRenderMode()
{
	return this->renderMode;
}

void Trial::setJitter(float jitter)
{
	this->jitter = jitter;
}

void Trial::setDensity(float density)
{
	this->density = density;
}

void Trial::passThroughPVMatrix( float *pM, float *vM )
{
	il->setPVMatrix(pM, vM);
}

float Trial::getShadowOffset()
{
	return shadowOffset;
}

float Trial::getMaxLength()
{
	return maxLength;
}

void Trial::addSeed(Seed s)
{
	seeds.push_back(s);

	geometryChange = true;
}

void Trial::clearSeeds(void)
{
	seeds.clear();

	geometryChange = true;
}

void Trial::generateTubes(int segments)
{
	vertices.clear();
	indices.clear();

	//std::cout << "Generating geometry for tube glyphs... ";

	//+++++++++++++++++++++++++++++++ GEOMETRY +++++++++++++++++++++++++++++

	Vertex tV; // temp Vertex
	GLsizei offset = 0;

	// create directionality geometry
	if (directionality) directionalIndicesCount = insertDirectionalGeometry(vertices, indices, offset);
	else directionalIndicesCount = 0;

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
	vec3 trans(-xSize / 2, -ySize / 2, 0.f);

	// store attribute info for each instance of the glyph
	instances.clear();
	instances.reserve(seeds.size() * 2);

	// In model space, the 2D slice is aligned with xy-plane and the slice is
	// centered at the origin with the slice front face normal = (0,0,1)
	for (std::vector<Seed>::iterator it = seeds.begin(); it != seeds.end(); ++it)
	{
		// 1 - Seed location on slice.
		vec3 basePoint = vec3(it->x, it->y, 0.f) + trans;
		instances.push_back(basePoint);

		// 2 - Flow vector. Will be used as an analog to the forward vector
		//                  of a per-seed flow-aligned coordinate frame.
		glm::vec3 w = glm::vec3(it->dx, it->dy, it->dz);
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
	glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(vec3), &instances[0], GL_STATIC_DRAW);

	// Instance base location attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(sizeof(vec3) * 0));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	// Instance w vector attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(sizeof(vec3) * 1));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	geometryChange = false;
	tubesGenerated = true;
	linesGenerated = false;

	//std::cout << "done." << std::endl;
}

void Trial::generateHairs()
{
	vertices.clear();
	indices.clear();

	//std::cout << "Generating geometry for tube glyphs... ";

	//+++++++++++++++++++++++++++++++ GEOMETRY ++++++++++++++++++++++++++++++++

	Vertex tV; // temp Vertex
	GLsizei offset = 0;

	// create directionality geometry
	if (directionality) directionalIndicesCount = insertDirectionalGeometry(vertices, indices, offset);
	else directionalIndicesCount = 0;


	// push origin
	tV.position = vec3(0.f, 0.f, 0.f);
	tV.normal = vec3(0.f, 0.f, -1.f);
	tV.texture = vec2(0.f, 0.f);

	vertices.push_back(tV);

	// push tip centerpoint
	tV.position = vec3(0.f, 0.f, 1.f);
	tV.normal = vec3(0.f, 0.f, 1.f);
	tV.texture = vec2(0.f, 0.f);

	vertices.push_back(tV);

	//+++++++++++++++++++++++++++++++ INDICES +++++++++++++++++++++++++++++++++

	indices.push_back(offset);
	indices.push_back(offset + 1);

	//+++++++++++++++++++++++++++++++ INSTANCE ATTRIBS ++++++++++++++++++++++++

	// a translation to center the slice at the origin
	vec3 trans(-xSize / 2, -ySize / 2, 0.f);

	// store attribute info for each instance of the glyph
	instances.clear();
	instances.reserve(seeds.size() * 2);

	// In model space, the 2D slice is aligned with xy-plane and the slice is
	// centered at the origin with the slice front face normal = (0,0,1)
	for (std::vector<Seed>::iterator it = seeds.begin(); it != seeds.end(); ++it)
	{
		// 1 - Seed location on slice.
		vec3 basePoint = vec3(it->x, it->y, 0.f) + trans;
		instances.push_back(basePoint);

		// 2 - Flow vector. Will be used as an analog to the forward vector
		//                  of a per-seed flow-aligned coordinate frame.
		glm::vec3 w = glm::vec3(it->dx, it->dy, it->dz);
		instances.push_back(w);
	}

	//+++++++++++++++++++++++++++++++ DATA TRANSFER +++++++++++++++++++++++++++

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
	glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(vec3), &instances[0], GL_STATIC_DRAW);

	// Instance base location attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(sizeof(vec3) * 0));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	// Instance w vector attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(sizeof(vec3) * 1));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	geometryChange = false;
	linesGenerated = true;
	tubesGenerated = false;
}


GLsizei Trial::insertDirectionalGeometry(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, GLsizei &offset)
{
	Icosphere sphere;
	sphere.create(3);

	Vertex tempV;

	for (auto &vert : sphere.getVertices())
	{
		tempV.position = vert;
		tempV.normal = normalize(vert); // normal for a vertex on a unit sphere is just the vertex position
		tempV.texture = vec2(0.f, 0.f);
		vertices.push_back(tempV);
	}

	GLsizei indexCount = 0;

	for (auto &i : sphere.getIndices())
	{
		indices.push_back(offset + i);
		indexCount++;
	}

	offset += sphere.getVertices().size();

	return indexCount;
}

std::vector<vec2> Trial::circle(int segments)
{
	float angleIncrement = 2.0f * (float)M_PI / (float)segments;

	std::vector<vec2> circle;

	for (int i = segments - 1; i >= 0; --i)
		circle.push_back(vec2(float(sin(i * angleIncrement)) * 0.5f,
		float(cos(i * angleIncrement)) * 0.5f));

	return circle;
}

void Trial::redraw()
{
	if (geometryChange)
		if (doLines)
			generateHairs();
		else
			generateTubes();

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

	// Draw the directional glyph heads
	glBindVertexArray(VAO);
	if (directionality)
	{
		glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), true);
		glUniform1ui(glGetUniformLocation(shader->Program, "glyphHead"), true);
		glDrawElementsInstanced(GL_TRIANGLES,			 // rendering triangle primitives
			directionalIndicesCount, // number of indices to be used in rendering
			GL_UNSIGNED_INT,		 // indices array type is unsigned int
			0,						 // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
			seeds.size());			 // number of instances to render
	}
	glBindVertexArray(0);

	// Now draw the line rendering
	if (doIL)
	{
		Shader::Off();

		if (!ilInit)
		{
			il->init();
			ilInit = true;
		}

		il->redraw();

		shader->Use();
	}
	else
	{
		glBindVertexArray(VAO);

		glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), false);

		glDrawElementsInstanced(doLines ? GL_LINES : GL_TRIANGLES,						// rendering line or triangle primitives
			indices.size() - directionalIndicesCount,				// number of indices to be used in rendering
			GL_UNSIGNED_INT,										// indices array type is unsigned int
			(GLvoid*)(sizeof(GLuint) * directionalIndicesCount),   // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
			seeds.size());											// number of instances to render
		glBindVertexArray(0);
	}

	if (use_texture) tex->disable();
}