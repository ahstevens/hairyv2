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

	void generateTubes( int segments = 8, float thickness = 1.0f, float lengthMultiplier = 1.0f );
	void generateHairs( float lengthMultiplier = 1.0f );

	void renderIL(ILines::ILLightingModel::Model lightModel);
	void renderPL();
	void renderRT();
	void renderPT();
	void renderSH();

	void setILPVMatrix(float * pM, float *vM);

	virtual void redraw( Shader shader );

private:	
	std::vector<glm::vec2> circle( int segments );

	IlluminatedLines *il;

	std::vector<Seed> seeds;              // the seeds to populate the Slice
	std::vector<Vertex> vertices;         // position/normal/tex_coords of each seed's geometry
	std::vector<GLuint> indices;          // indices for rendering each seed's geometry
	std::vector<GLvoid*> indices_offsets; // pointers to beginning of each seed's indices in the indices array
	std::vector<GLsizei> counts;          // holds the number of indices for each geometry primitive
	float width, height;                  // dimensions of the slice

	bool doIL, ilInit;
};

