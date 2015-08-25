#include "Trial.h"

#include <time.h> // time() for srand()


using namespace glm;

Trial::Trial(float xSize, float ySize, float density, float jitter) : xSize(xSize), ySize(ySize), density(density), jitter(jitter), cp(Slice(xSize, ySize))
{
}


Trial::~Trial()
{
}


void Trial::init()
{
	srand((unsigned int) time(NULL));
	
	std::cout << "Generating bimap..." << std::endl;
	bimap = new BiMap(xSize, ySize);
	//std::cout << "Normalizing bimap..." << std::endl;
	//bimap->normalize();
	
	std::cout << "Seeding the cutting plane..." << std::endl;

	float x_jitter, y_jitter;
    for(int i = 0; i < xSize; ++i)
	{
        for(int j = 0; j < ySize; ++j)
        {
            if( i == 0 )
                x_jitter = ( rand() % 2 ) * jitter;
            else if( i == xSize - 1 )
                x_jitter = ( rand() % 2 - 1 ) * jitter;
            else
                x_jitter = ( rand() % 3 - 1 ) * jitter;

            if( j == 0 )
                y_jitter = ( rand() % 2 ) * jitter;
            else if( j == ySize - 1 )
                y_jitter = ( rand() % 2 - 1 ) * jitter;
            else
                y_jitter = ( rand() % 3 - 1 ) * jitter;

			Slice::Seed seed;
            seed.x = (float) i + x_jitter;
            seed.y = (float) j + y_jitter;

            bimap->getVecValues( seed.x, seed.y, seed.dx, seed.dy, seed.dz);

			cp.addSeed( seed );
        }
	}	

	delete bimap;
	std::cout << "Cutting plane seeded." << std::endl;

	cp.generateTubes( 8, 0.5f, 0.1f );
	cp.setPosition( -(xSize / 2), -(ySize / 2), -10.0f );
}

void Trial::display( Shader shader )
{
	cp.redraw( shader );
}