#include "Probe.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

using namespace glm;

Probe::Probe(void)
{
	
}

Probe::~Probe(void)
{

}

std::vector<glm::vec2> Probe::circle(int segments)
{
    float angleIncrement = 2.0f * (float) M_PI / (float) segments;
    
    std::vector<glm::vec2> circle;

    for( int i = segments - 1; i >= 0; --i )
        circle.push_back(glm::vec2(float(sin(i * angleIncrement)) * 0.5f, 
								   float(cos(i * angleIncrement)) * 0.5f));

    return circle;
}

void Probe::generateProbe()
{
	vertices.clear();
	indices.clear();

	int segments = 16;

	std::cout << "Generating geometry for tube glyphs... ";
	
	//+++++++++++++++++++++++++++++++ GEOMETRY +++++++++++++++++++++++++++++

	Vertex tV; // temp Vertex
	GLsizei offset = 0;

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

		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);

		// Texture coordinate attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, texture)));
		glEnableVertexAttribArray(2);

	glBindVertexArray(0);
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
		
		glUniform1ui(glGetUniformLocation(shader->Program, "directionalGeom"), false);

		glDrawElementsInstanced(GL_TRIANGLES,											// rendering triangle primitives
								indices.size() - directionalIndicesCount,				// number of indices to be used in rendering
								GL_UNSIGNED_INT,										// indices array type is unsigned int
								(GLvoid*) (sizeof(GLuint) * directionalIndicesCount),   // byte offset into indices array bound to GL_ELEMENT_ARRAY_BUFFER
								seeds.size());											// number of instances to render
	glBindVertexArray(0);

	if (use_texture) tex->disable();
}
