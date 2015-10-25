#pragma once

#include <vector>
#include "BiMap.h"
#include "Slice.h"

class Trial
{
public:
	enum RenderMode {
		LINES_PLAIN,
		LINES_ILLUMINATED_CYLINDER_BLINN,
		LINES_ILLUMINATED_CYLINDER_PHONG,
		LINES_ILLUMINATED_MAXIMUM_PHONG,
		TUBES_PLAIN,
		TUBES_RINGED,
		SHADOWED_HEDGEHOGS
	};

	Trial(float xSize = 10, float ySize = 10, float density = 1.0f, float jitter = 0.25f, RenderMode renderMode = LINES_ILLUMINATED_CYLINDER_BLINN);
	~Trial();

	void init();

	void makeBiMap();
	void sampleBiMap();

	void testPattern();

	void setRenderMode(Trial::RenderMode renderMode);
	Trial::RenderMode getRenderMode();

	void setShader( Shader *shader );

	void setJitter(float jitter);

	void setDensity(float density);

	void passThroughPVMatrix( float *pM, float *vM );

	float getShadowOffset();

	float getMaxLength();

	glm::quat getTargetOrientation();

	void display();

private:
	float xSize, ySize;

	float jitter, density, maxLength;

	RenderMode renderMode;

	BiMap* bimap;

	float shadowOffset;

	Slice cp;
};

