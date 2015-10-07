#include "Trial.h"

#include <time.h> // time() for srand()

#define BIMAP_BASE_SIZE 100.f
#define EPSILON           0.001f

using namespace glm;

Trial::Trial(float xSize, float ySize, float density, float jitter, RenderMode renderMode) : xSize(xSize), ySize(ySize), density(density), jitter(jitter), renderMode(renderMode), cp(Slice(xSize, ySize))
{
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

	// position in middle of clipping volume and scale to fill screen
	cp.setPosition( 0.0f, 0.0f, -1000.0f );
	float temp = ( 827.f + 1000.f ) / 827.f;
	cp.setSize( temp, temp, temp );
}

void Trial::makeBiMap()
{
	// aspect ratio
	float ar = xSize / ySize;
	float bmMaxX, bmMaxY;

	if( ar > 1.0f ) {
		bmMaxX = BIMAP_BASE_SIZE;
		bmMaxY = BIMAP_BASE_SIZE / ar;
	}
	else {
		bmMaxX = BIMAP_BASE_SIZE * ar;
		bmMaxY = BIMAP_BASE_SIZE;
	}

	// free memory held by any existing BiMap
	if(bimap != NULL)
		delete bimap;

	std::cout << "Generating " << bmMaxX << " x " << bmMaxY << " bimap (AR = " << ar << ")... ";
	bimap = new BiMap( bmMaxX, bmMaxY );
	std::cout << "done" << std::endl;	
}

void Trial::sampleBiMap()
{
	shadowOffset = 0.f;

	float xStep = 1 / density;
	float yStep = 1 / density;

	std::cout << "Seeding the " << xSize << " x " << ySize << " cutting plane at a density of " << density << " glyphs/mm using the BiMap... ";
	
    for( float i = fmod( ( xSize / 2 ), xStep ); i < ( xSize + EPSILON ); i += xStep )
	{
        for( float j = fmod( ( ySize / 2 ), yStep ); j < ( ySize + EPSILON ); j += yStep )
        {
			float x_jitter;
            if( i < EPSILON )
                x_jitter = ( rand() % 2 ) * jitter;
            else if( abs( i - ( xSize - xStep ) ) < EPSILON )
                x_jitter = ( rand() % 2 - 1 ) * jitter;
            else
                x_jitter = ( rand() % 3 - 1 ) * jitter;

			float y_jitter;
            if( j < EPSILON )
                y_jitter = ( rand() % 2 ) * jitter;
            else if( abs( j - ( ySize - yStep ) ) < EPSILON )
                y_jitter = ( rand() % 2 - 1 ) * jitter;
            else
                y_jitter = ( rand() % 3 - 1 ) * jitter;

			Slice::Seed seed;
            seed.x = (float) i + ( x_jitter * xStep );
            seed.y = (float) j + ( y_jitter * yStep );

            bimap->getVecValues( ( seed.x / xSize ) * bimap->getXSize(), 
								 ( seed.y / ySize ) * bimap->getYSize(),
								 seed.dx, seed.dy, seed.dz);

			cp.addSeed( seed );

			if(seed.dz < shadowOffset) shadowOffset = seed.dz;
        }
	}

	std::cout << "done" << std::endl;
}

void Trial::testPattern()
{
	float xStep = 1 / density;
	float yStep = 1 / density;

	std::cout << "Seeding the " << xSize << " x " << ySize << " cutting plane at a density of " << density << " glyphs/mm using the test pattern... ";
	
    for( float i = fmod( ( xSize / 2 ), xStep ); i < ( xSize + EPSILON ); i += xStep )
	{
        for( float j = fmod( ( ySize / 2 ), yStep ); j < ( ySize + EPSILON ); j += yStep )
        {
			float x_jitter;
            if( i < EPSILON )
                x_jitter = ( rand() % 2 ) * jitter;
            else if( abs( i - ( xSize - xStep ) ) < EPSILON )
                x_jitter = ( rand() % 2 - 1 ) * jitter;
            else
                x_jitter = ( rand() % 3 - 1 ) * jitter;

			float y_jitter;
            if( j < EPSILON )
                y_jitter = ( rand() % 2 ) * jitter;
            else if( abs( j - ( ySize - yStep ) ) < EPSILON )
                y_jitter = ( rand() % 2 - 1 ) * jitter;
            else
                y_jitter = ( rand() % 3 - 1 ) * jitter;

			Slice::Seed seed;
            seed.x = (float) i + ( x_jitter * xStep );
            seed.y = (float) j + ( y_jitter * yStep );

			vec3 temp = normalize(vec3(seed.x, seed.y, 10.0) - vec3(xSize / 2, ySize / 2, 0.f)) * 10.f;

			seed.dx = temp.x;
			seed.dy = temp.y;
			seed.dz = temp.z;

			cp.addSeed( seed );
        }
	}

	std::cout << "done" << std::endl;
}

void Trial::setRenderMode(Trial::RenderMode renderMode)
{
	this->renderMode = renderMode;

	switch (renderMode)
	{
	case Trial::TRIAL_RENDER_LINES_PLAIN:
		cp.renderPL();
		break;
	case Trial::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_BLINN:
		cp.renderIL(ILines::ILLightingModel::IL_CYLINDER_BLINN);
		break;
	case Trial::TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_PHONG:
		cp.renderIL(ILines::ILLightingModel::IL_CYLINDER_PHONG);
		break;
	case Trial::TRIAL_RENDER_LINES_ILLUMINATED_MAXIMUM_PHONG:
		cp.renderIL(ILines::ILLightingModel::IL_MAXIMUM_PHONG);
		break;
	case Trial::TRIAL_RENDER_TUBES_PLAIN:
		cp.renderPT();
		break;
	case Trial::TRIAL_RENDER_TUBES_RINGED:
		cp.renderRT(8, 1.f, vec3(1.f,1.f,1.f), vec3(.1f,.1f,.1f) );
		break;
	case Trial::TRIAL_RENDER_SHADOWED_HEDGEHOGS:
		cp.renderSH();
		break;
	}
}

Trial::RenderMode Trial::getRenderMode()
{
	return this->renderMode;
}

void Trial::setShader( Shader *shader )
{
	cp.setShader( shader );
}

void Trial::passThroughPVMatrix( float *pM, float *vM )
{
	cp.setILPVMatrix( pM, vM );
}

float Trial::getShadowOffset()
{
	return shadowOffset;
}

void Trial::display()
{
	cp.redraw();
}