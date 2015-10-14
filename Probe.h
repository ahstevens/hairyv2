#pragma once
#include "Object.h"
#include <vector>



class Probe : public Object
{
public:

	Probe( void );
	~Probe( void );

	void renderPT( int segments = 8);
	void renderRT( int segments = 8, float stripe_pairs_per_mm = 1.f, 
		glm::vec3 stripe_color1 = glm::vec3(1.f, 1.f, 1.f),
		glm::vec3 stripe_color2 = glm::vec3(0.f, 0.f, 0.f));

	virtual void redraw();

private:
	void generateProbe( int segments = 8 );

	std::vector<glm::vec2> circle( int segments );
	
	std::vector<GLfloat> vertices_flat;     // a float array of vertices for Illumnated Lines
	std::vector<GLsizei> first;               // the array of starting indices for Illuminated Lines
	std::vector<glm::vec3> instances;

	GLsizei directionalIndicesCount;      // number of indices used to render the directionality geometry
	
	bool geometryChange, tubesGenerated;
};