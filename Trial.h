#pragma once

#include <vector>
#include "BiMap.h"
#include "Slice.h"

class Trial
{
public:
	enum RenderMode {
		TRIAL_RENDER_LINES_PLAIN,
		TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_BLINN,
		TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_PHONG,
		TRIAL_RENDER_LINES_ILLUMINATED_MAXIMUM_PHONG,
		TRIAL_RENDER_TUBES_PLAIN,
		TRIAL_RENDER_TUBES_RINGED,
		TRIAL_RENDER_SHADOWED_HEDGEHOGS
	};

	Trial(float xSize = 10, float ySize = 10, float density = 1.0f, float jitter = 0.25f, RenderMode renderMode = TRIAL_RENDER_LINES_ILLUMINATED_CYLINDER_BLINN);
	~Trial();

	void init();

	void makeBiMap();
	void sampleBiMap();

	void testPattern();

	void setRenderMode(Trial::RenderMode renderMode);
	Trial::RenderMode getRenderMode();

	void setShader( Shader *shader );

	void passThroughPVMatrix( float *pM, float *vM );

	float getShadowOffset();

	void display();

private:
	float xSize, ySize;

	float jitter, density;

	RenderMode renderMode;

	BiMap* bimap;

	float shadowOffset;

	Slice cp;
};

