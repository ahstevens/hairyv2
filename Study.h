#pragma once

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <string>
#include "Camera.h"
#include "Light.h"
#include "Trial.h"
#include "Polhemus.h"

class Study
{
public:
	static Study* getInstance( GLFWwindow* window );

	void init(GLfloat width_mm, GLfloat height_mm, GLfloat dist_mm);
	void training();
	void begin();
	void next();
	void end();
	
private:
	Study( GLFWwindow* window );
	static Study* instance;

	~Study();

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	void key_process(GLFWwindow* window, int key, int scancode, int action, int mode);

	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_process(GLFWwindow* window, double xpos, double ypos);

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void scroll_process(GLFWwindow* window, double xoffset, double yoffset);

	void do_movement();

	void generateTrial(Trial::RenderMode renderMode);

	glm::vec3 getRandomOrientation();

	Polhemus* polhemus;
	//Probe probe;

	GLFWwindow* window;
	GLfloat windowWidth, windowHeight, eyeDistance;

	bool keys[1024], firstMouse;
	
	GLfloat lastX, lastY;
	GLfloat deltaTime;		// Time between current frame and last frame
	GLfloat lastFrame;		// Time of last frame

	int draw_halos, cycle_light, draw_probe;

	GLfloat lengthMultiplier, thicknessMultiplier, directionalGeomScale, haloSize, hedgehogOffset;
	
	Camera camera;
	Light light;
	
	Slice probe;

	std::string participant;
	Trial trial;
	int trialNum;
	float timeStart;

};

