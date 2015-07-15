#include "Trial.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI
#include <time.h> // time() for srand()

#include "SweepSurface.h"

using namespace glm;

Trial::Trial(int xSize, int ySize, float density, float jitter) : xSize(xSize), ySize(ySize), density(density), jitter(jitter), camera(glm::vec3((float)(xSize-1)/2, (float)(ySize-1)/2, 25.0f)), light(glm::vec3(1.0f, 1.0f, 1.0f)), bimap((float)xSize, (float)ySize)
{
	init();
}


Trial::~Trial()
{
}


void Trial::init()
{
	srand((unsigned int) time(NULL));

	std::vector<glm::vec2> poly = circle(8);

	float x, y, x_jitter, y_jitter, dx, dy, dz;
    for(int i = 0; i < xSize; ++i)
	{
		std::vector<gridpoint_t> column;
        for(int j = 0; j < ySize; ++j)
        {
			gridpoint_t temp;

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


            x = (float) i + x_jitter;
            y = (float) j + y_jitter;

            bimap.getVecValues( x, y, dx, dy, dz);

			temp.x = x;
			temp.y = y;
            temp.x_jitter = x_jitter;
            temp.y_jitter = y_jitter;
            temp.dx = dx;
            temp.dy = dy;
            temp.dz = dz;

			column.push_back(temp);
			
			std::vector<glm::vec3> path;
			path.push_back( glm::vec3( 0.0f, 0.0f, 0.0f ) );
            path.push_back( glm::vec3( dx, dy, dz) );

			std::vector<glm::vec2> scales;
			scales.push_back( glm::vec2( 1.0f, 1.0f ) );
			scales.push_back( glm::vec2( 1.0f, 1.0f ) );

			std::vector<float> rots;
			rots.push_back( 0.0f );			
			rots.push_back( 0.0f );            
			
            SweepSurface sweepTemp(poly, path, scales, rots);
			sweepTemp.setSize( 0.5, 0.5, 0.5 );
			sweepTemp.setColor( ((float)i) / xSize, ((float)j ) / ySize, 0.5f);
			sweepTemp.setPosition( x, y, 0.0f );
			if( dz < 0 )
                sweepTemp.setRotate( 180, 0, 1, 0 );

            objects.push_back( sweepTemp );
        }
		grid.push_back( column );
	}	
}

std::vector<SweepSurface> Trial::getObjects() { return objects; }

std::vector<vec2> Trial::circle(int segments)
{
    float angleIncrement = 2.0f * (float) M_PI / (float) segments;
    
    std::vector<vec2> circle;

    for( int i = segments - 1; i >= 0; --i )
        circle.push_back(vec2(float(sin(i * angleIncrement)) * 0.5f, 
                              float(cos(i * angleIncrement)) * 0.5f));

    return circle;
}