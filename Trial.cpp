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

	setRenderMode(renderMode);

	// position in middle of clipping volume and scale to fill screen
	cp.setPosition( 0.0f, 0.0f, -500.0f );
	float temp = ( 560.f + 500.f ) / 560.f;
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
        }
	}

	std::cout << "done" << std::endl;
}

void Trial::setRenderMode(Trial::RenderMode renderMode, ILines::ILLightingModel::Model lightModel)
{
	this->renderMode = renderMode;

	switch (renderMode)
	{
	case Trial::TRIAL_RENDER_LINES_PLAIN:
		cp.renderPL();
		break;
	case Trial::TRIAL_RENDER_LINES_ILLUMINATED:
		cp.renderIL(lightModel);
		break;
	case Trial::TRIAL_RENDER_TUBES_PLAIN:
		cp.renderPT();
		break;
	case Trial::TRIAL_RENDER_TUBES_RINGED:
		cp.renderRT();
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

void Trial::setSliceShader( Shader *shader )
{
	cp.setShader( shader );
}

void Trial::passThroughPVMatrix( float *pM, float *vM )
{
	cp.setILPVMatrix( pM, vM );
}

void Trial::display()
{
	cp.redraw();
}