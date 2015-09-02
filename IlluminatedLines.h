#pragma once
#include "Object.h"
#include "ILines/ILRender.h"
#include "ILines/Vector.h"
#include <vector>

class IlluminatedLines :
	public Object
{
public:
	IlluminatedLines(int lineCount, int totalCount, std::vector<int> first, std::vector<int> vertCount, std::vector<float> vertices, float *colors = NULL);
	~IlluminatedLines();
		
	void init();

	static void errorCallbackIL(ILines::ILRender *ilRender);

	virtual void redraw(Shader shader);

private:
	void initGL();
	void initIL();
	void displayScene();
		
	bool isInitialized;

	GLuint VBOcol;
	
	ILines::Vector3f X, Y, Z;

	int	texDim;
	float ka, kd, ks, gloss;

	float uniformColor[4];

	GLfloat	lightDirection[4];
	GLfloat	lightPosition[4];

	bool dataHasColors;
	
	ILines::Vector3f	cameraPosition;
	ILines::Vector3f	sceneCenter;
	ILines::Vector3f	cameraUp;

	ILines::ILRender maximumPhongIL;
	ILines::ILRender cylinderPhongIL;
	ILines::ILRender cylinderBlinnIL;
	ILines::ILRender *curIL;

	bool maximumPhongSupported;
	bool cylinderBlinnSupported;
	bool cylinderPhongSupported;

	bool doColors;

	int	lineCount;
	std::vector<int> first;
	std::vector<int> vertCount;
	std::vector<float> vertices;
	float *colors;
	int	totalSize;

	ILines::ILRender::ILIdentifier	ilID;
	ILines::ILLightingModel::Model	lightingModel;
};

