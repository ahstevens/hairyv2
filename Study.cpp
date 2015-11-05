#include "Study.h"
#include <glm/gtc/type_ptr.hpp>
#include "Slice.h"
#include <time.h> // time() for srand()
#include <random>

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

#define NBLOCKS 5
#define NREPLICATESPERBLOCK 5
#define NDENSITYCONDITIONS 3
#define NRENDERINGCONDITIONS 5
#define NTRIALSPERBLOCK NREPLICATESPERBLOCK * NDENSITYCONDITIONS * NRENDERINGCONDITIONS

#define BGCOLOR glm::vec3(0.325f, 0.486f, 0.812f)
//#define BGCOLOR glm::vec3(0.f, 0.f, 0.f)

// Initialize class variables
Study* Study::instance = NULL;

// Returns singleton Study instance
Study* Study::getInstance( GLFWwindow* window )
{
    if ( !instance )
        instance = new Study( window );
    return instance;
}

Study::Study( GLFWwindow* window ) : probe(Probe(80.f, 20.f)), trainingTarget(Probe(80.f, 20.f)), target(Target( glm::vec3(1.f, 0.f, 0.f) ) )
{
	int width_px, height_px;
	glfwGetWindowSize(window, &width_px, &height_px);
	this->window = window;
	firstMouse = true;
	lastX  =  width_px  / 2.0f;
    lastY  =  height_px / 2.0f;
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;  	// Time of last frame

	draw_halos = cycle_light = probe_training = draw_probe = show_probe_hints = target_on_top = 0;

	density = 0.1f;
	jitter = 0.25f;
	renderMode = Trial::RenderMode::LINES_PLAIN;

	lengthMultiplier = thicknessMultiplier = directionalGeomScale = 1.f;
	haloSize = 0.5f;
	hedgehogOffset = 0.f;

	polhemus = Polhemus::getInstance();

	camera = Camera();
	light = Light(glm::vec3(1.0, 1.0, 1.0));

	// initialize key array
	for (int i = 0; i < 1024; ++i)
		keys[i] = 0;
	
	bgColor = BGCOLOR;

    // Build and compile our shader programs
	lightingShader = new Shader("materials.vert", "materials.frag");
	lineShader = new Shader("lines.vert", "lines.frag");
	haloShader = new Shader("halo.vert", "halo.frag");
	hogShader = new Shader("hedgehogs.vert", "hedgehogs.frag");
	targetShader = new Shader("target.vert", "target.frag");
    normalShader = new Shader("normals.vert", "normals.frag", "normals.geom");
}

Study::~Study()
{
	delete lightingShader;
	delete haloShader;
	delete hogShader;
	delete normalShader;
}

void Study::init(std::string name, GLfloat width_mm, GLfloat height_mm, GLfloat dist_mm)
{
	participant = name;
	windowWidth = width_mm;
	windowHeight = height_mm;
	eyeDistance = dist_mm;

	if (participant == std::string("demo"))
	{
		this->mode = Mode::DEMO;
		generateTrial();
	}
	else
	{
		this->mode = Mode::NONE;
		std::cout << "Press 'T' to begin training, or 'S' to begin the study" << std::endl;
	}	
		
	probe.setShader(lightingShader);
	probe.init();

	trainingTarget.setShader(lightingShader);
	trainingTarget.init();
	trainingTarget.setOrientation(getRandomOrientation());
	//trainingTarget.setSize(1.1f, 1.1f, 1.1f);

	target.setSize( 5.f, 5.f, 0.f );
	target.setShader( targetShader );

	// set target color and opacity. Shader must be active to set the uniform
	targetShader->Use();	
	glUniform3f(glGetUniformLocation(targetShader->Program, "col"), 1.f, 0.f, 0.f); // red
	Shader::Off(); // turn shader back off

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2.f);

	// set camera at eye position; far clipping plane is 1 meter behind screen
	glm::vec3 eyePos( 0.f, 0.f, eyeDistance );
	camera = Camera( eyePos, windowWidth, windowHeight, eyeDistance, eyeDistance + 2000.0f );

	this->mainLoop();
}

void Study::mainLoop()
{
	// main loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		polhemus->update();

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		if (mode == Mode::DEMO) do_movement();

		if (mode != Mode::NONE && mode != Mode::PAUSED)
		{
			render();

			// Swap the screen buffers
			glfwSwapBuffers(window);
		}
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
}

void Study::render()
{	
	// Change background for Shadowed Hedgehogs
	if( renderMode == Trial::RenderMode::SHADOWED_HEDGEHOGS)
		bgColor = glm::vec3(1.f, 1.f, 1.f);
	else
		bgColor = BGCOLOR;

	// Clear the colorbuffer
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (probe_training) // probe orientation training
	{	
		// define probe colors
		glm::vec3 white = glm::vec3(1.f, 1.f, 1.f);
		glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
		glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);

		if (show_probe_hints)
		{
			glm::quat p = glm::normalize(probe.getOrientation());
			glm::quat t = glm::normalize(trainingTarget.getOrientation());
			glm::vec3 vecXp = glm::rotate(p, glm::vec3(1.f, 0.f, 0.f));
			glm::vec3 vecXt = glm::rotate(t, glm::vec3(1.f, 0.f, 0.f));
			float cosTheta = dot(vecXp, vecXt);
			float theta = glm::degrees(acos(cosTheta));

			if (theta <= 1.f) // white halo if within 1 degree
			{
				initGL(haloShader);

				glUniform1f(glGetUniformLocation(haloShader->Program, "haloSize"), 1.f);
				glUniform3f(glGetUniformLocation(haloShader->Program, "haloColor"), 1.f, 1.f, 1.f );
				glUniform1f(glGetUniformLocation(haloShader->Program, "lengthMult"), 1.f);
				glUniform1f(glGetUniformLocation(haloShader->Program, "thicknessMult"), 1.f);
				glUniform1f(glGetUniformLocation(haloShader->Program, "directionalGeomScale"), 10.f);
				glUniform1i(glGetUniformLocation(haloShader->Program, "glyphHead"), false);

				trainingTarget.setShader(haloShader);
				
				// reverse the vertex winding order
				glFrontFace(GL_CW);
				// Draw model using the halo shader (regular model will be drawn on top)
				trainingTarget.redraw();
				// reset vertex winding order
				glFrontFace(GL_CCW);
			}
			else if (theta <= 5.f) // green probe if within 5 degrees
				trainingTarget.setColor(green);			
			else if (theta <= 15.f) // from 15 degrees to 5 degrees, gradient from yellow to green
			{
				float a = (theta - 15.f) / (5.f - 15.f);
				trainingTarget.setColor(mix(yellow, green, a));
			}
			else  if (theta <= 90.f) // from 90 degrees to 15 degrees, gradient from white to yellow
			{
				float a = (theta - 90.f) / (15.f - 90.f);
				trainingTarget.setColor(mix(white, yellow, a));
			}
			else // if greater than 90 degrees, white
				trainingTarget.setColor(white);
		}
		else // default probe color is white
			trainingTarget.setColor(white);

		initGL(lightingShader);
		lightingShader->Use();
		glUniform1f(glGetUniformLocation(lightingShader->Program, "lengthMult"), 1.f);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "thicknessMult"), 1.f);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "directionalGeomScale"), 10.f);

		probe.setOrientation(normalize(polhemus->getQuaternion()));

		if (draw_probe) probe.redraw();
		trainingTarget.setShader(lightingShader);
		trainingTarget.redraw();
	}
	else
	{
		switch (trial.getRenderMode())
		{
			case Trial::RenderMode::LINES_PLAIN:
				setupPL();
				break;
			case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN:
			case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG:
			case Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG:
				setupIL();

				trial.passThroughPVMatrix((float*)glm::value_ptr(camera.getProjectionMatrix()),
					(float*)glm::value_ptr(camera.getViewMatrix()));

				break;
			case Trial::RenderMode::SHADOWED_HEDGEHOGS:
				setupSH();
				break;
			case Trial::RenderMode::TUBES_PLAIN:
			case Trial::RenderMode::TUBES_RINGED:
				setupTubes();
				break;
		}

		// Display the trial slice
		trial.display();
			
		// Now display target cursor
		initGL(targetShader);

		// Should we render the target atop everything else, or render it on the slice?
		if(target_on_top) glDisable( GL_DEPTH_TEST );

		// Turn off writing to the depth mask for transparency effects
		glDepthMask( GL_FALSE );
				
		// First draw target cursor as a filled object
		glUniform1f(glGetUniformLocation(targetShader->Program, "opacity"), 0.1f);
		target.redraw(); // draw target cursor		

		// Now draw target cursor outline slightly darker
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glUniform1f(glGetUniformLocation(targetShader->Program, "opacity"), 0.25f);
		target.redraw();
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		// Reenable writing to the depth mask
		glDepthMask( GL_TRUE );

		// Turn the depth test back on if it was turned off previously
		if(target_on_top) glEnable( GL_DEPTH_TEST );
	}
}

void Study::setupPL()
{
	this->initGL(lineShader);
	trial.setShader(lineShader);
}

void Study::setupIL()
{
	this->initGL(lightingShader);
	trial.setShader(lightingShader);	
}

void Study::setupSH()
{
	initGL(hogShader);

	glUniform1f(glGetUniformLocation(hogShader->Program, "lengthMult"), (1 / density) / 2 / trial.getMaxLength());

	glUniform1f(glGetUniformLocation(hogShader->Program, "offset"), (-trial.getShadowOffset())*(1 / density) / 2 / trial.getMaxLength() + hedgehogOffset);

	// first pass to render shadow plane
	glUniform1i(glGetUniformLocation(hogShader->Program, "doShadows"), true);

	trial.setShader(hogShader);

	trial.display();

	// second pass to render the glyph plane
	glUniform1i(glGetUniformLocation(hogShader->Program, "doShadows"), false);
}

void Study::setupTubes()
{
	if (draw_halos)
	{
		initGL(haloShader);

		glUniform1f(glGetUniformLocation(haloShader->Program, "haloSize"), haloSize);
		glUniform3f(glGetUniformLocation(haloShader->Program, "haloColor"), 0.f, 0.f, 0.f);

		trial.setShader(haloShader);

		// reverse the vertex winding order
		glFrontFace(GL_CW);
		// Draw model using the halo shader (regular model will be drawn on top)
		trial.display();
		// reset vertex winding order
		glFrontFace(GL_CCW);
	}

	this->initGL(lightingShader);
	trial.setShader(lightingShader);
}

void Study::initGL(Shader *s)
{
	// Use cooresponding shader when setting uniforms/drawing objects
	s->Use();

	// Pass the matrices to the shader
	glm::mat4 view = camera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(s->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glm::mat4 projection = camera.getProjectionMatrix();
	glUniformMatrix4fv(glGetUniformLocation(s->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Pass light and camera positions to shader
	glm::vec4 lightPos = light.getPosition();
	glUniform4f(glGetUniformLocation(s->Program, "light.position"), lightPos.x, lightPos.y, lightPos.z, lightPos.w);
	glm::vec3 cameraPos = camera.getPosition();
	glUniform3f(glGetUniformLocation(s->Program, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glm::vec3 ambientColor = light.getAmbientColor();
	glm::vec3 diffuseColor = light.getDiffuseColor();
	glm::vec3 specularColor = light.getSpecularColor();
	glUniform3f(glGetUniformLocation(s->Program, "light.ambient"), ambientColor.r, ambientColor.g, ambientColor.b);
	glUniform3f(glGetUniformLocation(s->Program, "light.diffuse"), diffuseColor.r, diffuseColor.g, diffuseColor.b);
	glUniform3f(glGetUniformLocation(s->Program, "light.specular"), specularColor.r, specularColor.g, specularColor.b);

	glUniform1f(glGetUniformLocation(s->Program, "lengthMult"), lengthMultiplier);
	glUniform1f(glGetUniformLocation(s->Program, "thicknessMult"), thicknessMultiplier);
	glUniform1f(glGetUniformLocation(s->Program, "directionalGeomScale"), directionalGeomScale);
}

void Study::training()
{	
	float angleRad = getAngleError( probe.getOrientation(), trainingTarget.getOrientation() );
	std::cout << "Angular error: " << glm::degrees( angleRad ) << " deg (" << angleRad << " rad)" << std::endl;
	//trainingTarget.setOrientation(getRandomOrientation());
	trainingTarget.setOrientation(target.getOrientation());
}

void Study::begin()
{
	mode = Mode::STUDY;
	srand((unsigned int)time(NULL));

	Trial::RenderMode renders[5] = { Trial::RenderMode::LINES_PLAIN, 
									 Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN, 
									 Trial::RenderMode::SHADOWED_HEDGEHOGS, 
									 Trial::RenderMode::TUBES_PLAIN, 
									 Trial::RenderMode::TUBES_RINGED };
	float densities[3]			 = { 0.1f, 
									 0.5f, 
									 1.f };
	float lengths[3]			 = { 1.f, 
									 0.5f, 
									 0.1f };
	float thicknesses[3]		 = { 1.f, 
									 0.5f, 
									 0.1f };
	
	std::vector<Condition> block;

	for (int i = 0; i < NDENSITYCONDITIONS; ++i)
		for (int j = 0; j < NRENDERINGCONDITIONS; ++j)
			for (int k = 0; k < NREPLICATESPERBLOCK; ++k)
				block.push_back(Condition(renders[j], densities[i], lengths[i], thicknesses[i]));

	for (int i = 0; i < NBLOCKS; ++i)
	{
		std::random_shuffle(block.begin(), block.end());
		conditions.push_back(block);
	}

	std::cout << "Commencing study..." << std::endl;
	std::cout << std::endl;
	std::cout << "participant,block,trial,render,density,lengthMulti,thicknessMulti,directGeomMulti,probe.w,probe.x,probe.y,probe.z,target.w,target.x,target.y,target.z,error_degrees,time" << std::endl;
	next();
}

void Study::next()
{	
	// get current trial block from queue
	std::vector<Condition> *block = &conditions.back();

	if (block->size() == 0)
	{
		std::cout << "Block " << NBLOCKS - conditions.size() + 1 << " of " << NBLOCKS << " completed!" << std::endl;
		std::cout << std::endl;
		conditions.pop_back();
		if (conditions.size() == 0)
			end();
		else
			block = &conditions.back();		
		
		bgColor = glm::vec3(0.f, 0.5f, 0.f);
		std::cout << "Please take a short break, then press the ENTER key when ready to begin the next block." << std::endl;
		mode = PAUSED;
	}
	else	
		bgColor = glm::vec3(0.f, 0.f, 0.f);

	// Give blank screen immediately while processing new trial
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	// get current trial condition from block queue
	Condition curr = block->back();
	block->pop_back();

	renderMode = curr.renderMode;
	density = curr.density;
	lengthMultiplier = curr.lengthMultiplier;
	thicknessMultiplier = curr.thicknessMultiplier;
	directionalGeomScale = curr.thicknessMultiplier;

	generateTrial();

	stopwatch.start();
}

void Study::end()
{
	std::cout << "Study Complete!" << std::endl;
	mode = Mode::NONE;
	
	// Tell GLFW to kill the OpenGL window
	glfwSetWindowShouldClose(window, GL_TRUE);
}


void Study::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	getInstance(window)->key_process(window, key, scancode, action, mode);
}

// Is called whenever a key is pressed/released via GLFW
void Study::key_process(GLFWwindow* window, int key, int scancode, int action, int keymode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && mode != STUDY)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
			switch (mode)
			{
			case DEMO:
				if (keys[GLFW_KEY_F])
				{
					renderMode = Trial::RenderMode::TUBES_PLAIN;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_G])
				{
					renderMode = Trial::RenderMode::TUBES_RINGED;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_H])
				{
					renderMode = Trial::RenderMode::SHADOWED_HEDGEHOGS;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_I])
				{
					renderMode = Trial::RenderMode::LINES_PLAIN;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_L])
					cycle_light = abs(cycle_light - 1);
				if (keys[GLFW_KEY_M])
					draw_halos = abs(draw_halos - 1);
				if (keys[GLFW_KEY_P])
				{
					polhemus->printInfo();
					std::cout << "Angular Error to Target: " << glm::degrees( getAngleError( getTargetOrientation(), polhemus->getQuaternion() ) ) << std::endl;
					std::cout << std::endl;
				}
				if (keys[GLFW_KEY_Q])
					training();
				if (keys[GLFW_KEY_R])
					generateTrial();
				if (keys[GLFW_KEY_T])
				{
					renderMode = Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_U])
				{
					renderMode = Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_Y])
				{
					renderMode = Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG;
					trial.setRenderMode(renderMode);
				}
				if (keys[GLFW_KEY_BACKSPACE])
				{
					thicknessMultiplier = lengthMultiplier = directionalGeomScale = 1.f;
					haloSize = 0.5f;
					hedgehogOffset = 0.f;
				}

				if (keys[GLFW_KEY_MINUS])
					lengthMultiplier -= (lengthMultiplier > 0.1f) ? 0.1f : 0.f;
				if (keys[GLFW_KEY_EQUAL])
					lengthMultiplier += 0.1f;
				if (keys[GLFW_KEY_LEFT_BRACKET])
					thicknessMultiplier -= (thicknessMultiplier > 0.01f) ? 0.01f : 0.f;
				if (keys[GLFW_KEY_RIGHT_BRACKET])
					thicknessMultiplier += 0.01f;
				if (keys[GLFW_KEY_SEMICOLON])
					directionalGeomScale -= (directionalGeomScale > 0.01f) ? 0.01f : 0.f;
				if (keys[GLFW_KEY_APOSTROPHE])
					directionalGeomScale += 0.01f;
				if (keys[GLFW_KEY_COMMA])
					haloSize -= (haloSize > 0.01f) ? 0.01f : 0.f;
				if (keys[GLFW_KEY_PERIOD])
					haloSize += 0.01f;
				if (keys[GLFW_KEY_NUM_LOCK])
					target_on_top = abs(target_on_top - 1);
				if (keys[GLFW_KEY_KP_SUBTRACT])
				{
					if (density > 0.1f + 0.0001f)
					{
						density -= 0.1f;
						trial.setDensity(density);
						trial.sampleBiMap();
					}
				}
				if (keys[GLFW_KEY_KP_ADD])
				{
					density += 0.1f;
					trial.setDensity(density);
					trial.sampleBiMap();
				}

				if (keys[GLFW_KEY_INSERT])
					probe_training = abs(probe_training - 1);
				if (keys[GLFW_KEY_DELETE])
					draw_probe = abs(draw_probe - 1);
				if (keys[GLFW_KEY_HOME])
				{
					glm::vec3 eyePos(0.f, 0.f, eyeDistance);
					camera = Camera(eyePos, windowWidth, windowHeight, eyeDistance, eyeDistance + 2000.0f);
				}
				if (keys[GLFW_KEY_END])
					show_probe_hints = abs(show_probe_hints - 1);
				if (keys[GLFW_KEY_PAGE_DOWN])
					hedgehogOffset -= (hedgehogOffset > 0.1f) ? 0.1f : 0.f;
				if (keys[GLFW_KEY_PAGE_UP])
					hedgehogOffset += 0.1f;

				if (keys[GLFW_KEY_KP_1])
					glLineWidth(1.f);
				if (keys[GLFW_KEY_KP_2])
					glLineWidth(2.f);
				if (keys[GLFW_KEY_KP_3])
					glLineWidth(3.f);
				if (keys[GLFW_KEY_KP_4])
					glLineWidth(4.f);
				if (keys[GLFW_KEY_KP_5])
					glLineWidth(5.f);

				break;
			case PAUSED:
				if (keys[GLFW_KEY_ENTER])
					mode = STUDY;
				break;
			case NONE:
				if (keys[GLFW_KEY_S])
					begin();
				if (keys[GLFW_KEY_T])
					mode = Mode::TRAINING;
				break;
			case STUDY:
				if (keys[GLFW_KEY_SPACE] && stopwatch.read() > 2.0)
				{
					glm::quat probeQuat = polhemus->getQuaternion();
					glm::quat targetQuat = getTargetOrientation();
					std::cout << participant << ",";
					std::cout << NBLOCKS - conditions.size() << ",";
					std::cout << NTRIALSPERBLOCK - conditions.back().size() - 1 << ",";
					std::cout << renderMode << ",";
					std::cout << density << ",";
					std::cout << lengthMultiplier << ",";
					std::cout << thicknessMultiplier << ",";
					std::cout << directionalGeomScale << ",";
					std::cout << probeQuat.w << ",";
					std::cout << probeQuat.x << ",";
					std::cout << probeQuat.y << ",";
					std::cout << probeQuat.z << ",";
					std::cout << targetQuat.w << ",";
					std::cout << targetQuat.x << ",";
					std::cout << targetQuat.y << ",";
					std::cout << targetQuat.z << ",";
					std::cout << glm::degrees( getAngleError( probeQuat, targetQuat ) ) << ",";
					std::cout << stopwatch.read() << std::endl;

					next();
				}
				break;
			case TRAINING:
				if (keys[GLFW_KEY_SPACE])
					training();
				break;
			}
		}
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void Study::do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.processKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.processKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.processKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.processKeyboard(RIGHT, deltaTime);
}


void Study::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	getInstance(window)->mouse_process(window, xpos, ypos);
}

void Study::mouse_process(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (GLfloat) xpos;
        lastY = (GLfloat) ypos;
        firstMouse = false;
    }

    GLfloat xoffset = (GLfloat) xpos - lastX;
    GLfloat yoffset = lastY - (GLfloat) ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = (GLfloat) xpos;
    lastY = (GLfloat) ypos;

    if(mode == DEMO) camera.processMouseMovement(xoffset, yoffset);
}

void Study::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	getInstance(window)->scroll_process(window, xoffset, yoffset);
}

void Study::scroll_process(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll((GLfloat) yoffset);
}

void Study::generateTrial()
{
	//std::cout << "Generating trial for " << windowWidth << " x " << windowHeight << "mm screen..." << std::endl;
	trial = Trial(windowWidth, windowHeight, density, jitter);
	trial.init();
	trial.setRenderMode(renderMode);

	Slice::Seed targSeed = trial.getRandomSeed();
	target.setPosition( targSeed.x - windowWidth / 2.f, targSeed.y - windowHeight / 2.f, 0.f );
	target.setSeed( targSeed );
	std::cout << "Target Vec: ( " << targSeed.dx << ", " << targSeed.dy << ", " << targSeed.dx << " )" << std::endl;
	//std::cout << "Trial generated" << std::endl;
}

glm::quat Study::getTargetOrientation()
{
	Slice::Seed seed = target.getSeed();

	glm::vec3 v1 = glm::normalize( glm::vec3( seed.dx, seed.dy, seed.dz ) );
	glm::vec3 v2 = glm::vec3( 1.f, 0.f, 0.f );

	glm::vec3 xProd = glm::cross( v1, v2 );

	float w = sqrt( ( v1.length() ^ 2 ) * ( v2.length() ^ 2 ) ) + glm::dot( v1, v2 );

	return glm::normalize( glm::quat( w, xProd.x, xProd.y, xProd.z ) );
}

float Study::getAngleError(glm::quat p, glm::quat q)
{
	p = glm::normalize( p ); 
	q = glm::normalize( q );
	glm::vec3 vecXp = glm::rotate( p, glm::vec3( 1.f, 0.f, 0.f ) );
	glm::vec3 vecXq = glm::rotate( q, glm::vec3( 1.f, 0.f, 0.f ) );
	float cosTheta = dot( vecXp, vecXq );
	return ( cosTheta > 0.999999f ) ? 0.f : acos( cosTheta );
}

glm::quat Study::getRandomOrientation()
{
	std::random_device seed;  // random seed
	std::mt19937 gen(seed()); // Mersenne Twister RNG
	std::uniform_real_distribution<float> dist(-1, 1);

	float w, x, y, z;

	w = dist( gen );
	x = dist( gen );
	y = dist( gen );
	z = dist( gen );
	
	return glm::normalize( glm::quat( w, x, y, z ) );
}
