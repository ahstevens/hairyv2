#pragma once

#include <vector>
#include "BiMap.h"
#include "Slice.h"

class Trial
{
public:
	enum RenderMode {
		TRIAL_RENDER_LINES_PLAIN,
		TRIAL_RENDER_LINES_ILLUMINATED,
		TRIAL_RENDER_TUBES_PLAIN,
		TRIAL_RENDER_TUBES_RINGED,
		TRIAL_RENDER_SHADOWED_HEDGEHOGS
	};

	Trial(float xSize = 10, float ySize = 10, float density = 1.0f, float jitter = 0.25f);
	~Trial();

	void init();

	void setRenderMode(Trial::RenderMode renderMode);
	Trial::RenderMode getRenderMode();

	void display( Shader shader );

private:
	float xSize, ySize;

	float jitter, density;

	RenderMode renderMode;

	BiMap* bimap;

	Slice cp;
};

