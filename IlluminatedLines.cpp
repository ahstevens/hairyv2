#include "IlluminatedLines.h"


IlluminatedLines::IlluminatedLines(int lineCount, int totalSize, std::vector<int> first, std::vector<int> vertCount, std::vector<float> vertices, float *colors, ILines::ILLightingModel::Model lightModel)
{
	this->lineCount = lineCount;
	this->totalSize = totalSize;
	this->first = first;
	this->vertCount = vertCount;
	this->vertices = vertices;
	this->colors = colors;
	this->lightingModel = lightModel;

	isInitialized = false;
	lightingChecked = false;

	texDim = 256;
	ka = 0.05f;
	kd = 0.8f;
	ks = 1.0f;
	gloss = 10.0f;
	
	uniformColor[0] = 1.0f;
	uniformColor[1] = 0.6f;
	uniformColor[2] = 0.1f;
	uniformColor[3] = 0.55f;

	lightDirection[0] = -1.0f;
	lightDirection[1] = -1.0f;
	lightDirection[2] = -1.0f;
	lightDirection[3] =  0.0f;

	lightPosition[0] = 1.0f;
	lightPosition[1] = 1.0f;
	lightPosition[2] = 1.0f;
	lightPosition[3] = 0.0f;

	dataHasColors = false;

	doColors = false;
}


IlluminatedLines::~IlluminatedLines()
{
	ILines::ILRender::deleteIdentifier(ilID);
	//delete[] vertices;
	delete[] colors;
}

void IlluminatedLines::setPVMatrix(float *pM, float *vM)
{
	this->pM = pM;
	this->vM = vM;
}

void IlluminatedLines::init()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(pM);

	initGL();
	initIL();
}

void IlluminatedLines::initGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	glGenBuffersARB(1, &VBO);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBO);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 3 * sizeof(vertices[0]) * totalSize, &(vertices.front()), GL_STATIC_DRAW_ARB);
	
	if (dataHasColors)
	{
		glGenBuffersARB(1, &VBOcol);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBOcol);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4 * sizeof(colors[0]) * totalSize, colors, GL_STATIC_DRAW_ARB);
	}
	
}

void IlluminatedLines::initIL()
{
	if (!lightingChecked)
	{
		maximumPhongSupported = ILines::ILRender::isLightingModelSupported(ILines::ILLightingModel::IL_MAXIMUM_PHONG);
		cylinderBlinnSupported = ILines::ILRender::isLightingModelSupported(ILines::ILLightingModel::IL_CYLINDER_BLINN);
		cylinderPhongSupported = ILines::ILRender::isLightingModelSupported(ILines::ILLightingModel::IL_CYLINDER_PHONG);

		maximumPhongIL.setErrorCallback(errorCallbackIL);
		cylinderBlinnIL.setErrorCallback(errorCallbackIL);
		cylinderPhongIL.setErrorCallback(errorCallbackIL);

		lightingChecked = true;
	}

	if ( cylinderBlinnSupported )
	{
		if ( lightingModel == ILines::ILLightingModel::IL_CYLINDER_BLINN )
		{
			std::cout << "Setting up textures for the cylinder averaging Phong/Blinn lighting model...";
			cylinderBlinnIL.setupTextures(ka, kd, ks, 4.0f * gloss, texDim,
				ILines::ILLightingModel::IL_CYLINDER_BLINN, false);
			std::cout << " done." << std::endl;

			curIL = &cylinderBlinnIL;

			return;
		}
	}
	else
		std::cout << "Cylinder averaging Phong/Blinn lighting model not supported." << std::endl;

	if (cylinderPhongSupported)
	{
		if( lightingModel == ILines::ILLightingModel::IL_CYLINDER_PHONG )
		{
			std::cout << "Setting up textures for the cylinder averaging Phong lighting model...";
			cylinderPhongIL.setupTextures(ka, kd, ks, gloss, texDim,
				ILines::ILLightingModel::IL_CYLINDER_PHONG, false,
				lightDirection);
			std::cout << " done." << std::endl;

			curIL = &cylinderPhongIL;

			return;
		}
	}
	else
		std::cout << "Cylinder averaging Phong lighting model not supported." << std::endl;

	if (maximumPhongSupported)
	{
		if ( lightingModel == ILines::ILLightingModel::IL_MAXIMUM_PHONG )
		{
			std::cout << "Setting up textures for the maximum principle Phong lighting model...";
			maximumPhongIL.setupTextures(ka, 0.6f * kd, 0.3f * ks, gloss, texDim,
				ILines::ILLightingModel::IL_MAXIMUM_PHONG, false,
				lightDirection);
			std::cout << " done." << std::endl;

			curIL = &maximumPhongIL;

			return;
		}
	}
	else
		std::cout << "Maximum principle Phong lighting model not supported." << std::endl;
	
	if (curIL == NULL)
	{
		std::cerr << "Could not find a supported lighting model!" << std::endl;
	}
}

void IlluminatedLines::errorCallbackIL(ILines::ILRender *ilRender)
{
	ILines::ILRender::ILError err;

	err = ilRender->getError();

	std::cout << "IL Error : " << ILines::ILRender::errorString(err) << std::endl;

	if (err == ILines::ILRender::IL_GL_ERROR)
		std::cout << "GL Error : " << gluErrorString(ilRender->getGLError()) << std::endl;

}

void IlluminatedLines::redraw(Shader shader)
{
	/* Handle rotations and translations separately. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Specifiy the light position in eye coordinates. */
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	//gluLookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
	//	sceneCenter.x, sceneCenter.y, sceneCenter.z,
	//	cameraUp.x, cameraUp.y, cameraUp.z);
	glMultMatrixf(vM);

	// translate object to middle of viewing frustum 
	glTranslatef(0.0f, 0.0f, -500.0f);

	// scale it to fill screen
	float temp = (560.f + 500.f) / 560.f;
	glScalef(temp, temp, temp);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	displayScene();
}


void IlluminatedLines::displayScene()
{
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBO);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	if (doColors && dataHasColors)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBOcol);
		glColorPointer( 4, GL_FLOAT, 4 * sizeof(GLfloat), 0);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	else
		glDisableClientState(GL_COLOR_ARRAY);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(uniformColor);


	if (!isInitialized)
	{
		isInitialized = true;
		ilID = ILines::ILRender::prepareMultiDrawArrays(&(first.front()), &(vertCount.front()), lineCount);
	}

	curIL->enableZSort(true);
	
	glDepthMask(GL_FALSE);

	curIL->multiDrawArrays(ilID);

	glDepthMask(GL_TRUE);
}

void IlluminatedLines::setLightingModel(ILines::ILLightingModel::Model lightModel)
{
	this->lightingModel = lightModel;
	initIL();
}