#include "IlluminatedLines.h"


IlluminatedLines::IlluminatedLines(int lineCount, int totalSize, std::vector<int> first, std::vector<int> vertCount, std::vector<float> vertices, float *colors)
{
	this->lineCount = lineCount;
	this->totalSize = totalSize;
	this->first = first;
	this->vertCount = vertCount;
	this->vertices = vertices;
	this->colors = colors;

	texDim = 256;
	ka = 0.05f;
	kd = 0.8f;
	ks = 1.0f;
	gloss = 10.0f;

	rotX = rotY = zoomZ = transX = transY = 0;

	uniformColor[0] = 1.0f;
	uniformColor[1] = 0.5f;
	uniformColor[2] = 0.0f;
	uniformColor[3] = 0.4f;

	lightDirection[0] = -1.0f;
	lightDirection[1] = -1.0f;
	lightDirection[2] = -1.0f;
	lightDirection[3] =  0.0f;

	lightPosition[0] = 1.0f;
	lightPosition[1] = 1.0f;
	lightPosition[2] = 1.0f;
	lightPosition[3] = 0.0f;

	dataHasColors = false;

	cameraPosition = ILines::Vector3f(0.0f, 0.0f, 560.0f);
	sceneCenter = ILines::Vector3f(0.0f, 0.0f, 0.0f);
	cameraUp = ILines::Vector3f(0.0f, 1.0f, 0.0f);
	cameraPerspective[0] = 45.0;
	cameraPerspective[1] = 1.0;
	cameraPerspective[2] = 1.0;
	cameraPerspective[3] = 1000.0;

	doColors = false;
}


IlluminatedLines::~IlluminatedLines()
{
	ILines::ILRender::deleteIdentifier(ilID);
	//delete[] vertices;
	delete[] colors;
}

void IlluminatedLines::init()
{


	initGL();
	initIL();

	Y = ILines::normalize(cameraUp);
	Z = ILines::normalize(cameraPosition - sceneCenter);
	X = ILines::normalize(ILines::cross(Y, Z));
}

void IlluminatedLines::initGL()
{
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
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
	maximumPhongSupported = ILines::ILRender::isLightingModelSupported(ILines::ILLightingModel::IL_MAXIMUM_PHONG);
	cylinderBlinnSupported = ILines::ILRender::isLightingModelSupported(ILines::ILLightingModel::IL_CYLINDER_BLINN);
	cylinderPhongSupported = ILines::ILRender::isLightingModelSupported(ILines::ILLightingModel::IL_CYLINDER_PHONG);

	maximumPhongIL.setErrorCallback(errorCallbackIL);
	cylinderBlinnIL.setErrorCallback(errorCallbackIL);
	cylinderPhongIL.setErrorCallback(errorCallbackIL);
	
	if (maximumPhongSupported)
	{
		std::cout << "Setting up textures for the maximum principle Phong lighting model...";
		maximumPhongIL.setupTextures(ka, 0.6f * kd, 0.3f * ks, gloss, texDim,
			ILines::ILLightingModel::IL_MAXIMUM_PHONG, false,
			lightDirection);
		std::cout << " done." << std::endl;
	}
	else
		std::cout << "Maximum principle Phong lighting model not supported." << std::endl;

	if (cylinderBlinnSupported)
	{
		std::cout << "Setting up textures for the cylinder averaging Phong/Blinn lighting model...";
		cylinderBlinnIL.setupTextures(ka, kd, ks, 4.0f * gloss, texDim,
			ILines::ILLightingModel::IL_CYLINDER_BLINN, false);
		std::cout << " done." << std::endl;
	}
	else
		std::cout << "Cylinder averaging Phong/Blinn lighting model not supported." << std::endl;

	if (cylinderPhongSupported)
	{
		std::cout << "Setting up textures for the cylinder averaging Phong lighting model...";
		cylinderPhongIL.setupTextures(ka, kd, ks, gloss, texDim,
			ILines::ILLightingModel::IL_CYLINDER_PHONG, false,
			lightDirection);
		std::cout << " done." << std::endl;
	}
	else
		std::cout << "Cylinder averaging Phong lighting model not supported." << std::endl;


	if (cylinderBlinnSupported)
	{
		curIL = &cylinderBlinnIL;
		lightingModel = ILines::ILLightingModel::IL_CYLINDER_BLINN;
	}
	else if (maximumPhongSupported)
	{
		curIL = &maximumPhongIL;
		lightingModel = ILines::ILLightingModel::IL_MAXIMUM_PHONG;
	}
	else if(cylinderPhongSupported)
	{
		curIL = &cylinderPhongIL;
		lightingModel = ILines::ILLightingModel::IL_CYLINDER_PHONG;
	}

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
	float	mvMatrix[16];

	glGetFloatv(GL_MODELVIEW_MATRIX, mvMatrix);

	/* Handle rotations and translations separately. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(rotY, Y.x, Y.y, Y.z);
	glRotatef(rotX, X.x, X.y, X.z);
	glMultMatrixf(mvMatrix);

	/* Save the rotational component of the modelview matrix. */
	glPushMatrix();
	glLoadIdentity();

	/* Specifiy the light position in eye coordinates. */
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	gluLookAt(cameraPosition.x, cameraPosition.y, cameraPosition.z,
		sceneCenter.x, sceneCenter.y, sceneCenter.z,
		cameraUp.x, cameraUp.y, cameraUp.z);

	ILines::Vector3f trans;
	trans = zoomZ / 20.0f * Z + transX / 150.0f * X - transY / 150.0f * Y;
	glTranslatef(trans.x, trans.y, trans.z);
	glTranslatef(+sceneCenter.x, +sceneCenter.y, +sceneCenter.z);
	glRotatef(rotY, Y.x, Y.y, Y.z);
	glRotatef(rotX, X.x, X.y, X.z);
	glMultMatrixf(mvMatrix);
	glTranslatef(-sceneCenter.x, -sceneCenter.y, -sceneCenter.z);

	rotX = rotY = 0;

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(uniformColor);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	static bool	isInitialized = false;

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