#include "Probe.h"
#include "Icosphere.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

// glm::value_ptr
#include <glm/gtc/type_ptr.hpp>
// glm::translate
#include <glm/gtc/matrix_transform.hpp>
// glm::gtx::quaternion
#include <glm\ext.hpp>

using namespace glm;

Probe::Probe(void) 
{
	geometryChange = true;
	tubesGenerated = false;
}

Probe::~Probe(void)
{
}

std::vector<vec2> Probe::circle(int segments)
{
    float angleIncrement = 2.0f * (float) M_PI / (float) segments;
    
    std::vector<vec2> circle;

    for( int i = segments - 1; i >= 0; --i )
        circle.push_back(vec2(float(sin(i * angleIncrement)) * 0.5f, 
                              float(cos(i * angleIncrement)) * 0.5f));

    return circle;
}

void Probe::generateProbe(int segments)
{
	vertices.clear();
	indices.clear();

	//std::cout << "Generating geometry for tube glyphs... ";
	
	//+++++++++++++++++++++++++++++++ GEOMETRY +++++++++++++++++++++++++++++

	Vertex tV; // temp Vertex
	GLsizei offset = 0;

	// create directionality geometry
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


	// left face
	tV.position = vec3(-0.5f, 0.5f, 0.f);
	tV.normal = vec3(-1.f, 0.f, 0.f);
	tV.texture = vec2(0.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, -0.5f, 0.f);
	tV.texture = vec2(0.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, -0.5f, 1.f);
	tV.texture = vec2(1.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, 0.5f, 1.f);
	tV.texture = vec2(1.f, 0.f);
	vertices.push_back(tV);

	// right face
	tV.position = vec3(0.5f, -0.5f, 0.f);
	tV.normal = vec3(1.f, 0.f, 0.f);
	tV.texture = vec2(0.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, 0.5f, 0.f);
	tV.texture = vec2(0.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, 0.5f, 1.f);
	tV.texture = vec2(1.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, -0.5f, 1.f);
	tV.texture = vec2(1.f, 0.f);
	vertices.push_back(tV);

	// top face
	tV.position = vec3(0.5f, 0.5f, 0.f);
	tV.normal = vec3(0.f, 1.f, 0.f);
	tV.texture = vec2(0.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, 0.5f, 0.f);
	tV.texture = vec2(0.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, 0.5f, 1.f);
	tV.texture = vec2(1.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, 0.5f, 1.f);
	tV.texture = vec2(1.f, 0.f);
	vertices.push_back(tV);

	// bottom face
	tV.position = vec3(-0.5f, -0.5f, 0.f);
	tV.normal = vec3(0.f, -1.f, 0.f);
	tV.texture = vec2(0.f, 0.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, -0.5f, 0.f);
	tV.texture = vec2(0.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, -0.5f, 1.f);
	tV.texture = vec2(1.f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, -0.5f, 1.f);
	tV.texture = vec2(1.f, 0.f);
	vertices.push_back(tV);

	// endcap face
	tV.position = vec3(0.5f, 0.5f, 1.f);
	tV.normal = vec3(0.f, 0.f, 1.f);
	tV.texture = vec2(0.01f, 0.01f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, 0.5f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(-0.5f, -0.5f, 1.f);
	vertices.push_back(tV);

	tV.position = vec3(0.5f, -0.5f, 1.f);
	vertices.push_back(tV);



	//+++++++++++++++++++++++++++++++ INDICES +++++++++++++++++++++++++++++

	int nFaces = 5;

	for(int i = 0; i < nFaces; ++i)
	{
		indices.push_back(offset);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);
		indices.push_back(offset);

		offset += 4;
	}

	//+++++++++++++++++++++++++++++++ INSTANCE ATTRIBS +++++++++++++++++++++++++++++
	
	// store attribute info for each instance of the glyph
	instances.clear();

	vec3 basePoint = vec3( 0.f, 0.f, 0.f );
	instances.push_back( basePoint );
		
	// 2 - Flow quaternion. Will be used as an analog to the forward vector
	//                      of a per-seed flow-aligned coordinate frame.
	glm::vec3 w = glm::vec3( 1.f, 0.f, 0.f );
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

	geometryChange = false;
	tubesGenerated = true;
	//std::cout << "done." << std::endl;
}

void Probe::renderPT( int segments )
{
	if (geometryChange || !tubesGenerated)
		generateProbe( segments );
	
	use_texture = false;
}

void Probe::renderRT( int segments, float stripe_pairs_per_mm, vec3 stripe_color1, vec3 stripe_color2 )
{
	if( geometryChange || !tubesGenerated )
		generateProbe( segments );

	use_texture = true;

	tex = new Texture();
	
	int nStripes = (int) (stripe_pairs_per_mm * 2 * 10);
	tex->stripes1D(nStripes, stripe_color1, stripe_color2);
	tex->checker(4, 4);
	tex->setMinFilter(GL_NEAREST);
	tex->setMagFilter(GL_NEAREST);
}

void Probe::redraw()
{
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

	glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), true);
	glUniform1ui(glGetUniformLocation(shader->Program, "glyphHead"), false);
	glDrawElementsInstanced(GL_TRIANGLES,			 // rendering triangle primitives
							directionalIndicesCount, // number of indices to be used in rendering
							GL_UNSIGNED_INT,		 // indices array type is unsigned int
							0,						 // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
							1);			 // number of instances to render

		
	glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), false);
	glDisable(GL_CULL_FACE);
	glDrawElementsInstanced(GL_TRIANGLES,											// rendering triangle primitives
							indices.size() - directionalIndicesCount,				// number of indices to be used in rendering
							GL_UNSIGNED_INT,										// indices array type is unsigned int
							(GLvoid*) (sizeof(GLuint) * directionalIndicesCount),   // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
							1);											// number of instances to render
	glEnable(GL_CULL_FACE);
	glBindVertexArray(0);

	if (use_texture) tex->disable();	
}