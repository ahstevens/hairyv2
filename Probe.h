#pragma once
#include "Object.h"
#include <vector>



class Probe : public Object
{
public:

	Probe( float length, float width );
	~Probe( void );

	void init();

	virtual void redraw();

private:
	void generateProbe();

	std::vector<glm::vec2> circle( int segments );
	
	std::vector<GLfloat> vertices_flat;     // a float array of vertices for Illumnated Lines
	std::vector<GLsizei> first;               // the array of starting indices for Illuminated Lines
	std::vector<glm::vec3> instances;

	GLsizei directionalIndicesCount;      // number of indices used to render the directionality geometry	

	float length, width;
};