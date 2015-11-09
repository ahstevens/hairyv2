#pragma once

#include <vector>
#include "Object.h"
#include "BiMap.h"
#include "Shader.h"
#include "IlluminatedLines.h"

class Trial : public Object
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

	struct Seed {
		GLfloat x, y;
		GLfloat dx, dy, dz;
		GLfloat twist;

		float length() { return sqrt(dx*dx + dy*dy + dz*dz); }
	};

	Trial(float xSize = 10, float ySize = 10, float density = 1.0f, float jitter = 0.25f, RenderMode renderMode = LINES_PLAIN);
	~Trial();

	void init();

	void makeBiMap();
	void sampleBiMap();
	Seed getRandomSeed();

	void setRenderMode(RenderMode renderMode, float lengthMultiplier = 1.f);
	RenderMode getRenderMode();

	void setJitter(float jitter);

	void setDensity(float density);

	void passThroughPVMatrix( float *pM, float *vM );

	float getShadowOffset();

	float getMaxLength();

	void addSeed(Seed s);                   // Add a seed to the Slice
	void clearSeeds(void);                  // Clear all seeds from the Slice

	void redraw();

private:
	void generateTubes(int segments = 8);
	void generateHairs();
	GLsizei insertDirectionalGeometry(std::vector<Vertex> &v, std::vector<GLuint> &i, GLsizei &offset);
	std::vector<glm::vec2> circle(int segments);

	float xSize, ySize;

	float jitter, density, maxLength;

	RenderMode renderMode;

	BiMap* bimap;

	IlluminatedLines *il;

	std::vector<Seed> seeds;              // the seeds to populate the Slice

	std::vector<glm::vec3> instances;

	float shadowOffset;

	GLsizei directionalIndicesCount;      // number of indices used to render the directionality geometry

	bool doIL, ilInit, doLines, geometryChange, tubesGenerated, linesGenerated, ilGenerated, directionality;
};

