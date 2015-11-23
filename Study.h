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
#include "Probe.h"
#include "Stopwatch.h"
#include "Target.h"

class Study
{
public:
	static Study* getInstance( GLFWwindow* window );

	void init(std::string name, GLfloat width_mm, GLfloat height_mm, GLfloat dist_mm);
	void training();
	void begin();
	void next();
	void end();
	
private:
	enum Mode {
		DEMO,
		TRAINING,
		STUDY,
		PAUSED,
		NONE
	};

	struct Condition {
		Trial::RenderMode renderMode;
		float density;
		float lengthMultiplier;
		float thicknessMultiplier;
		float glyphHeadMultiplier;
		
		Condition(Trial::RenderMode renderMode, float density, float lengthMultiplier, float thicknessMultiplier, float glyphHeadMultiplier) 
			: renderMode(renderMode), density(density), lengthMultiplier(lengthMultiplier), thicknessMultiplier(thicknessMultiplier), glyphHeadMultiplier(glyphHeadMultiplier)
		{}
	};

	Study( GLFWwindow* window );
	static Study* instance;

	~Study();

	void mainLoop();

	void render();

	void setupPL();
	void setupIL();
	void setupSH();
	void setupTubes();

	void initGL(Shader *s);

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	void key_process(GLFWwindow* window, int key, int scancode, int action, int mode);

	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_process(GLFWwindow* window, double xpos, double ypos);

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void scroll_process(GLFWwindow* window, double xoffset, double yoffset);

	void do_movement();

	void generateTrial( Trial::RenderMode renderMode );

	glm::quat getAdjustedTargetCursorOrientation();

	float getAngleError(glm::quat probe, glm::quat target);

	void updateTrainingTarget();
	glm::quat getRandomOrientation();

	glm::quat vecsToQuat(glm::vec3 u, glm::vec3 v);

	bool fileExists( const std::string &fname );

	void prepareOutput( std::string name );
	void recordTrial( bool trainingTrial = false );

	GLFWwindow* window;
	GLfloat windowWidth, windowHeight, eyeDistance;

	bool keys[1024], firstMouse;
	
	GLfloat lastX, lastY;
	GLfloat deltaTime;		// Time between current frame and last frame
	GLfloat lastFrame;		// Time of last frame

	Polhemus* polhemus;

	Stopwatch stopwatch;

	std::vector< std::vector< std::vector< Condition > > > blocks;

	std::ofstream outFile;
	std::string outFileName;

	Mode mode;

	int draw_halos, cycle_light, probe_training, draw_probe, show_probe_hints, target_on_top, training_target_random, old_hog;

	float density, jitter;

	glm::vec3 bgColor;

	Shader *lightingShader, *haloShader, *hogShader, *hogShaderRev, *lineShader, *targetShader;

	GLfloat lengthMultiplier, thicknessMultiplier, directionalGeomScale, haloSize, hedgehogOffset;
	
	Camera camera;
	Light light;
	
	Probe probe, trainingTarget;

	Target targetCursor;

	std::string participant;
	Trial trial;
	int trialNum;
	float timeStart;

};

