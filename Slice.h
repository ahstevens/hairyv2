#pragma once
#include "Object.h"
#include <vector>

struct Seed {
	GLfloat x, y;
	GLfloat dx, dy, dz;
};

class Slice : public Object
{
public:
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

	virtual void redraw( Shader shader );

private:
	std::vector<Seed> seeds;
	float width, height;
};

