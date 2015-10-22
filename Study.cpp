#include "Study.h"
#include <glm/gtc/type_ptr.hpp>
#include "Slice.h"
#include <time.h> // time() for srand()

#define _USE_MATH_DEFINES
#include <math.h> // M_PI

#define NBLOCKS 5
#define NCONDITIONS 15

// Initialize class variables
Study* Study::instance = NULL;

// Returns singleton Study instance
Study* Study::getInstance( GLFWwindow* window )
{
    if ( !instance )
        instance = new Study( window );
    return instance;
}

Study::Study( GLFWwindow* window )
{
	int width_px, height_px;
	glfwGetWindowSize(window, &width_px, &height_px);
	this->window = window;
	firstMouse = true;
	lastX  =  width_px  / 2.0f;
    lastY  =  height_px / 2.0f;
	deltaTime = 0.0f;	// Time between current frame and last frame
	lastFrame = 0.0f;  	// Time of last frame

	draw_halos = cycle_light = orient_probe = draw_probe = show_probe_hints = 0;

	density = 0.1f;
	jitter = 0.25f;

	lengthMultiplier = thicknessMultiplier = directionalGeomScale = 1.f;
	haloSize = 0.5f;
	hedgehogOffset = 0.f;

	polhemus = Polhemus::getInstance();

	camera = Camera();
	light = Light(glm::vec3(1.0, 1.0, 1.0));

	// initialize key array
	for (int i = 0; i < 1024; ++i)
		keys[i] = 0;
	
    // Build and compile our shader programs
	lightingShader = new Shader("materials.vert", "materials.frag");
	lineShader = new Shader("lines.vert", "lines.frag");
	haloShader = new Shader("halo.vert", "halo.frag");
	hogShader = new Shader("hedgehogs.vert", "hedgehogs.frag");
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
		generateTrial(Trial::RenderMode::LINES_PLAIN);
	}
	else
	{
		this->mode = Mode::NONE;
	}	
		
	probe.setShader(lightingShader);
	probe.renderRT(8, 0.1f);

	trainingTarget.setShader(lightingShader);
	trainingTarget.renderPT(8);
	trainingTarget.setOrientation(getRandomOrientation());
	trainingTarget.setSize(1.f, 1.1f, 1.1f);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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

		if (mode != Mode::NONE) render();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
}

void Study::render()
{	
	// Clear the colorbuffer
	glClearColor(0.325f, 0.486f, 0.812f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (orient_probe)
	{
		if (show_probe_hints)
		{
			glm::quat p = glm::normalize(probe.getOrientation());
			glm::quat t = glm::normalize(trainingTarget.getOrientation());
			glm::vec3 vecXp = glm::rotate(p, glm::vec3(1.f, 0.f, 0.f));
			glm::vec3 vecXt = glm::rotate(t, glm::vec3(1.f, 0.f, 0.f));
			float cosTheta = dot(vecXp, vecXt);
			float theta = glm::degrees(acos(cosTheta));

			if (theta <= 1.f)
			{
				initGL(haloShader);

				glUniform1f(glGetUniformLocation(haloShader->Program, "haloSize"), 1.f);
				glUniform4f(glGetUniformLocation(haloShader->Program, "haloColor"), 1.f, 1.f, 1.f, 0.5f );
				glUniform1f(glGetUniformLocation(haloShader->Program, "lengthMult"), 60.f);
				glUniform1f(glGetUniformLocation(haloShader->Program, "thicknessMult"), 10.f);
				glUniform1f(glGetUniformLocation(haloShader->Program, "directionalGeomScale"), 7.5f);

				trainingTarget.setShader(haloShader);
				
				// reverse the vertex winding order
				glFrontFace(GL_CW);
				// Draw model using the halo shader (regular model will be drawn on top)
				trainingTarget.redraw();
				// reset vertex winding order
				glFrontFace(GL_CCW);
			}
			else if (theta <= 5.f)
			{
				glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
				glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
				trainingTarget.setColor(0.f, 1.f, 0.f);
			}
			else if (theta <= 15.f)
			{
				glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
				glm::vec3 green = glm::vec3(0.f, 1.f, 0.f);
				float a = (theta - 15.f) / (5.f - 15.f);
				trainingTarget.setColor(mix(yellow, green, a));
			}
			else  if (theta <= 90.f)
			{
				glm::vec3 white = glm::vec3(1.f, 1.f, 1.f);
				glm::vec3 yellow = glm::vec3(1.f, 1.f, 0.f);
				float a = (theta - 90.f) / (15.f - 90.f);
				trainingTarget.setColor(mix(white, yellow, a));
			}
			else
				trainingTarget.setColor(1.f, 1.f, 1.f);
		}
		else
			trainingTarget.setColor(1.f, 1.f, 1.f);

		initGL(lightingShader);
		lightingShader->Use();
		glUniform1f(glGetUniformLocation(lightingShader->Program, "lengthMult"), 60.f);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "thicknessMult"), 10.f);
		glUniform1f(glGetUniformLocation(lightingShader->Program, "directionalGeomScale"), 7.5f);

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
			this->initGL(lineShader);
			trial.setShader(lineShader);
			trial.display();
			break;
		case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN:
		case Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG:
		case Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG:
			Shader::Off();

			trial.passThroughPVMatrix((float*)glm::value_ptr(camera.getProjectionMatrix()),
				(float*)glm::value_ptr(camera.getViewMatrix()));
			trial.display();

			break;
		case Trial::RenderMode::SHADOWED_HEDGEHOGS:
			initGL(hogShader);

			glUniform1f(glGetUniformLocation(hogShader->Program, "lengthMult"), (1 / density) / 2 / trial.getMaxLength());

			glUniform1f(glGetUniformLocation(hogShader->Program, "offset"), (-trial.getShadowOffset())*(1 / density) / 2 / trial.getMaxLength() + hedgehogOffset);

			// first pass to render shadow plane
			glUniform1i(glGetUniformLocation(hogShader->Program, "doShadows"), true);

			trial.setShader(hogShader);

			trial.display();

			// second pass to render the glyph plane
			glUniform1i(glGetUniformLocation(hogShader->Program, "doShadows"), false);

			trial.display();

			break;
		case Trial::RenderMode::TUBES_PLAIN:
		case Trial::RenderMode::TUBES_RINGED:
			if (draw_halos)
			{
				initGL(haloShader);

				glUniform1f(glGetUniformLocation(haloShader->Program, "haloSize"), haloSize);
				glUniform4f(glGetUniformLocation(haloShader->Program, "haloColor"), 0.f, 0.f, 0.f, 1.f );
				
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
			trial.display();

			break;
		}
	}
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
	glm::quat p = glm::normalize( probe.getOrientation() );
	glm::quat t = glm::normalize( trainingTarget.getOrientation() );
	glm::vec3 vecXp = glm::rotate( p, glm::vec3( 1.f, 0.f, 0.f ) );
	glm::vec3 vecXt = glm::rotate( t, glm::vec3( 1.f, 0.f, 0.f ) );
	float cosTheta = dot( vecXp, vecXt );
	float angleRad = ( cosTheta > 0.999999f ) ? 0.f : acos( cosTheta );
	std::cout << "Angular error: " << glm::degrees( angleRad ) << " deg (" << angleRad << " rad)" << std::endl;
	trainingTarget.setOrientation(getRandomOrientation());
}

void Study::begin()
{
	mode = Mode::STUDY;
	srand((unsigned int)time(NULL));
	
	for (unsigned int i = 0; i < NBLOCKS * NCONDITIONS; ++i)
	{

	}
}

void Study::next()
{

}

void Study::end()
{

}


void Study::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	getInstance(window)->key_process(window, key, scancode, action, mode);
}

// Is called whenever a key is pressed/released via GLFW
void Study::key_process(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
			switch (mode)
			{
			case DEMO:
				if (keys[GLFW_KEY_F])
					trial.setRenderMode(Trial::RenderMode::TUBES_PLAIN);
				if (keys[GLFW_KEY_G])
					trial.setRenderMode(Trial::RenderMode::TUBES_RINGED);
				if (keys[GLFW_KEY_H])
					trial.setRenderMode(Trial::RenderMode::SHADOWED_HEDGEHOGS);
				if (keys[GLFW_KEY_I])
					trial.setRenderMode(Trial::RenderMode::LINES_PLAIN);
				if (keys[GLFW_KEY_L])
					cycle_light = abs(cycle_light - 1);
				if (keys[GLFW_KEY_M])
					draw_halos = abs(draw_halos - 1);
				if (keys[GLFW_KEY_P])
					polhemus->printInfo();
				if (keys[GLFW_KEY_Q])
					training();
				if (keys[GLFW_KEY_R])
					generateTrial(trial.getRenderMode());
				if (keys[GLFW_KEY_T])
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_BLINN);
				if (keys[GLFW_KEY_U])
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_MAXIMUM_PHONG);
				if (keys[GLFW_KEY_Y])
					trial.setRenderMode(Trial::RenderMode::LINES_ILLUMINATED_CYLINDER_PHONG);
				if (keys[GLFW_KEY_BACKSPACE])
				{
					thicknessMultiplier = lengthMultiplier = directionalGeomScale = 1.f;
					haloSize = 0.5f;
					hedgehogOffset = 5.f;
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

				if (keys[GLFW_KEY_INSERT])
					orient_probe = abs(orient_probe - 1);
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
				break;
			case NONE:
				if (keys[GLFW_KEY_S])
					begin();
				if (keys[GLFW_KEY_T])
					mode = Mode::TRAINING;
				break;
			case STUDY:
				if (keys[GLFW_KEY_SPACE])
					next();
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

    camera.processMouseMovement(xoffset, yoffset);
}

void Study::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	getInstance(window)->scroll_process(window, xoffset, yoffset);
}

void Study::scroll_process(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll((GLfloat) yoffset);
}

void Study::generateTrial(Trial::RenderMode renderMode)
{
	std::cout << "Generating trial for " << windowWidth << " x " << windowHeight << "mm screen..." << std::endl;
	trial = Trial(windowWidth, windowHeight, density, jitter);
	trial.init();
	trial.setRenderMode(renderMode);
	std::cout << "Trial generated" << std::endl;
}

glm::quat Study::getRandomOrientation()
{
	srand( (unsigned) time( NULL ) );

	float angle = ( (float) rand() / (float) RAND_MAX ) * 360; // 0 to 360 

	float z = ( (float) rand() / (float) RAND_MAX ) * 2 - 1; // -1 to 1

	glm::vec3 axis;

	axis.x = sqrtf(1.f - z * z) * cos(angle);
	axis.y = sqrtf(1.f - z * z) * sin(angle);
	axis.z = z;
	
	angle = ((float)rand() / (float)RAND_MAX) * 2 * M_PI; // 0 to 2pi 

	glm::quat ret = glm::normalize( glm::angleAxis( angle, axis ) );

	return ret;
}