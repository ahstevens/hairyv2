#pragma once
#include "Object.h"
#include "IlluminatedLines.h"
#include <vector>



class Slice : public Object
{
public:
	struct Seed {
		GLfloat x, y;
		GLfloat dx, dy, dz;
		GLfloat twist;

		float length() { return sqrt( dx*dx + dy*dy + dz*dz ); }
	};

	Slice( void );
	Slice( float width, float height );
	Slice( float width, float height, std::vector<Seed> seeds );
	~Slice( void );

	void setWidth( float width );
	float getWidth( void );
	void setHeight( float height );
	float getHeight( void );

	void addSeed( Seed s );                   // Add a seed to the Slice
	void addSeeds( std::vector<Seed> seeds ); // Add seeds to the Slice
	void removeSeed( void );                  // Removes last seed added to the Slice
	void removeSeeds( int n );                // Removes last n seeds added to the Slice
	void clearSeeds( void );                  // Clear all seeds from the Slice
	int seedCount();                          // Returns the number of seeds in the Slice

	void updateOrientation(glm::vec3 orientation);

	void renderIL( ILines::ILLightingModel::Model lightModel, float lengthMultiplier = 1.0f );
	void renderPL( float lengthMultiplier = 1.0f );
	void renderPT( int segments = 8);
	void renderRT( int segments = 8, float stripe_pairs_per_mm = 1.f, 
		glm::vec3 stripe_color1 = glm::vec3(1.f, 1.f, 1.f),
		glm::vec3 stripe_color2 = glm::vec3(0.f, 0.f, 0.f));
	void renderSH( float lengthMultiplier = 1.0f );

	void setILPVMatrix(float * pM, float *vM);

	virtual void redraw();

private:
	void generateTubes( int segments = 8 );
	void generateHairs( float lengthMultiplier = 1.0f );

	std::vector<glm::vec2> circle( int segments );

	IlluminatedLines *il;

	std::vector<Seed> seeds;              // the seeds to populate the Slice

	std::vector<GLfloat> vertices_flat;     // a float array of vertices for Illumnated Lines
	std::vector<GLsizei> first;               // the array of starting indices for Illuminated Lines
	std::vector<glm::vec3> instances;

	GLsizei directionalIndicesCount;      // number of indices used to render the directionality geometry

	float width, height;                  // dimensions of the slice

	bool doIL, ilInit, geometryChange, tubesGenerated, linesGenerated, directionality;
};